

#ifndef __MULTI_REACTOR_H__
#define __MULTI_REACTOR_H__

#define BUFFER_LENGTH 512
#define CONNECTION_SIZE 100000

typedef int (*reactor_msg_handler)(char *msg, int length, char *response);
int multi_reactor_start(unsigned short port, reactor_msg_handler handler);
void multi_reactor_stop();

#endif
