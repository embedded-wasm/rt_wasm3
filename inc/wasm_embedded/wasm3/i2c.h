#ifndef WASME_I2C_H
#define WASME_I2C_H

#include <stdint.h>

#include "wasm_embedded/i2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// WASME context forward-declaration
typedef struct wasme_ctx_s wasme_ctx_t;

/// Bind the provided I2C driver to the WASM3 module for use
int32_t WASME_bind_i2c(wasme_ctx_t* ctx, const i2c_drv_t *drv, void* drv_ctx);

#ifdef __cplusplus
}
#endif

#endif
