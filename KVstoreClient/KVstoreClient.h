#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define BUFFER_SIZE 512

int client_socket = -1;

int kvstore_connect(char *ip, char *port)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1)
    {
        perror("socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(atoi(port));

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect failed");
        close(client_socket);
        client_socket = -1;
        return -1;
    }

    return 0;
}

void kvstore_disconnect()
{
    if (client_socket != -1)
    {
        close(client_socket);
        client_socket = -1;
    }
}

int kvstore_send_command(char *input, char *output)
{
    if (client_socket == -1)
    {
        fprintf(stderr, "Not connected to server\n");
        return -1;
    }

    ssize_t sent_bytes = send(client_socket, input, strlen(input), 0);
    if (sent_bytes == -1)
    {
        perror("send failed");
        return -1;
    }

    ssize_t bytes_received = recv(client_socket, output, BUFFER_SIZE - 1, 0);
    if (bytes_received == -1)
    {
        perror("recv failed");
        return -1;
    }

    output[bytes_received] = '\0';
    return 0;
}

