
#include "wasm3.h"
#include "m3_env.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "m3_info.h"
#include "extra/wasi_core.h"

#include "wasm_embedded/wasm3/spi.h"
#include "wasm_embedded/wasm3/internal.h"

#define TAG "WASME_SPI"


#define WASME_DEBUG_SPI

// Debug print helper
#ifdef WASME_DEBUG_SPI
#define WASME_SPI_DEBUG_PRINTF(...) if(spi_debug) printf(__VA_ARGS__);
#else
#define WASME_SPI_DEBUG_PRINTF(...)
#endif

// Shared I2C driver objects
// TODO: work out how to bind these to wasm3 context
static const spi_drv_t* spi_drv = NULL;
static const void* spi_drv_ctx = NULL;

// SPI debug logging flag
static bool spi_debug = false;


m3ApiRawFunction(m3_spi_init)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, dev)
    m3ApiGetArg      (uint32_t, baud)
    m3ApiGetArg      (int32_t, mosi)
    m3ApiGetArg      (int32_t, miso)
    m3ApiGetArg      (int32_t, sck)
    m3ApiGetArg      (int32_t, cs)
    m3ApiGetArgMem   (uint32_t*, handle)


    WASME_SPI_DEBUG_PRINTF("SPI init port: %d freq: %d mosi: %d miso: %d sck: %d cs: %d\r\n",
            dev, baud, mosi, miso, sck, cs);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->init) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = spi_drv->init(spi_drv_ctx, dev, baud, mosi, miso, sck, cs);

    if(res >= 0) {
        *handle = res;
        WASME_SPI_DEBUG_PRINTF("SPI handle: %d\r\n", res);
    }

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_spi_deinit)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, handle)

    WASME_SPI_DEBUG_PRINTF("SPI deinit handle: %d\r\n", handle);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->deinit) { m3ApiReturn(__WASI_ERRNO_NOENT); }


    int32_t res = spi_drv->deinit(spi_drv_ctx, handle);

    m3ApiReturn(res);
}

typedef struct {
    uint8_t* data;
    uint32_t len;
} buff_t;

m3ApiRawFunction(m3_spi_read)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_SPI_DEBUG_PRINTF("SPI read data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->write) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    WASME_SPI_DEBUG_PRINTF("SPI read port: %d, %d bytes (%p)\r\n", handle, *len, data);

    int32_t res = spi_drv->read(spi_drv_ctx, handle, data, *len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_spi_write)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_SPI_DEBUG_PRINTF("SPI write data: %p len: %d\r\n", data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->write) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    WASME_SPI_DEBUG_PRINTF("SPI write port: %d, %d bytes (%p)\r\n", handle, *len, data);

    int32_t res = spi_drv->write(spi_drv_ctx, handle, data, *len);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_spi_transfer)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, read_ptr)
    m3ApiGetArg      (uint32_t, write_ptr);

    // Resolve relative buffer object pointer
    uint32_t* read_mem_p = m3ApiOffsetToPtr(read_ptr);
    // Resolve relative data pointer im buffer
    uint8_t* read_data = m3ApiOffsetToPtr(*read_mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* read_len = m3ApiOffsetToPtr(read_ptr+4);

    // Resolve relative buffer object pointer
    uint32_t* write_mem_p = m3ApiOffsetToPtr(read_ptr);
    // Resolve relative data pointer im buffer
    uint8_t* write_data = m3ApiOffsetToPtr(*write_mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* write_len = m3ApiOffsetToPtr(read_ptr+4);


    WASME_SPI_DEBUG_PRINTF("SPI transfer port: %d, read: %p write %p len: %d\r\n", handle, read_data, write_data, *read_len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->transfer) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = spi_drv->transfer(spi_drv_ctx, handle, read_data, write_data, *read_len);

    WASME_SPI_DEBUG_PRINTF("SPI transfer port: %d, %d bytes (read: %p write: %p)\r\n", handle, *read_len, read_data, write_data);

    m3ApiReturn(res);
}

m3ApiRawFunction(m3_spi_transfer_inplace)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, ptr);

    // Resolve relative buffer object pointer
    uint32_t* mem_p = m3ApiOffsetToPtr(ptr);
    // Resolve relative data pointer im buffer
    uint8_t* data = m3ApiOffsetToPtr(*mem_p);
    // Fetch length, imo this should be *mem_p + 4 but, idk
    uint32_t* len = m3ApiOffsetToPtr(ptr+4);

    WASME_SPI_DEBUG_PRINTF("SPI transfer_inplace port: %d, data: %p len: %d\r\n", handle, data, *len);

    // Check args are valid
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_FAULT); }
    if (!spi_drv) { m3ApiReturn(__WASI_ERRNO_NODEV); }
    if (!spi_drv->transfer) { m3ApiReturn(__WASI_ERRNO_NOENT); }

    int32_t res = spi_drv->transfer_inplace(spi_drv_ctx, handle, data, *len);

    WASME_SPI_DEBUG_PRINTF("SPI transfer_inplace port: %d, %d bytes (%p)\r\n", handle, *len, data);

    m3ApiReturn(res);
}

// TODO: implement exec wasm3 function


const static char* wasme_spi_mod = "spi";

int32_t WASME_bind_spi(wasme_ctx_t* ctx, const spi_drv_t* drv, void* drv_ctx) {
    M3Result m3_res;

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "init", "i(iiiiiii)", &m3_spi_init);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "deinit", "i(i)", &m3_spi_deinit);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "read", "i(ii)", &m3_spi_read);

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "write", "i(ii)", &m3_spi_write);
    
    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "transfer", "i(iii)", &m3_spi_transfer);

    m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "transfer_inplace", "i(ii)", &m3_spi_transfer_inplace);
    
    // TODO: link exec function here when implemented
    //m3_res = m3_LinkRawFunction(ctx->mod, wasme_spi_mod, "write_read", "i(iiii)", &m3_spi_write_read);
    
    spi_drv = drv;
    spi_drv_ctx = drv_ctx;
    
    return 0;
}
