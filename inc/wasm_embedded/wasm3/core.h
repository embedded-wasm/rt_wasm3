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

typedef struct wasme_ctx_s wasme_ctx_t;

typedef struct {
    const char* name;
    const uint8_t* data;
    uint32_t data_len;
} wasme_task_t;

// ANCHOR: core_api
/// Intialise WASME ctx with the provided task
wasme_ctx_t* WASME_init(const wasme_task_t* task, uint32_t mem_limit);

/// Execute the specified function
int WASME_run(wasme_ctx_t* ctx, const char* name, int32_t argc, const char** argv);

/// De-initialise a WASME instance
void WASME_deinit(wasme_ctx_t** ctx);
// ANCHOR_END: core_api


#ifdef __cplusplus
}
#endif

#endif
