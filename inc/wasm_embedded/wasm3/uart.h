#ifndef WASME_UART_H
#define WASME_UART_H

#include <stdint.h>

#include "wasm_embedded/uart.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// WASME context forward-declaration
typedef struct wasme_ctx_s wasme_ctx_t;

/// Bind the provided UART driver to the WASM3 module for use
int32_t WASME_bind_uart(wasme_ctx_t* ctx, const uart_drv_t *drv, void* drv_ctx);

#ifdef __cplusplus
}
#endif

#endif
