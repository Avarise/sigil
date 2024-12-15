#pragma once
#include <vector>
#include "utils.h"

namespace sigil::parser {
    struct command_t {
        std::vector<std::string> body;
    };

    struct syntax_node {

    };

    sigil::status_t register_command(syntax_node &translator, command_t command);
    sigil::status_t unregister_command(syntax_node &translator, command_t command);
    sigil::status_t join_translators(syntax_node &translator, syntax_node &new_translator);
}