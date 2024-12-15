/*
    Web Host module, to serve connections with other clients.
*/
#pragma once
#include "utils.h"

namespace sigil::station {
    struct server_descriptor_t;
    struct remote_t;


    // Server API
    sigil::status_t start_server(server_descriptor_t desc);
    sigil::status_t open_connection(remote_t *client);
    sigil::status_t close_connection(remote_t *client);
}
