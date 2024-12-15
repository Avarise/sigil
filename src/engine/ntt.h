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
#include "system.h"
#include "utils.h"

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
    
    struct engine_t : sync_data_t {
        sigil::ntt::scene_t *target_scene;

        void sync_engine();
    };

    // VM Tree API
    sigil::status_t initialize();
    sigil::status_t deinitialize();

    scene_t *spawn_scene();
    scene_t *get_scene();

    // Random entity generator
    void generate_entities(uint32_t num_entities, scene_t *target_scene);
}