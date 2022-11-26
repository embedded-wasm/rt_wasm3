
cmake_minimum_required(VERSION 3.12)
include(ExternalProject)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(wasme)

option(WASME_BUILD_WASM3 "Include WASM3 in build" ON)
option(WASME_USE_WASI "Enable WASI" ON)

if(WASME_USE_WASI)
    message("WASI ENABLED")
    set (WASME_ARGS -DWASIENV=1 -DBUILD_WASI=none)
else()
    message("WASI DISABLED")
    set(WASME_ARGS -DBUILD_WASI=none)
endif()

# Setup WASM3 for building / linking if enabled
if(WASME_BUILD_WASM3)
message("WASM3 BUILD ENABLED")


ExternalProject_Add(
    wasm3
    GIT_REPOSITORY https://github.com/ryankurte/wasm3.git
    GIT_TAG fix/wasienv-link-libm
    #GIT_REPOSITORY https://github.com/wasm3/wasm3.git
    #GIT_TAG main
    CMAKE_ARGS ${WASME_ARGS} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_C_COMPILER_WORKS=1 -DCMAKE_CXX_COMPILER_WORKS=1 -DBUILD_NATIVE=off
    UPDATE_COMMAND ""
    INSTALL_COMMAND cp source/libm3.a ${CMAKE_CURRENT_BINARY_DIR}
)

ExternalProject_Get_Property(wasm3 source_dir)
include_directories(${source_dir}/source)

ExternalProject_Get_Property(wasm3 binary_dir)
link_directories(${binary_dir}/source/)
set(WASM3_LIB ${binary_dir}/source/libm3.a)

endif()

# include headers if wasm3 dir is provided
if(WASME_WASM3_DIR)
include_directories(${WASME_WASM3_DIR}/source)
endif()

# Fetch embedded-wasm/spec headers by path or from repo
if(WASME_SPEC_DIR)

message("Using embedded-wasm/spec from: ${WASME_SPEC_DIR}")
# Include spec library directory
include_directories(${WASME_SPEC_DIR}/inc)

else()

message("Using embedded-wasm/spec from: https://github.com/embedded-wasm/spec.git")

# Fetch spec project
ExternalProject_Add(
    spec
    GIT_REPOSITORY https://github.com/embedded-wasm/spec.git
    GIT_TAG main
    CONFIGURE_COMMAND ""
    UPDATE_COMMAND ""
    INSTALL_COMMAND ""
)

# Include spec library directory
ExternalProject_Get_Property(spec source_dir)
include_directories(${source_dir}/inc)

endif()

# Setup library
include_directories(inc)
include_directories(../../ewasm/spec/lib)

set(WASME_SOURCES
    lib/core.c
    lib/i2c.c
    lib/spi.c
    lib/gpio.c
    lib/uart.c
    lib/wasi.c
)

# Build library
# TODO: make m3 optional here
add_library(wasme ${WASME_SOURCES})

if(WASME_BUILD_WASM3)

add_dependencies(wasme wasm3)
target_link_libraries(wasme ${WASM3_LIB} m)
install(FILES ${WASM3_LIB} DESTINATION .)

endif()

install(TARGETS wasme ARCHIVE DESTINATION .)
