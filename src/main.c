#include "kvstore.h"

int main()
{
    init_kvengine();
    start_kvstore_server(NETWORK_REACTOR, 9999);
    dest_kvengine();
}