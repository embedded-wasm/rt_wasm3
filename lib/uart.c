
#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "m3_info.h"
#include "extra/wasi_core.h"

#include "wasm_embedded/wasm3/uart.h"
#include "wasm_embedded/wasm3/internal.h"

#define TAG "WASME_UART"

#define WASME_DEBUG_UART

// Debug print helper
#ifdef WASME_DEBUG_UART
#define WASME_UART_DEBUG_PRINTF(...) if (uart_debug) printf(__VA_ARGS__);
#else
#define WASME_UART_DEBUG_PRINTF(...)
#endif

// Shared UART driver objects
// TODO: work out how to bind these to wasm3 context
static const uart_drv_t* uart_drv = NULL;
static const void* uart_drv_ctx = NULL;

// UART debug logging flag
static bool uart_debug = false;



m3ApiRawFunction(m3_uart_init)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, dev)
    m3ApiGetArg      (uint32_t, baud)
    m3ApiGetArg      (int32_t, tx)
    m3ApiGetArg      (int32_t, rx)
    m3ApiGetArgMem   (uint32_t*, handle)


    WASME_UART_DEBUG_PRINTF("UART init port: %d freq: %d tx: %d rx: %d\r\n", dev, baud, tx, rx);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!uart_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!uart_drv->init) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = uart_drv->init(uart_drv_ctx, dev, baud, tx, rx);

    if(res >= 0) {
        *handle = res;
        WASME_UART_DEBUG_PRINTF("UART handle: %d\r\n", res);
    }

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_uart_deinit)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)

    WASME_UART_DEBUG_PRINTF("UART deinit handle: %d\r\n", handle);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!uart_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!uart_drv->deinit) { m3ApiReturn(__WASI_ERRNO_NOENT); }


    int32_t res = uart_drv->deinit(uart_drv_ctx, handle);

    m3ApiReturn(res);
}

typedef struct {
    uint8_t* data;
    uint32_t len;
} buff_t;

m3ApiRawFunction(m3_uart_write)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, flags)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_UART_DEBUG_PRINTF("UART write data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!uart_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!uart_drv->write) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    WASME_UART_DEBUG_PRINTF("UART write port: %d flags: 0x%x, %d bytes (%p)\r\n", handle, flags, *len, data);

    int32_t res = uart_drv->write(uart_drv_ctx, handle, flags, data, *len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_uart_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, flags)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch kength, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_UART_DEBUG_PRINTF("UART read data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!uart_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!uart_drv->read) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = uart_drv->read(uart_drv_ctx, handle, flags, data, *len);

    WASME_UART_DEBUG_PRINTF("UART read port: %d flags: 0x%x, %d bytes (%p)\r\n", handle, flags, *len, data);

    m3ApiReturn(res);
}


const static char* wasme_uart_mod = "uart";

int32_t WASME_bind_uart(wasme_ctx_t* ctx, const uart_drv_t* drv, void* drv_ctx) {
    M3Result m3_res;

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_uart_mod, "init", "i(iiiii)", &m3_uart_init);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_uart_mod, "deinit", "i(i)", &m3_uart_deinit);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_uart_mod, "write", "i(iii)", &m3_uart_write);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_uart_mod, "read", "i(iii)", &m3_uart_read);
    
    uart_drv = drv;
    uart_drv_ctx = drv_ctx;
    
    return 0;
}
