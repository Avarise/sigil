/*
    Web Host module, to serve connections with other clients.
*/
#pragma once
#include "system.h"

namespace sigil::vmwebhost {
    struct server_init_desc_t {

    };

    struct remote_t {

    };

    // VM Tree API
    sigil::status_t start();
    sigil::status_t stop();

    // Server API
    sigil::status_t start_server(server_init_desc_t desc);
    sigil::status_t open_connection(remote_t *client);
    sigil::status_t close_connection(remote_t *client);
}
