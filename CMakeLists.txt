# CMake core
cmake_minimum_required(VERSION 3.10)
set(Sigil_VERSION 1.0.0)
project(SigilVM VERSION ${Sigil_VERSION}
   DESCRIPTION "SigilVM"
)

if(NOT CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Testing with gtest
enable_testing()
# Register custom functions
# add_dependencies(update_bashrc install_config)
set(CMAKE_CTEST_ARGUMENTS --output-on-failure)

# Support for clangd in VSCode clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")








# Libraries
find_package(Vulkan REQUIRED)
set(LIBRARIES "glfw;Vulkan::Vulkan")
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)

# Project Paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")
set(SIGIL_RC_USER "$ENV{HOME}/.config/sigilrc")
set(SIGIL_RC ${CMAKE_SOURCE_DIR}/assets/configs/sigilrc)
set(BASHRC "$ENV{HOME}/.bashrc")
set(ZSHRC "$ENV{HOME}/.zshrc")
# Find the running kernel's build directory
set(KERNEL_HEADERS "/lib/modules/${CMAKE_SYSTEM_PROCESSOR}/build")

# SigilVM source paths
set(DIR_VM_TESTING ${CMAKE_SOURCE_DIR}/src/tests) # Excluded from installed library, TODO: move some testing to .h files
set(DIR_VM_ENGINE ${CMAKE_SOURCE_DIR}/src/engine)
set(DIR_VM_RENDER ${CMAKE_SOURCE_DIR}/src/render)
set(DIR_VM_SOUND ${CMAKE_SOURCE_DIR}/src/sound)
set(DIR_VM_APPS ${CMAKE_SOURCE_DIR}/src/apps) # Excluded from installed library
set(DIR_VM_GAME ${CMAKE_SOURCE_DIR}/src/game)
set(DIR_VM_CORE ${CMAKE_SOURCE_DIR}/src/core)
set(DIR_VM_NET ${CMAKE_SOURCE_DIR}/src/net)

