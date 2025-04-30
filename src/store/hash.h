
#ifndef __HASH_H__
#define __HASH_H__

#define MAX_KEY_LEN 128
#define MAX_VALUE_LEN 512
#define MAX_TABLE_SIZE 1024

#define ENABLE_KEY_POINTER 1

typedef struct hashnode_s
{
#if ENABLE_KEY_POINTER
    char *key;
    char *value;
#else
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
#endif
    struct hashnode_s *next;

} hashnode_t;

typedef struct hashtable_s
{

    hashnode_t **nodes; //* change **,

    int max_slots;
    int count;

} hashtable_t;

extern hashtable_t global_hash;

int hash_create(hashtable_t *hash);
void hash_destory(hashtable_t *hash);

int hash_set(hashtable_t *hash, char *key, char *value);
char *hash_get(hashtable_t *hash, char *key);
int hash_mod(hashtable_t *hash, char *key, char *value);
int hash_del(hashtable_t *hash, char *key);
int hash_exist(hashtable_t *hash, char *key);

#endif