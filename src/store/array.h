
#ifndef __ARRAY_H__
#define __ARRAY_H__

#define ARRAY_SIZE 1024

typedef struct array_item_s
{
    char *key;
    char *value;
} array_item_t;

typedef struct array_s
{
    array_item_t *table;
    int idx;
    int total;
} array_t;

extern array_t global_array;

int array_create(array_t *inst);
void array_destory(array_t *inst);

int array_set(array_t *inst, char *key, char *value);
char *array_get(array_t *inst, char *key);
int array_del(array_t *inst, char *key);
int array_mod(array_t *inst, char *key, char *value);
int array_exist(array_t *inst, char *key);

#endif