### Usage
    git submodule update --init --recursive
    ./src/scripts/build.sh

### TODO:
    Change module model:
    Use C style: focus on structures instead of classes.
    Modules are wrapped within namespace: sigil::system, sigil::visor, sigil::station etc
    Use function calls defined in headers as public API
    Define only required structures there, keep module data in .cpp file
    Handle data initialization and cleanup there.
    Each module (besides root) is required to have public calls for start() and stop()
    We dont create/destroy them, they are hard compiled in, but not started(initialized)
    Starting/stopping will allocate descriptors, initialize subsystems etc.
    start and stop might return VM_SKIPPED if desired state is already there.
    These calls are available for static and dynamic usage (function pointers are stored in parent struct)
    Some additional calls for modules
    
    VM Tree:
    references and initialization status in runtime
    acts as core for command parsing, VM tree has parser shadow tree, in parser.h
    create a global command root, attached to root node_t a kind of runtime namespace 
    Introduce module names as enum within core/system.h in addition to string name

    Add watchdog module:
    Periodically goes over vm tree and checks if data there is valid, for example if all
    nodes have start/stop functions, check cookies and canaries, maybe do some
    garbage collection like stopping a module if it is not used

    