# SigilVM library headers *.h
file(GLOB HEADERS_ENGINE ${DIR_VM_ENGINE}/*.h)
file(GLOB HEADERS_RENDER ${DIR_VM_RENDER}/*.h)
file(GLOB HEADERS_SOUND ${DIR_VM_SOUND}/*.h)
file(GLOB HEADERS_CORE ${DIR_VM_CORE}/*.h)
file(GLOB HEADERS_GAME ${DIR_VM_GAME}/*.h)
file(GLOB HEADERS_NET ${DIR_VM_NET}/*.h)

# SigilVM library source *.cpp
file(GLOB SRC_ENGINE ${DIR_VM_ENGINE}/*.cpp)
file(GLOB SRC_RENDER ${DIR_VM_RENDER}/*.cpp)
file(GLOB SRC_SOUND ${DIR_VM_SOUND}/*.cpp)
file(GLOB SRC_CORE ${DIR_VM_CORE}/*.cpp)
file(GLOB SRC_GAME ${DIR_VM_GAME}/*.cpp)
file(GLOB SRC_NET ${DIR_VM_NET}/*.cpp)

# SigilVM includes
include_directories(${DIR_VM_RENDER})
include_directories(${DIR_VM_ENGINE})
include_directories(${DIR_VM_SOUND})
include_directories(${DIR_VM_CORE})
include_directories(${DIR_VM_GAME})
include_directories(${DIR_VM_NET})
# Add include directories
include_directories(${KERNEL_HEADERS}/include)


# External paths
set(DIR_IMGUI ${CMAKE_SOURCE_DIR}/src/extern/imgui)
set(DIR_GLFW ${CMAKE_SOURCE_DIR}/src/extern/glfw)
set(DIR_IMGUI_BACKENDS ${CMAKE_SOURCE_DIR}/src/extern/imgui/backends)
# External library components
set(HEADERS_IMGUI_BACKENDS ${DIR_IMGUI_BACKENDS}/imgui_impl_glfw.h ${DIR_IMGUI_BACKENDS}/imgui_impl_vulkan.h) 
set(SRC_IMGUI_BACKENDS ${DIR_IMGUI_BACKENDS}/imgui_impl_glfw.cpp ${DIR_IMGUI_BACKENDS}/imgui_impl_vulkan.cpp) 
file(GLOB SRC_GLFW ${CMAKE_SOURCE_DIR}/src/extern/glfw/*.cpp)
file(GLOB SRC_IMGUI ${DIR_IMGUI}/*.cpp) 

# Include directories for shorter includes
# Will be generated into compile_commands.json for VSCode clangd
include_directories(${DIR_IMGUI})
include_directories(${DIR_IMGUI_BACKENDS})
include_directories(${DIR_GLFW}/include)
include_directories(${DIR_GLFW}/deps)

# TODO: Add SigilVM as library for Linux
# Build target: sigil library
# add_library(sigil SHARED ${SRC_VM} ${SRC_UTILS})
# set_target_properties(sigil PROPERTIES
#    VERSION ${Sigil_VERSION}
#    PUBLIC_HEADER "${DIR_VM_CORE}"
# )

# Build target: SigilVM Library
# install(TARGETS sigil LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/sigil)
# configure_file(sigil.pc.in sigil.pc @ONLY)
# install(FILES ${CMAKE_BINARY_DIR}/sigil.pc DESTINATION lib/pkgconfig)

# Build target: xorit
add_executable(xorit ${DIR_VM_APPS}/xorit.cpp  ${SRC_CORE})
target_link_libraries(xorit ${LIBRARIES})
target_compile_definitions(xorit PUBLIC -DSIGIL_DEBUG_MODE)

# Build target: gbgen
add_executable(gbgen ${DIR_VM_APPS}/gbgen.cpp)
target_link_libraries(gbgen ${LIBRARIES})
target_compile_definitions(gbgen PUBLIC)

#Build target: sigil-tools
add_executable(sigil-tools ${DIR_VM_APPS}/tools.cpp  ${SRC_CORE} ${SRC_ENGINE} ${SRC_NET} ${SRC_RENDER} ${SRC_IMGUI} ${SRC_IMGUI_BACKENDS})
target_link_libraries(sigil-tools ${LIBRARIES})
target_compile_definitions(sigil-tools PUBLIC -DSIGIL_DEBUG_MODE -DSIGIL_USE_GUI)

# Test target: initialization
add_executable(gtest-initialization ${DIR_VM_TESTING}/initialization.cpp ${SRC_CORE} ${SRC_NET})
target_link_libraries(gtest-initialization ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-initialization PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME InitializationTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-initialization )

# Test target: networking
add_executable(gtest-networking ${DIR_VM_TESTING}/networking.cpp ${SRC_CORE} ${SRC_NET})
target_link_libraries(gtest-networking ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-networking PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME NetworkingTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-networking)

# Test target: graphics
add_executable(gtest-graphics ${DIR_VM_TESTING}/parser.cpp ${SRC_CORE})
target_link_libraries(gtest-graphics ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-graphics PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME GraphicsTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-graphics)

# Test target: parser
add_executable(gtest-parser ${DIR_VM_TESTING}/parser.cpp ${SRC_CORE})
target_link_libraries(gtest-parser ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-parser PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME ParserTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-parser)

# Test target: performance
add_executable(gtest-performance ${DIR_VM_TESTING}/performance.cpp ${SRC_CORE})
target_link_libraries(gtest-performance ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-performance PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME PerformanceTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-performance)

# Test target: security
add_executable(gtest-security ${DIR_VM_TESTING}/security.cpp ${SRC_CORE})
target_link_libraries(gtest-security ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-security PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME SecurityTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-security)


# # Add the module source file
# add_library(sigilvmmod MODULE src/kernel/ksigil.c)

# # Set compilation flags
# target_compile_options(sigilvmmod PRIVATE -Wall -Wextra -Werror)

# # Ensure we're building against the kernel headers
# target_include_directories(sigilvmmod PRIVATE ${KERNEL_HEADERS}/include)




# Install step: Copy file
add_custom_target(install_config
   COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SIGIL_RC} ${SIGIL_RC_USER}
   COMMENT "Copying Sigil configuration file to ${SIGIL_RC_USER}"
)

# add_custom_target(update_bashrc ALL
#    COMMAND /bin/sh -c "[ -f ${DEST_FILE} ] && ! grep -qxF '# Sigil configuration' ${BASHRC} && echo '\n# Sigil configuration\n[ -f ${DEST_FILE} ] && source ${DEST_FILE}' >> ${BASHRC} || true"
#    COMMENT "Ensuring ~/.sigil is sourced in ~/.bashrc"
#    VERBATIM
# )

