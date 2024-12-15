#include <map>
#include "virtual-machine.h"
#include "station.h"
#include "system.h"
#include "utils.h"

static sigil::vmnode_t *station_node = nullptr;
std::vector<sigil::station::wifi_entry_t> networks_found = {};
// LUT to find networks by name, usable on esp
std::map<std::string, int> networks_name_map = {};

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
sigil::status_t  sigil::station::initialize() {
    sigil::status_t status = virtual_machine::get_state();
    if (status != VM_OK) return status;


    vmnode_descriptor_t station_init_data;
    station_init_data.name.value = "station";

    status = virtual_machine::add_platform_node(station_init_data);
    if (status != VM_OK) return status;

    // Initialize module private data now

    return sigil::VM_OK;
}



sigil::status_t sigil::station::deinitialize() {

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

