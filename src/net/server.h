#pragma once
/*
    Web Host module, to serve connections with other clients.
*/
#include <asm-generic/errno.h>
#include <cerrno>
#include <vector>
#include <map>
#include "../vm/core.h"
#include "../vm/node.h"


namespace sigil::vmwebhost {
    struct vmwebhost_data_t {
        // Data for server hosting and port mgmt
    };

    struct server_init_desc_t {

    };

    struct remote_t {

    };

    // VM Tree API
    sigil::status_t initialize(sigil::vmnode_t *vmsr);
    sigil::vmnode_t probe(sigil::vmnode_t *vmsr);
    sigil::status_t deinitialize();
    vmwebhost_data_t* api_handle(sigil::vmnode_t *vmsr);

    // Server API
    sigil::status_t start_server(server_init_desc_t desc);
    sigil::status_t open_connection(remote_t *client);
    sigil::status_t close_connection(remote_t *client);
}
