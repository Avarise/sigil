/*
    Station module, used to connect/disconnect to and from networks.
    Will be used for desktop manager if it ever comes.
*/
#pragma once
#include "utils.h"

namespace sigil::station {
    struct wifi_entry_t;

    // VM Tree controls
    sigil::status_t initialize();
    sigil::status_t deinitialize();

    // Station controls
    sigil::status_t connect_to(wifi_entry_t network);
    sigil::status_t disconnect();
    int scan_networks(); //Return num found networks or negative on error

    struct wifi_entry_t {
        uint8_t encryption_type;
        std::string password;
        std::string ssid;
        int32_t channel;
        char bssid[6]; // MAC address
        int32_t rssi;
        bool hidden;
    };
}
