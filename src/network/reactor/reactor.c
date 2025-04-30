
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>

#include "reactor.h"

typedef int (*RCALLBACK)(int fd);
static reactor_msg_handler static_handler;

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
} Connection;

int epfd = 0;

Connection reactor_conn_list[CONNECTION_SIZE] = {0};

static int request(Connection *c)
{
	c->wlength = static_handler(c->rbuffer, c->rlength, c->wbuffer);
	return 0;
}

static int accept_cb(int fd);
static int recv_cb(int fd);
static int send_cb(int fd);

static int set_event(int fd, int event, int flag)
{
	if (flag)
	{
		struct epoll_event ev;
		ev.events = event;
		ev.data.fd = fd;
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
		{
			return -1;
		}
	}
	else
	{
		struct epoll_event ev;
		ev.events = event;
		ev.data.fd = fd;
		if (epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev) < 0)
		{
			return -1;
		}
	}
	return 0;
}

static int event_register(int fd, int event)
{
	if (fd < 0)
		return -1;

	reactor_conn_list[fd].fd = fd;
	reactor_conn_list[fd].recv_callback = recv_cb;
	reactor_conn_list[fd].send_callback = send_cb;

	memset(reactor_conn_list[fd].rbuffer, 0, BUFFER_LENGTH);
	reactor_conn_list[fd].rlength = 0;

	memset(reactor_conn_list[fd].wbuffer, 0, BUFFER_LENGTH);
	reactor_conn_list[fd].wlength = 0;

	if (set_event(fd, event, 1) < 0)
	{
		return -1;
	}
	return 0;
}

static int accept_cb(int fd)
{
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);

	int clientfd = accept(fd, (struct sockaddr *)&clientaddr, &len);
	if (clientfd < 0)
	{
		printf("accept errno: %d --> %s\n", errno, strerror(errno));
		return -1;
	}

	event_register(clientfd, EPOLLIN);

	return 0;
}

static int recv_cb(int fd)
{

	memset(reactor_conn_list[fd].rbuffer, 0, BUFFER_LENGTH);
	int count = recv(fd, reactor_conn_list[fd].rbuffer, BUFFER_LENGTH, 0);
	if (count == 0)
	{
		close(fd);
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);

		return 0;
	}
	if (count < 0)
	{
		printf("count: %d, errno: %d, %s\n", count, errno, strerror(errno));
		close(fd);
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);

		return 0;
	}

	reactor_conn_list[fd].rlength = count;

	request(&reactor_conn_list[fd]);

	set_event(fd, EPOLLOUT, 0);

	return count;
}

static int send_cb(int fd)
{
	int count = 0;

	if (reactor_conn_list[fd].wlength != 0)
	{
		count = send(fd, reactor_conn_list[fd].wbuffer, reactor_conn_list[fd].wlength, 0);
	}

	set_event(fd, EPOLLIN, 0);

	return count;
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
	}

	listen(sockfd, 10);

	return sockfd;
}

int reactor_start(unsigned short port, reactor_msg_handler handler)
{

	static_handler = handler;

	epfd = epoll_create(1);

	int sockfd = r_init_server(port);

	reactor_conn_list[sockfd].fd = sockfd;
	reactor_conn_list[sockfd].accept_callback = accept_cb;

	set_event(sockfd, EPOLLIN, 1);

	while (1)
	{
		struct epoll_event events[4096] = {0};
		int nready = epoll_wait(epfd, events, 4096, -1);

		for (int i = 0; i < nready; i++)
		{
			int connfd = events[i].data.fd;

			if (connfd == sockfd)
			{
				reactor_conn_list[connfd].accept_callback(connfd);
			}
			else
			{
				if (events[i].events & EPOLLIN)
				{
					reactor_conn_list[connfd].recv_callback(connfd);
				}

				if (events[i].events & EPOLLOUT)
				{
					reactor_conn_list[connfd].send_callback(connfd);
				}
			}
		}
	}
}

void reactor_stop()
{
}