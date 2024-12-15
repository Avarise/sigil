#pragma once
#include "utils.h"

namespace sigil::sound {
    // VM Tree API
    status_t initialize();
    status_t deinitialize();

    // Sound player api
    status_t add_track(const char* file_path);
    status_t remove_tracks();
    // Common sound track controls
    status_t start();
    status_t pause();
    status_t stop();
    // TODO: Use this for scripted sounds, like clicks or ingame sounds
    status_t play_on_demand();
}