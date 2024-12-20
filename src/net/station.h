/*
    Station module, used to connect/disconnect to and from networks.
    Will be used for desktop manager if it ever comes.
*/
#pragma once
#include "system.h"

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

    // VM Tree API
    sigil::status_t start();
    sigil::vmnode_t stop();

    // Station API
    int scan_networks(); //Return num found networks or negative on error
    sigil::status_t connect_to(wifi_entry_t network);
    sigil::status_t disconnect();
}
