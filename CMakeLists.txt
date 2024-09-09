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

# Support for clangd in VSCode
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

# Paths
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include")
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/src/extern/imgui)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/src/extern/glfw)
set(TEST_DIR ${CMAKE_SOURCE_DIR}/src/tests)
set(APPS_DIR ${CMAKE_SOURCE_DIR}/src/apps)
set(VM_DIR ${CMAKE_SOURCE_DIR}/src/vm)
set(SIGIL_HEADER ${CMAKE_SOURCE_DIR}/src/sigil.h)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
include_directories(${GLFW_DIR}/include)
include_directories(${GLFW_DIR}/deps)
#add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)

set(IMGUI_BACKENDS ${IMGUI_DIR}/backends/imgui_impl_glfw.h ${IMGUI_DIR}/backends/imgui_impl_vulkan.h) 
set(IMGUI_BACKENDS_SRC ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp) 
file(GLOB SRC_IMGUI ${IMGUI_DIR}/*.cpp) 

# File globs, to be replaced by manual lists in future 
file(GLOB HEADERS_ENGINE ${CMAKE_SOURCE_DIR}/src/engine/*.h)
file(GLOB HEADERS_RENDER ${CMAKE_SOURCE_DIR}/src/render/*.h)
file(GLOB HEADERS_SOUND ${CMAKE_SOURCE_DIR}/src/sound/*.h)
file(GLOB HEADERS_UTILS ${CMAKE_SOURCE_DIR}/src/utils/*.h)
file(GLOB HEADERS_NET ${CMAKE_SOURCE_DIR}/src/net/*.h)
file(GLOB HEADERS_VM ${CMAKE_SOURCE_DIR}/src/vm/*.h)
file(GLOB SRC_GLFW ${CMAKE_SOURCE_DIR}/src/extern/glfw/*.cpp)
file(GLOB SRC_ENGINE ${CMAKE_SOURCE_DIR}/src/engine/*.cpp)
file(GLOB SRC_RENDER ${CMAKE_SOURCE_DIR}/src/render/*.cpp)
file(GLOB SRC_SOUND ${CMAKE_SOURCE_DIR}/src/sound/*.cpp)
file(GLOB SRC_UTILS ${CMAKE_SOURCE_DIR}/src/utils/*.cpp)
file(GLOB SRC_NET ${CMAKE_SOURCE_DIR}/src/net/*.cpp)
file(GLOB SRC_VM ${CMAKE_SOURCE_DIR}/src/vm/*.cpp)
set(SRC_GLOB_ALL ${SRC_SOUND} ${SRC_UTILS} ${SRC_NET} ${SRC_VM} ${SRC_RENDER} ${SRC_ENGINE} ${SRC_GLFW} ${SRC_IMGUI} ${IMGUI_BACKENDS_SRC})

# Build target: sigil library
add_library(sigil SHARED ${SRC_VM} ${SRC_UTILS})
set_target_properties(sigil PROPERTIES
   VERSION ${Sigil_VERSION}
   PUBLIC_HEADER "${SIGIL_HEADER}"
)

# Build target: SigilVM Library
install(TARGETS sigil LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include/sigil)
configure_file(sigil.pc.in sigil.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/sigil.pc DESTINATION lib/pkgconfig)

# Build target: sigil-tools
add_executable(sigil-tools ${APPS_DIR}/tools.cpp  ${SRC_GLOB_ALL})
target_link_libraries(sigil-tools ${LIBRARIES})
target_compile_definitions(sigil-tools PUBLIC -DSIGIL_DEBUG_MODE)

# Build target: xorit
add_executable(xorit ${APPS_DIR}/xorit.cpp  ${SRC_UTILS})
target_link_libraries(xorit ${LIBRARIES})
target_compile_definitions(xorit PUBLIC -DSIGIL_DEBUG_MODE)

# Build target: gbgen
add_executable(gbgen ${APPS_DIR}/gbgen.cpp  ${SRC_UTILS})
target_link_libraries(gbgen ${LIBRARIES})
target_compile_definitions(gbgen PUBLIC -DSIGIL_DEBUG_MODE)

# Test target: initialization
add_executable(gtest-initialization ${TEST_DIR}/initialization.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-initialization ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-initialization PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME InitializationTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-initialization)

# Test target: networking
add_executable(gtest-networking ${TEST_DIR}/networking.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-networking ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-networking PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME NetworkingTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-networking)

# Test target: graphics
add_executable(gtest-graphics ${TEST_DIR}/parser.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-graphics ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-graphics PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME GraphicsTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-graphics)

# Test target: parser
add_executable(gtest-parser ${TEST_DIR}/parser.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-parser ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-parser PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME ParserTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-parser)

# Test target: performance
add_executable(gtest-performance ${TEST_DIR}/performance.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-performance ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-performance PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME PerformanceTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-performance)

# Test target: security
add_executable(gtest-security ${TEST_DIR}/security.cpp ${SRC_GLOB_ALL})
target_link_libraries(gtest-security ${LIBRARIES} gtest gtest_main)
target_compile_definitions(gtest-security PUBLIC -DImTextureID=ImU64 -DSIGIL_USE_GUI)
add_test(NAME SecurityTestSuite COMMAND ${CMAKE_BINARY_DIR}/gtest-security)
