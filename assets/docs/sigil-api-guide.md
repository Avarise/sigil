### Scripting meta files

    Each directory for which we want custom behaviour, can hold sigil.meta.sh
    file, which will be sourced by these scripts. They source first file found
    within current directory, or parent directory, going up to the fs root.

### SigilVM Topology

    vmsr
    |-- platform
    |   |-- iocommon
    |   |-- station
    |   \-- dev-0-pci
    \-- runtime
        |-- server
        |-- visor
        \-- ntt
            

    vmsr:
        Virtual machine system root, all state changes to it are issued via
        runtime and platform nodes.
    platform:
        Less dynamic part of API responsible for physical host and connectivity.
    runtime:
        Part of API that manages creation and removal of new processes and nodes.
    
    iocommon:
        API for common I/O operation using physical buttons, 4bit lcd,
        uart, signals and x11 events.
    memory:
        Memory information provider and debugger.
    station:
        Manager for platform's network hardware
    vmwebhost:
        Remote shell and http server 

    ntt-host:
        Core of entity framework, uses scenes to manage vm content. By default
        it aggregates runtime and platform data and presents it as system entity.
    ntt-store:
        Optional vmnode of entity framework, required to create new components.
    ntt-physics-engine:
        Physics simulator for entity framework.
    ntt-hat-engine:
        Character generator for entity framework.
    ntt-realm-engine
        World instance runner for games.
    ntt-rpg-extensions
        Set of RPG specific scripts and formats.

    vulkan-host:
        Core of GPU compute API, manages connectivity, and allocation of queues.
    vulkan-renderer:
        Dear ImGui based graphics render API.

    dashboard:
        X11 based GUI.
    
    Nodes of the tree are represented by vmnode_t struct in the components namespace.
    Each one carries predefined functionality, and can register additional data,
    such as AST for command parsing, thread/task descriptor for job scheduling or
    custom state structures.

    To ensure only one root is present within a host, use of DBUS probing is supported
    via sigil::system::probe function.

### SigilVM components

    SigilVM uses C++ namespaces to organize code into:
        Components, which are type definitions.
        Engines, which continuously run over specified set of components.
    Both of those can be reflected upon by runtime module "ntt".
    Additionally, ntt-store together with ntt-host can create new components and
    compose them into entities, for purposes of simulation, debugging, and app-creation. 


    sigil::components::name
        String based name tag. 

    sigil::components::vmnode_t
        Arranged into tree, vmnode is a container for application modules.

    sigil::components::ref_t
        Void type data pointer with additional fields to track size.

    sigil::components::window
        All in one type for glfw, Vulkan, x11.
    
    sigil::components::scene
        Scene is organization unit for entity system.
        Scenes can have different subtypes, which ensure that certain components
        exist within that scene:
            - Blank Scene: has no components
            - Render Scene: ensures simple camera
            - Compute Scene: ensures that a data sink and source exist

    sigil::components::script
        Executable set of commands for the VM and its modules.

    sigil::components::parser
        Can take in plain text command and other text based sources and execute them.
        Parser builds a tree of command handlers, which modules can register.

    sigil::components::event
        Wrapper for signals. Event has a list of handlers called whenever event
        is triggered.
        
    sigil::components::controller
        Script-like component, that attaches to an entity and can issue commands.
        For example, player character in game would redirect keyboard input using
        a controller, and NPC would redirect AI commands via the same mechanism.

    sigil::components::transform3d
        Contains translation, rotation and scaling descriptors.
        Transform is used by camera and meshes.

    sigil::components::camera
        Component to project scenes using render module.

    sigil::components::light3d
        Dynamic 3d light.

    sigil::components::light2d
        Dynamic 2d light.

    sigil::components::mesh
        3d mesh that can be attached to scene or skeleton.
     
    sigil::components::skeleton
        Tree of mesh attachment points.

    sigil::components::texture
        2d image component, that covers mesh.

    sigil::components::material
        Normal vector that influences surface properties like reflectivity,
        roughness etc.

    sigil::components::sprite
        2d image component drawn upon window

    sigil::components::color_t
        RGB container.

    sigil::components::wifi_t
        WiFi descriptor, can optionally hold PSK.

    sigil::components::rng
        Random number generator.

    sigil::system::probe
    sigil::system::init
    sigil::system::reload
    sigil::system::shutdown

### NTT Viewer

    [0] - VMSR
    [1] - Platform
    [2] - Runtime

### Sigil Scene Editor: UX/UI

    Main window bar, at the top, can be hidden with a shortcut. (not chosen yet)

    File:
        new
        open
        save
        save as

    Entity Framework (Paged)
    1 - Tree View
    2 - Ref ID Table
    3 - Entity Store

    Main Window (Paged)
    1 - Text Editor with vim motions support + terminal
    2 - Alternate Editor mode: asset editor with support for 3d and 
        4d assets + hex mode memory view style
    3 - Info View like size in memory

    Output Capture:
        Mirror stdout/stderr and other outputs. (with scroll)

    Peek View:
        Shows most relevant info about component or entity. 

