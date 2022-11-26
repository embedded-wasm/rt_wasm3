
#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "m3_info.h"
#include "extra/wasi_core.h"

#include "wasm_embedded/wasm3/i2c.h"
#include "wasm_embedded/wasm3/internal.h"

#define TAG "WASME_I2C"

#define WASME_DEBUG_I2C

// Debug print helper
#ifdef WASME_DEBUG_I2C
#define WASME_I2C_DEBUG_PRINTF(...) if (i2c_debug) printf(__VA_ARGS__);
#else
#define WASME_I2C_DEBUG_PRINTF(...)
#endif

// Shared I2C driver objects
// TODO: work out how to bind these to wasm3 context
static const i2c_drv_t* i2c_drv = NULL;
static const void* i2c_drv_ctx = NULL;

// I2C debug logging flag
static bool i2c_debug = false;



m3ApiRawFunction(m3_i2c_init)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, dev)
    m3ApiGetArg      (uint32_t, baud)
    m3ApiGetArg      (int32_t, sda)
    m3ApiGetArg      (int32_t, scl)
    m3ApiGetArgMem   (uint32_t*, handle)


    WASME_I2C_DEBUG_PRINTF("I2C init port: %d freq: %d sda: %d scl: %d\r\n", dev, baud, sda, scl);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!i2c_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!i2c_drv->init) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = i2c_drv->init(i2c_drv_ctx, dev, baud, sda, scl);

    if(res >= 0) {
        *handle = res;
        WASME_I2C_DEBUG_PRINTF("I2C handle: %d\r\n", res);
    }

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_deinit)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)

    WASME_I2C_DEBUG_PRINTF("I2C deinit handle: %d\r\n", handle);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!i2c_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!i2c_drv->deinit) { m3ApiReturn(__WASI_ERRNO_NOENT); }


    int32_t res = i2c_drv->deinit(i2c_drv_ctx, handle);

    m3ApiReturn(res);
}

typedef struct {
    uint8_t* data;
    uint32_t len;
} buff_t;

m3ApiRawFunction(m3_i2c_write)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint16_t, addr)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_I2C_DEBUG_PRINTF("I2C write data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!i2c_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!i2c_drv->write) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    WASME_I2C_DEBUG_PRINTF("I2C write port: %d addr: 0x%x, %d bytes (%p)\r\n", handle, addr, *len, data);

    int32_t res = i2c_drv->write(i2c_drv_ctx, handle, addr, data, *len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint16_t, addr)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch kength, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_I2C_DEBUG_PRINTF("I2C read data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!i2c_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!i2c_drv->read) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = i2c_drv->read(i2c_drv_ctx, handle, addr, data, *len);

    WASME_I2C_DEBUG_PRINTF("I2C read port: %d addr: 0x%x, %d bytes (%p)\r\n", handle, addr, *len, data);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_i2c_write_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint16_t, addr)
    m3ApiGetArg      (uint32_t, out_ptr);
    m3ApiGetArg      (uint32_t, in_ptr);

    // Resolve relative buffer object pointer
    uint32_t* out_p = m3ApiOffsetToPtr(out_ptr);
    uint8_t* data_out = m3ApiOffsetToPtr(*out_p);
    uint32_t* len_out = m3ApiOffsetToPtr(out_ptr+4);

    uint32_t* in_p = m3ApiOffsetToPtr(in_ptr);
    uint8_t* data_in = m3ApiOffsetToPtr(*in_p);
    uint32_t* len_in = m3ApiOffsetToPtr(in_ptr+4);

    WASME_I2C_DEBUG_PRINTF("I2C write_read data_out: %p len_out: %d data_in: %p len_in: %d\r\n", 
        data_out, *len_out, data_in, *len_in);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!i2c_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!i2c_drv->write_read) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = i2c_drv->write_read(i2c_drv_ctx, handle, addr, data_out, *len_out, data_in, *len_in);

    WASME_I2C_DEBUG_PRINTF("I2C write_read port: %d addr: %x, out: %p %d bytes, in: %p %d bytes\r\n", handle, addr, data_out, *len_out, data_in, *len_in);


    m3ApiReturn(res);
}

const static char* wasme_i2c_mod = "i2c";

int32_t WASME_bind_i2c(wasme_ctx_t* ctx, const i2c_drv_t* drv, void* drv_ctx) {
    M3Result m3_res;

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_i2c_mod, "init", "i(iiiii)", &m3_i2c_init);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_i2c_mod, "deinit", "i(i)", &m3_i2c_deinit);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_i2c_mod, "write", "i(iii)", &m3_i2c_write);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_i2c_mod, "read", "i(iii)", &m3_i2c_read);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_i2c_mod, "write_read", "i(iiii)", &m3_i2c_write_read);
    
    i2c_drv = drv;
    i2c_drv_ctx = drv_ctx;
    
    return 0;
}
