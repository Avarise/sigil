#pragma once
/*
    Station manager, used to connect/disconnect to and from networks
*/
#include <asm-generic/errno.h>
#include <cerrno>
#include <vector>
#include <map>
#include "../vm/core.h"
#include "../vm/node.h"


namespace sigil::station {

    struct wifi_entry_t {
        uint8_t encryption_type;
        std::string password;
        std::string ssid;
        int32_t channel;
        char bssid[6]; // MAC address
        int32_t rssi;
        bool hidden;
    };
    struct station_data_t {
        std::vector<wifi_entry_t> networks_found;
        std::map<std::string, int> networks_name_map;
    };

    // VM Tree API
    sigil::status_t initialize(sigil::vmnode_t *vmsr);
    sigil::vmnode_t probe(sigil::vmnode_t *vmsr);
    sigil::status_t deinitialize();

    // Station API
    int scan_networks(); //Return num found networks or negative on error
    sigil::status_t connect_to(std::string ssid, std::string psk);
    sigil::status_t disconnect();

}
