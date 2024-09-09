#include "node.h"

namespace sigil {
    struct event_responder_t {

    };

    struct event_t {
        sigil::vmnode_t reporter;

        std::vector<event_responder_t> responder; 
    };
}


