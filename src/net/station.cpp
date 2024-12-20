#include "system.h"
#include "utils.h"
#include "virtual-machine.h"
#include "station.h"

static sigil::vmnode_t *station_node = nullptr;

static struct station_data_t {
    std::vector<sigil::station::wifi_entry_t> networks_found;
    std::map<std::string, int> networks_name_map;
    station_data_t() {memset((void*)this, 0, sizeof(*this));}
} *station_data = nullptr;

sigil::status_t  sigil::station::start() {
    sigil::vmnode_t* vmsr = virtual_machine::get_root_node();
    if (!vmsr) return sigil::VM_ARG_NULL;

    sigil::vmnode_t* platform = virtual_machine::get_platform_node();
    if (!vmsr) return sigil::VM_ARG_NULL;
    
    station_node = platform->spawn_subnode("station");
    if (!station_node) return sigil::VM_FAILED_ALLOC;

    station_data = new station_data_t();
    station_node->start = sigil::station::start;
    station_node->data = station_data;
    station_node->stop = sigil::station::stop;
    return sigil::VM_OK;
}



sigil::status_t stop() {
    if (!station_node) return sigil::VM_SKIPPED;
    if (!station_data) return sigil::VM_UNKNOWN_SUCCESS;
    delete station_data;
    station_data = nullptr;
    station_node->node_data.data = nullptr;
    return sigil::VM_OK;
}

int sigil::station::scan_networks() {
    int num_found_networks = 0;
    // TODO: Implement scan, async not needed
    return num_found_networks;
}


// void sigil::station::scanner_task(void *context) {
//     while (true) {
//         utils::delay(1000);
//         //mod_print(station::mod, "running network scan...\n");
//         scan_networks();
//     }
// #   ifdef TARGET_8266
//     vTaskDelete(NULL);
// #   endif
// }

// int sigil::station::init(void *ctx) {
//     if (!mod) return -EPERM;

//     sigil::station::data = new station_data_t;

//     if (!sigil::station::data) return -ENOMEM;

//     sigil::station::data->scanner_worker = sigil::runtime::workthread::create("station_scanner", station::mod);
//     if (!sigil::station::data->scanner_worker) {
//         //mod_print(station::mod, "error - could not spawn worker module\n");
//         return -ENOMEM;
//     }

//     mod->data = data;

//     return 0;
// }
