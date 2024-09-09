#include "core.h"

namespace sigil {
    namespace parser {
        struct command_t {
            std::vector<std::string> body;
        };

        struct syntax_node {

        };

        // Translator is a root of a tree, if given empty translator, it initializes
        // a new translation unit
        sigil::status_t register_command(syntax_node &translator, command_t command);
        sigil::status_t unregister_command(syntax_node &translator, command_t command);


        sigil::status_t exec(const syntax_node &translator, std::string &source);
        sigil::status_t append_ast(syntax_node &translator, syntax_node &new_tree);
    }
}