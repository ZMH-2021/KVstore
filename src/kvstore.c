#include "kvstore.h"

static network_ops_t network_ops;

const char *command[] = {
	"SET", "GET", "DEL", "MOD", "EXIST",
	"RSET", "RGET", "RDEL", "RMOD", "REXIST",
	"HSET", "HGET", "HDEL", "HMOD", "HEXIST"};

enum
{
	KVS_CMD_START = 0,
	// array
	KVS_CMD_SET = KVS_CMD_START,
	KVS_CMD_GET,
	KVS_CMD_DEL,
	KVS_CMD_MOD,
	KVS_CMD_EXIST,
	// rbtree
	KVS_CMD_RSET,
	KVS_CMD_RGET,
	KVS_CMD_RDEL,
	KVS_CMD_RMOD,
	KVS_CMD_REXIST,
	// hash
	KVS_CMD_HSET,
	KVS_CMD_HGET,
	KVS_CMD_HDEL,
	KVS_CMD_HMOD,
	KVS_CMD_HEXIST,

	KVS_CMD_COUNT,
};

int kvs_split_token(char *msg, char *tokens[])
{

	if (msg == NULL || tokens == NULL)
		return -1;

	int idx = 0;
	char *token = strtok(msg, " ");

	while (token != NULL)
	{
		tokens[idx++] = token;
		token = strtok(NULL, " ");
	}

	return idx;
}

int kvs_filter_protocol(char **tokens, int count, char *response)
{

	if (tokens[0] == NULL || count == 0 || response == NULL)
		return -1;

	int cmd = KVS_CMD_START;
	for (; cmd < KVS_CMD_COUNT; cmd++)
	{
		if (strcmp(tokens[0], command[cmd]) == 0)
		{
			break;
		}
	}

	int length = 0;
	int ret = 0;
	char *key = tokens[1];
	char *value = tokens[2];

	switch (cmd)
	{

	case KVS_CMD_SET:
		ret = array_set(&global_array, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "EXIST\r\n");
		}

		break;
	case KVS_CMD_GET:
	{
		char *result = array_get(&global_array, key);
		if (result == NULL)
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_DEL:
		ret = array_del(&global_array, key);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_MOD:
		ret = array_mod(&global_array, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_EXIST:
		ret = array_exist(&global_array, key);
		if (ret == 0)
		{
			length = sprintf(response, "EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;

	case KVS_CMD_RSET:
		ret = rbtree_set(&global_rbtree, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "EXIST\r\n");
		}

		break;
	case KVS_CMD_RGET:
	{
		char *result = rbtree_get(&global_rbtree, key);
		if (result == NULL)
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_RDEL:
		ret = rbtree_del(&global_rbtree, key);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_RMOD:
		ret = rbtree_mod(&global_rbtree, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_REXIST:
		ret = rbtree_exist(&global_rbtree, key);
		if (ret == 0)
		{
			length = sprintf(response, "EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;

	case KVS_CMD_HSET:
		ret = hash_set(&global_hash, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "EXIST\r\n");
		}

		break;
	case KVS_CMD_HGET:
	{
		char *result = hash_get(&global_hash, key);
		if (result == NULL)
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_HDEL:
		ret = hash_del(&global_hash, key);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_HMOD:
		ret = hash_mod(&global_hash, key, value);
		if (ret < 0)
		{
			length = sprintf(response, "ERROR\r\n");
		}
		else if (ret == 0)
		{
			length = sprintf(response, "OK\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_HEXIST:
		ret = hash_exist(&global_hash, key);
		if (ret == 0)
		{
			length = sprintf(response, "EXIST\r\n");
		}
		else
		{
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;

	default:
		assert(0);
	}

	return length;
}

int kvs_protocol(char *msg, int length, char *response)
{

	if (msg == NULL || length <= 0 || response == NULL)
		return -1;

	char *tokens[KVS_MAX_TOKENS] = {0};

	int count = kvs_split_token(msg, tokens);
	if (count == -1)
		return -1;

	return kvs_filter_protocol(tokens, count, response);
}

int init_kvengine(void)
{

	memset(&global_array, 0, sizeof(array_t));
	array_create(&global_array);

	memset(&global_rbtree, 0, sizeof(rbtree_t));
	rbtree_create(&global_rbtree);

	memset(&global_hash, 0, sizeof(hashtable_t));
	hash_create(&global_hash);

	return 0;
}

void dest_kvengine(void)
{
	array_destory(&global_array);

	rbtree_destory(&global_rbtree);

	hash_destory(&global_hash);
}

int start_kvstore_server(network_model_t model, unsigned short port)
{
	switch (model)
	{
	case NETWORK_REACTOR:
		network_ops.start = reactor_start;
		printf("network_model: NETWORK_REACTOR\n");
		printf("port: %d\n", port);
		break;
	case NETWORK_MULTI_REACTOR:
		network_ops.start = multi_reactor_start;
		printf("network_model: NETWORK_MULTI_REACTOR\n");
		printf("port: %d\n", port);
		break;
	default:
		return -1;
	}
	return network_ops.start(port, kvs_protocol);
}
