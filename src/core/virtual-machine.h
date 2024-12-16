#pragma once
#include "system.h"

namespace sigil {
    struct virtual_machine_t {
        status_t initialize(int arg, char **argv);
        status_t deinitialize();
        status_t start_api_server();
        status_t start_api_vulkan();
        status_t start_api_visor();
        status_t start_api_sound();
        status_t start_api_ntt();

        // Virtual machine data
        vmnode_t *root;
    };

    void console_subprogram();
}


// /* 
//     All in one loader, for use in tests and verbose aplications
// */
// sigil::status_t system::load_all_extensions() {
//     printf("SigilVM: loading all available extensions\n");
//     assert(vmroot != nullptr);

//     sigil::status_t ret;

//     ret = iocommon::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded iocommon\n");

//     ret = vulkan::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded vkhost\n");

//     ret = ntt::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded entity framework\n");

//     ret = station::initialize(vmroot);
//     if (ext_load_failed(ret)) goto err;
//     printf("SigilVM: loaded station manager\n");

//     //     ret = vmwebhost::initialize(vmroot);
//     //     if (ext_load_failed(ret)) goto err;
//     //    printf("SigilVM: loaded vmwebhost\n");

//     return sigil::VM_OK;

//     err:
//     printf("SigilVM: Failed to load extensions (%s)\n", sigil::status_t_cstr(ret));
//     return ret;
// }