#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <pthread.h>

#include "multi_reactor.h"

#define MAX_REACTORS 4

typedef int (*RCALLBACK)(int fd, void *reactor);

typedef struct
{
    int epfd;            // 每个reactor自己的epoll实例
    pthread_t thread_id; // 线程ID
    int thread_running;  // 线程运行标志
} Reactor;

typedef struct
{
    int fd;

    char rbuffer[BUFFER_LENGTH];
    int rlength;

    char wbuffer[BUFFER_LENGTH];
    int wlength;

    RCALLBACK send_callback;
    RCALLBACK recv_callback;
    RCALLBACK accept_callback;

    Reactor *reactor; // 关联的reactor
} Connection;

static reactor_msg_handler static_handler;
Reactor reactors[MAX_REACTORS];
static int reactor_count = 0;
static int next_reactor = 0;
Connection multi_reactor_conn_list[CONNECTION_SIZE] = {0};
int listen_fd = -1;

static int request(Connection *c)
{
    c->wlength = static_handler(c->rbuffer, c->rlength, c->wbuffer);
    return 0;
}

static int accept_cb(int fd, void *reactor);
static int recv_cb(int fd, void *reactor);
static int send_cb(int fd, void *reactor);

static int set_event(Reactor *reactor, int fd, int event, int flag)
{
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;

    if (flag)
    {
        if (epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
        {
            return -1;
        }
    }
    else
    {
        if (epoll_ctl(reactor->epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
        {
            return -1;
        }
    }
    return 0;
}

static int event_register(Reactor *reactor, int fd, int event)
{
    if (fd < 0)
        return -1;

    multi_reactor_conn_list[fd].fd = fd;
    multi_reactor_conn_list[fd].recv_callback = recv_cb;
    multi_reactor_conn_list[fd].send_callback = send_cb;
    multi_reactor_conn_list[fd].reactor = reactor;

    memset(multi_reactor_conn_list[fd].rbuffer, 0, BUFFER_LENGTH);
    multi_reactor_conn_list[fd].rlength = 0;

    memset(multi_reactor_conn_list[fd].wbuffer, 0, BUFFER_LENGTH);
    multi_reactor_conn_list[fd].wlength = 0;

    if (set_event(reactor, fd, event, 1) < 0)
    {
        return -1;
    }
    return 0;
}

static int accept_cb(int fd, void *reactor)
{
    (void)reactor;
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int clientfd = accept(fd, (struct sockaddr *)&clientaddr, &len);
    if (clientfd < 0)
    {
        printf("accept errno: %d --> %s\n", errno, strerror(errno));
        return -1;
    }

    // 轮询分配reactor
    Reactor *assigned_reactor = &reactors[next_reactor];
    next_reactor = (next_reactor + 1) % reactor_count;

    event_register(assigned_reactor, clientfd, EPOLLIN);

    return 0;
}

static int recv_cb(int fd, void *reactor)
{
    Connection *conn = &multi_reactor_conn_list[fd];
    Reactor *r = (Reactor *)reactor;

    memset(conn->rbuffer, 0, BUFFER_LENGTH);
    int count = recv(fd, conn->rbuffer, BUFFER_LENGTH, 0);
    if (count == 0)
    {
        close(fd);
        epoll_ctl(r->epfd, EPOLL_CTL_DEL, fd, NULL);
        return 0;
    }
    if (count < 0)
    {
        printf("count: %d, errno: %d, %s\n", count, errno, strerror(errno));
        close(fd);
        epoll_ctl(r->epfd, EPOLL_CTL_DEL, fd, NULL);
        return 0;
    }

    conn->rlength = count;
    request(conn);
    set_event(r, fd, EPOLLOUT, 0);

    return count;
}

static int send_cb(int fd, void *reactor)
{
    Connection *conn = &multi_reactor_conn_list[fd];
    Reactor *r = (Reactor *)reactor;

    int count = 0;
    if (conn->wlength != 0)
    {
        count = send(fd, conn->wbuffer, conn->wlength, 0);
    }

    set_event(r, fd, EPOLLIN, 0);
    return count;
}

void *reactor_thread(void *arg)
{
    Reactor *reactor = (Reactor *)arg;

    while (reactor->thread_running)
    {
        struct epoll_event events[4096] = {0};
        int nready = epoll_wait(reactor->epfd, events, 4096, 100);

        for (int i = 0; i < nready; i++)
        {
            int connfd = events[i].data.fd;
            Connection *conn = &multi_reactor_conn_list[connfd];

            if (events[i].events & EPOLLIN)
            {
                conn->recv_callback(connfd, reactor);
            }

            if (events[i].events & EPOLLOUT)
            {
                conn->send_callback(connfd, reactor);
            }
        }
    }

    return NULL;
}

static int r_init_server(unsigned short port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (-1 == bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)))
    {
        printf("bind failed: %s\n", strerror(errno));
        return -1;
    }

    listen(sockfd, 10);
    return sockfd;
}

int multi_reactor_start(unsigned short port, reactor_msg_handler handler)
{

    static_handler = handler;
    reactor_count = MAX_REACTORS;
    next_reactor = 0;

    // 初始化主reactor(监听socket)
    listen_fd = r_init_server(port);
    if (listen_fd < 0)
    {
        return -1;
    }

    // 初始化子reactors
    for (int i = 0; i < reactor_count; i++)
    {
        reactors[i].epfd = epoll_create(1);
        reactors[i].thread_running = 1;

        if (pthread_create(&reactors[i].thread_id, NULL, reactor_thread, &reactors[i]) != 0)
        {
            perror("pthread_create");
            return -1;
        }
    }

    // 主reactor处理accept
    Reactor main_reactor;
    main_reactor.epfd = epoll_create(1);

    multi_reactor_conn_list[listen_fd].fd = listen_fd;
    multi_reactor_conn_list[listen_fd].accept_callback = accept_cb;
    multi_reactor_conn_list[listen_fd].reactor = &main_reactor;

    set_event(&main_reactor, listen_fd, EPOLLIN, 1);

    // 主reactor循环
    while (1)
    {
        struct epoll_event events[10] = {0};
        int nready = epoll_wait(main_reactor.epfd, events, 10, -1);

        for (int i = 0; i < nready; i++)
        {
            int fd = events[i].data.fd;
            if (fd == listen_fd)
            {
                multi_reactor_conn_list[fd].accept_callback(fd, &main_reactor);
            }
        }
    }

    return 0;
}

void multi_reactor_stop()
{
    // 停止所有子reactor线程
    for (int i = 0; i < reactor_count; i++)
    {
        reactors[i].thread_running = 0;
        pthread_join(reactors[i].thread_id, NULL);
        close(reactors[i].epfd);
    }

    if (listen_fd != -1)
    {
        close(listen_fd);
    }
}