/** \file wasme.c
 * Embedded WASM Core API
 */

#ifndef WASME_H
#define WASME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief embedded-wasm wasm3 context
typedef struct wasme_ctx_s wasme_ctx_t;


/// @brief binary wasm task
typedef struct {
    const uint8_t* data;
    uint32_t data_len;
} wasme_task_t;


/// @brief Driver exec function, implemented by drivers to handle syscalls
typedef int32_t (*core_driver_handle_f)(
    uint32_t ins, uint32_t flags,
    int32_t handle,
    uint8_t* cmd_buff, uint32_t cmd_len,
    uint8_t* resp_buff, uint32_t resp_len);


// ANCHOR: core_api

/// Intialise WASME ctx with the provided task
wasme_ctx_t* WASME_init(const wasme_task_t* task, uint32_t mem_limit);

/// Bind driver to WASME ctx
int WASME_bind(wasme_ctx_t* ctx, uint32_t cla, core_driver_handle_f drv, void* drv_ctx);

/// Execute the named function
int WASME_run(wasme_ctx_t* ctx, const char* name, int32_t argc, const char** argv);

/// De-initialise a WASME instance
void WASME_deinit(wasme_ctx_t** ctx);

// ANCHOR_END: core_api


#ifdef __cplusplus
}
#endif

#endif
