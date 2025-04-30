
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stddef.h>

#include "hash.h"

hashtable_t global_hash = {0};

static int _hash(char *key, int size)
{

	if (!key)
		return -1;

	int sum = 0;
	int i = 0;

	while (key[i] != 0)
	{
		sum += key[i];
		i++;
	}

	return sum % size;
}

hashnode_t *_create_node(char *key, char *value)
{

	hashnode_t *node = (hashnode_t *)malloc(sizeof(hashnode_t));
	if (!node)
		return NULL;

#if ENABLE_KEY_POINTER
	char *kcopy = malloc(strlen(key) + 1);
	if (kcopy == NULL)
		return NULL;
	memset(kcopy, 0, strlen(key) + 1);
	strncpy(kcopy, key, strlen(key));

	node->key = kcopy;

	char *kvalue = malloc(strlen(value) + 1);
	if (kvalue == NULL)
	{
		free(kvalue);
		return NULL;
	}
	memset(kvalue, 0, strlen(value) + 1);
	strncpy(kvalue, value, strlen(value));

	node->value = kvalue;

#else
	strncpy(node->key, key, MAX_KEY_LEN);
	strncpy(node->value, value, MAX_VALUE_LEN);
#endif
	node->next = NULL;

	return node;
}

int hash_create(hashtable_t *hash)
{

	if (!hash)
		return -1;

	hash->nodes = (hashnode_t **)malloc(sizeof(hashnode_t *) * MAX_TABLE_SIZE);
	if (!hash->nodes)
		return -1;

	hash->max_slots = MAX_TABLE_SIZE;
	hash->count = 0;

	return 0;
}

void hash_destory(hashtable_t *hash)
{

	if (!hash)
		return;

	int i = 0;
	for (i = 0; i < hash->max_slots; i++)
	{
		hashnode_t *node = hash->nodes[i];

		while (node != NULL)
		{ // error

			hashnode_t *tmp = node;
			node = node->next;
			hash->nodes[i] = node;

			free(tmp);
		}
	}

	free(hash->nodes);
}

// 5 + 2

int hash_set(hashtable_t *hash, char *key, char *value)
{

	if (!hash || !key || !value)
		return -1;

	int idx = _hash(key, MAX_TABLE_SIZE);

	hashnode_t *node = hash->nodes[idx];
#if 1
	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
		{ // exist
			return 1;
		}
		node = node->next;
	}
#endif

	hashnode_t *new_node = _create_node(key, value);
	new_node->next = hash->nodes[idx];
	hash->nodes[idx] = new_node;

	hash->count++;

	return 0;
}

char *hash_get(hashtable_t *hash, char *key)
{

	if (!hash || !key)
		return NULL;

	int idx = _hash(key, MAX_TABLE_SIZE);

	hashnode_t *node = hash->nodes[idx];

	while (node != NULL)
	{

		if (strcmp(node->key, key) == 0)
		{
			return node->value;
		}

		node = node->next;
	}

	return NULL;
}

int hash_mod(hashtable_t *hash, char *key, char *value)
{

	if (!hash || !key)
		return -1;

	int idx = _hash(key, MAX_TABLE_SIZE);

	hashnode_t *node = hash->nodes[idx];

	while (node != NULL)
	{

		if (strcmp(node->key, key) == 0)
		{
			break;
		}

		node = node->next;
	}

	if (node == NULL)
	{
		return 1;
	}

	// node -->
	free(node->value);

	char *kvalue = malloc(strlen(value) + 1);
	if (kvalue == NULL)
		return -2;
	memset(kvalue, 0, strlen(value) + 1);
	strncpy(kvalue, value, strlen(value));

	node->value = kvalue;

	return 0;
}

int hash_count(hashtable_t *hash)
{
	return hash->count;
}

int hash_del(hashtable_t *hash, char *key)
{
	if (!hash || !key)
		return -2;

	int idx = _hash(key, MAX_TABLE_SIZE);

	hashnode_t *head = hash->nodes[idx];
	if (head == NULL)
		return -1; // noexist
	// head node
	if (strcmp(head->key, key) == 0)
	{
		hashnode_t *tmp = head->next;
		hash->nodes[idx] = tmp;

		free(head);
		hash->count--;

		return 0;
	}

	hashnode_t *cur = head;
	while (cur->next != NULL)
	{
		if (strcmp(cur->next->key, key) == 0)
			break; // search node

		cur = cur->next;
	}

	if (cur->next == NULL)
	{

		return -1;
	}

	hashnode_t *tmp = cur->next;
	cur->next = tmp->next;
#if ENABLE_KEY_POINTER
	free(tmp->key);
	free(tmp->value);
#endif
	free(tmp);

	hash->count--;

	return 0;
}

int hash_exist(hashtable_t *hash, char *key)
{

	char *value = hash_get(hash, key);
	if (!value)
		return 1;

	return 0;
}
