#ifndef WASME_GPIO_H
#define WASME_GPIO_H

#include <stdint.h>

#include "wasm_embedded/gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/// WASME context forward-declaration
typedef struct wasme_ctx_s wasme_ctx_t;

/// Bind the provided GPIO driver to the WASM3 module for use
int32_t WASME_bind_gpio(wasme_ctx_t* ctx, const gpio_drv_t *drv, void* drv_ctx);

#endif
