

#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "m3_info.h"
#include "extra/wasi_core.h"

#include "wasm_embedded/wasm3/gpio.h"
#include "wasm_embedded/wasm3/internal.h"

#define TAG "WASME_GPIO"


#define WASME_DEBUG_GPIO

// Debug print helper
#ifdef WASME_DEBUG_GPIO
#define WASME_GPIO_DEBUG_PRINTF(...) if(gpio_debug) printf(__VA_ARGS__);
#else
#define WASME_GPIO_DEBUG_PRINTF(...)
#endif

// Shared I2C driver objects
// TODO: work out how to bind these to wasm3 context
static const gpio_drv_t* gpio_drv = NULL;
static const void* gpio_drv_ctx = NULL;

// GPIO debug print flag
static bool gpio_debug = false;


m3ApiRawFunction(m3_gpio_init)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, pin)
    m3ApiGetArg      (uint32_t, port)
    m3ApiGetArg      (int32_t, mode)
    m3ApiGetArgMem   (uint32_t*, handle)


    WASME_GPIO_DEBUG_PRINTF("GPIO init port: %d pin: %d mode: %d\r\n", port, pin, mode);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!gpio_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!gpio_drv->init) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = gpio_drv->init(gpio_drv_ctx, port, pin, mode);

    if(res >= 0) {
        *handle = res;
        WASME_GPIO_DEBUG_PRINTF("GPIO handle: %d\r\n", res);
    }

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_gpio_deinit)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, handle)

    WASME_GPIO_DEBUG_PRINTF("GPIO deinit handle: %d\r\n", handle);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!gpio_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!gpio_drv->deinit) { m3ApiReturn(__WASI_ERRNO_NOENT); }


    int32_t res = gpio_drv->deinit(gpio_drv_ctx, handle);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_gpio_set)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, value);

    WASME_GPIO_DEBUG_PRINTF("GPIO set handle: %d value: %u \r\n", handle, value);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!gpio_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!gpio_drv->set) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = gpio_drv->set(gpio_drv_ctx, handle, value);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_gpio_get)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, value)

    // Resolve value to pointer
    uint32_t* v = m3ApiOffsetToPtr(value);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!gpio_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!gpio_drv->get) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = gpio_drv->get(gpio_drv_ctx, handle, v);

    WASME_GPIO_DEBUG_PRINTF("GPIO get handle: %d value: %u \r\n", handle, *v);

    m3ApiReturn(res);
}

const static char* wasme_gpio_mod = "gpio";

int32_t WASME_bind_gpio(wasme_ctx_t* ctx, const gpio_drv_t* drv, void* drv_ctx) {
    M3Result m3_res;

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_gpio_mod, "init", "i(iii)", &m3_gpio_init);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_gpio_mod, "deinit", "i(i)", &m3_gpio_deinit);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_gpio_mod, "set", "i(ii)", &m3_gpio_set);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_gpio_mod, "get", "i(ii)", &m3_gpio_get);
    
    gpio_drv = drv;
    gpio_drv_ctx = drv_ctx;
    
    return 0;
}
