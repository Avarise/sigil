#include <cstdio>
#include "../vm/system.h"
#include "station.h"

using namespace sigil;

static sigil::vmnode_t *station_node = nullptr;
static station::station_data_t *station_data = nullptr;

void cleanup_station_data() {
    if (!station_node) return;
    if (!station_data) return;
    delete station_data;
    station_data = nullptr;
    station_node->node_data.data = nullptr;
}

int sigil::station::scan_networks() {
    int num_found_networks = 0;
    // TODO: Implement scan, async not needed
    return num_found_networks;
}

sigil::status_t  sigil::station::initialize(sigil::vmnode_t *vmsr) {
    if (!vmsr) return sigil::VM_ARG_NULL;
    if (system::validate_root(vmsr) != sigil::VM_OK) return sigil::VM_INVALID_ROOT;
    
    sigil::vmnode_t *platform = vmsr->peek_subnode("platform", 1);
    if (!platform) return sigil::VM_NOT_FOUND;;

    station_node = platform->spawn_subnode("station");
    if (!station_node) return sigil::VM_FAILED_ALLOC;

    station_data = new station_data_t();
    station_node->set_data(station_data, cleanup_station_data);
    return sigil::VM_OK;
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
