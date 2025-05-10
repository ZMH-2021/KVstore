#include "KVstoreClient.h"

int main(int argc, char *argv[])
{
    char in_buffer[BUFFER_SIZE] = {"SET age 20"};
    char out_buffer[BUFFER_SIZE] = {0};

    if (kvstore_connect("43.138.4.2", "9999") != 0)
    {
        printf("Connection failed\n");
        return -1;
    }

    if (kvstore_send_command("SET age 20", out_buffer) != 0)
    {
        printf("Command failed\n");
    }
    else
    {
        printf("%s\n", out_buffer);
    }

    kvstore_disconnect();

    return 0;
}