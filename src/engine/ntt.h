#pragma once
/*
    Header for ntt module of SigilVM.
    ntt is work in progress of a general purpose ecs
    General purpose comes as support for multiple types of components and scenes:
        - regular 3d scene with camera
        - data processing scene with data endpoints and scripted entities
*/

#include <sys/types.h>
#include <cstdint>
#include <cstdio>
#include <vector>
#include "../vm/core.h"
#include "../vm/node.h"

namespace sigil::ntt {
    struct entity_t {
        uint32_t uuid;
    };

    struct controller_t {

    };

    struct script_component_t {
        std::string script;
    };

    typedef struct scene_t : sigil::reference_t {
        std::vector<entity_t> entities;
        std::string name;
        bool paused;
    } scene_t;
    
    struct host_data_t {
        std::vector<scene_t*> scenes;
        uint32_t num_engines;
    };

    struct store_data_t {
        //std::vector<sigil::dynamic_mesh_t> dynamic_mesh_store;
        //std::vector<sigil::static_mesh_t> static_mesh_store;
        std::vector<sigil::name_t> name_store;
    };

    // VM Tree API
    sigil::status_t initialize(sigil::vmnode_t *vmsr);
    sigil::vmnode_t probe(sigil::vmnode_t *vmsr);
    sigil::status_t deinitialize();

    scene_t *spawn_scene();
    scene_t *get_scene();

    // Random entity generator
    void generate_entities(uint32_t num_entities, scene_t *target_scene);
}