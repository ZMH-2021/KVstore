#ifndef __KV_STORE_H__
#define __KV_STORE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

#include "array.h"
#include "rbtree.h"
#include "hash.h"
#include "reactor.h"
#include "multi_reactor.h"

#define KVS_MAX_TOKENS 1024

typedef enum
{
    NETWORK_REACTOR,
    NETWORK_MULTI_REACTOR
} network_model_t;

typedef int (*msg_handler)(char *msg, int length, char *response);
typedef struct
{
    int (*start)(unsigned short port, msg_handler handler);
} network_ops_t;

// use
int init_kvengine(void);
int start_kvstore_server(network_model_t model, unsigned short port);
void dest_kvengine(void);

#endif