#ifndef __RBTREE_H__
#define __RBTREE_H__

#define RED 1
#define BLACK 2

#define ENABLE_KEY_CHAR 1

#if ENABLE_KEY_CHAR
typedef char *KEY_TYPE;
#else
typedef int KEY_TYPE;
#endif

typedef struct _rbtree_node
{
    unsigned char color;
    struct _rbtree_node *right;
    struct _rbtree_node *left;
    struct _rbtree_node *parent;
    KEY_TYPE key;
    void *value;
} rbtree_node;

typedef struct _rbtree
{
    rbtree_node *root;
    rbtree_node *nil;
} rbtree_t;

extern rbtree_t global_rbtree;

int rbtree_create(rbtree_t *inst);
void rbtree_destory(rbtree_t *inst);
int rbtree_set(rbtree_t *inst, char *key, char *value);
char *rbtree_get(rbtree_t *inst, char *key);
int rbtree_del(rbtree_t *inst, char *key);
int rbtree_mod(rbtree_t *inst, char *key, char *value);
int rbtree_exist(rbtree_t *inst, char *key);

#endif