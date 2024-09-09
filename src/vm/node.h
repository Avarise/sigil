#pragma once
#include <mutex>
#include "core.h"
#include "parser.h"


namespace sigil {
    typedef status_t(*node_deinit_ft)(void); 


    struct vmnode_t : reference_t {
        sigil::name_t name;
        vmnode_t *master_node;
        std::vector<vmnode_t*> subnodes;
        uint8_t depth_at_tree;
        parser::syntax_node *syntax_node;
        std::mutex node_mutex;

        struct node_data_t {
            void *data;
            //node_deinit_ft data_cleanup;
            void (*data_cleanup)(void);
        } node_data;

        sigil::status_t set_data(void *data, void (*data_cleanup)(void)) {
            if (!data || !data_cleanup) return sigil::VM_ARG_NULL;
            this->node_data.data = data;
            this->node_data.data_cleanup = data_cleanup;
            return sigil::VM_OK;
        }

        vmnode_t();
        ~vmnode_t();
        vmnode_t(const char *name);
        vmnode_t(const sigil::name_t name);
        sigil::status_t deinit();
        sigil::status_t deinit_self();
        sigil::status_t deinit_subnodes();
        vmnode_t* search(const char *name, int depth_current, int depth_max);
        vmnode_t* spawn_subnode();
        vmnode_t* spawn_subnode(const char *name);
        vmnode_t* peek_subnode(const char *name, int depth_max);
        vmnode_t* peek_master_node();
        vmnode_t* get_subnode(const char *name, int depth_max);
        vmnode_t* get_master_node();
        vmnode_t* get_root_node();
        void release();
        std::string get_node_name();
        std::string get_node_name_tree();
        void print_nodeinfo();
        void print_nodemem();
    };
}