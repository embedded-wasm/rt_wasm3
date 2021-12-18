#ifndef WASME_SPI_H
#define WASME_SPI_H

#include <stdint.h>

#include "wasm_embedded/spi.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// WASME context forward-declaration
typedef struct wasme_ctx_s wasme_ctx_t;

/// Bind the provided SPI driver to the WASM3 module for use
int32_t WASME_bind_spi(wasme_ctx_t* ctx, const spi_drv_t *drv, void* drv_ctx);

#ifdef __cplusplus
}
#endif

#endif
