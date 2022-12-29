//! WASME type header for internal use only
#ifndef WASME_INT_H
#define WASME_INT_H

#include "wasm3.h"
#include "core.h"

// Default maximum number of bound drivers
#ifndef CORE_MAX_DRIVERS
#define CORE_MAX_DRIVERS 16
#endif


/// @brief Core driver instance
typedef struct wasme_driver_s {
    /// @brief Class for matching drivers and syscalls
    uint32_t class;

    /// @brief Handle function for executing syscalls
    core_driver_handle_f handle;

    /// @brief Context for driver handler
    void* ctx;

} wasme_driver_t;

/// @brief Concrete WASME context type
struct wasme_ctx_s {
    IM3Environment env;
    IM3Runtime rt;
    IM3Module mod;

    // Driver bindings
    wasme_driver_t drivers[CORE_MAX_DRIVERS];

    // Number of bound drivers
    uint32_t num_drivers;
};

#endif
