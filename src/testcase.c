

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_MSG_LENGTH 512

long long time_used_ms(struct timeval begin, struct timeval end)
{
    return (end.tv_sec - begin.tv_sec) * 1000 + (end.tv_usec - begin.tv_usec) / 1000;
}

int send_msg(int connfd, char *msg, int length)
{
    int res = send(connfd, msg, length, 0);
    if (res < 0)
    {
        perror("send");
        exit(1);
    }
    return res;
}

int recv_msg(int connfd, char *msg, int length)
{

    int res = recv(connfd, msg, length, 0);
    if (res < 0)
    {
        perror("recv");
        exit(1);
    }
    return res;
}

void testcase(int connfd, char *msg, char *res)
{

    if (!msg || !res)
        return;

    send_msg(connfd, msg, strlen(msg));

    char result[MAX_MSG_LENGTH] = {0};
    recv_msg(connfd, result, MAX_MSG_LENGTH);

    if (strcmp(result, res) == 0)
    {
        // printf("PASS\n");
    }
    else
    {
        printf("FAILED: '%s' != '%s' \n", result, res);
    }
}

int connect_tcpserver(const char *ip, unsigned short port)
{

    int connfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (0 != connect(connfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)))
    {
        perror("connect");
        return -1;
    }

    return connfd;
}

void array_testcase(int connfd, int count)
{

    int i = 0;

    struct timeval begin, end;
    gettimeofday(&begin, NULL);

    for (i = 0; i < count; i++)
    {
        testcase(connfd, "SET name zmh", "OK\r\n");
        testcase(connfd, "GET name", "zmh\r\n");
        testcase(connfd, "MOD name zhangsan", "OK\r\n");
        testcase(connfd, "GET name", "zhangsan\r\n");
        testcase(connfd, "EXIST name", "EXIST\r\n");
        testcase(connfd, "DEL name", "OK\r\n");
        testcase(connfd, "GET name", "NO EXIST\r\n");
        testcase(connfd, "MOD name zhangwei", "NO EXIST\r\n");
        testcase(connfd, "EXIST name", "NO EXIST\r\n");
        testcase(connfd, "EXIST name", "NO EXIST\r\n");
    }

    gettimeofday(&end, NULL);

    long long time_used = time_used_ms(begin, end); // ms
    long long qps = (10 * count * 1000) / time_used;

    printf("array testcase --> total time: %lld ms, qps: %lld\n", time_used, qps);
}

void rbtree_testcase(int connfd, int count)
{

    int i = 0;

    struct timeval begin, end;
    gettimeofday(&begin, NULL);

    for (i = 0; i < count; i++)
    {
        testcase(connfd, "RSET name zmh", "OK\r\n");
        testcase(connfd, "RGET name", "zmh\r\n");
        testcase(connfd, "RMOD name zhangsan", "OK\r\n");
        testcase(connfd, "RGET name", "zhangsan\r\n");
        testcase(connfd, "REXIST name", "EXIST\r\n");
        testcase(connfd, "RDEL name", "OK\r\n");
        testcase(connfd, "RGET name", "NO EXIST\r\n");
        testcase(connfd, "RMOD name zhangwei", "NO EXIST\r\n");
        testcase(connfd, "REXIST name", "NO EXIST\r\n");
        testcase(connfd, "REXIST name", "NO EXIST\r\n");
    }

    gettimeofday(&end, NULL);

    long long time_used = time_used_ms(begin, end); // ms
    long long qps = (10 * count * 1000) / time_used;

    printf("rbtree testcase --> total time: %lld ms, qps: %lld\n", time_used, qps);
}

void hash_testcase(int connfd, int count)
{

    int i = 0;

    struct timeval begin, end;
    gettimeofday(&begin, NULL);

    for (i = 0; i < count; i++)
    {
        testcase(connfd, "HSET name zmh", "OK\r\n");
        testcase(connfd, "HGET name", "zmh\r\n");
        testcase(connfd, "HMOD name zhangsan", "OK\r\n");
        testcase(connfd, "HGET name", "zhangsan\r\n");
        testcase(connfd, "HEXIST name", "EXIST\r\n");
        testcase(connfd, "HDEL name", "OK\r\n");
        testcase(connfd, "HGET name", "NO EXIST\r\n");
        testcase(connfd, "HMOD name zhangwei", "NO EXIST\r\n");
        testcase(connfd, "HEXIST name", "NO EXIST\r\n");
        testcase(connfd, "HEXIST name", "NO EXIST\r\n");
    }

    gettimeofday(&end, NULL);

    long long time_used = time_used_ms(begin, end); // ms
    long long qps = (10 * count * 1000) / time_used;

    printf("hash testcase --> total time: %lld ms, qps: %lld\n", time_used, qps);
}

// ./testcase 192.168.243.131 9999 1
// ./testcase ip port mode
int main(int argc, char *argv[])
{

    if (argc != 4)
    {
        printf("arg error\n");
        return -1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int mode = atoi(argv[3]);

    int count = 1000;

    int connfd = connect_tcpserver(ip, port);

    if (mode == 1)
    {
        array_testcase(connfd, count);
    }
    if (mode == 2)
    {
        rbtree_testcase(connfd, count);
    }
    if (mode == 3)
    {
        hash_testcase(connfd, count);
    }

    close(connfd);

    return 0;
}
