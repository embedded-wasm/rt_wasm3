
#include "wasm_embedded/wasm3/core.h"
#include "wasm_embedded/wasm3/internal.h"

#include <stdio.h>

#include "wasm3.h"
#include "m3_api_wasi.h"
#include "m3_env.h"
#include "m3_exception.h"
#include "m3_info.h"
#include "extra/wasi_core.h"

wasme_ctx_t* WASME_init(const wasme_task_t* task, uint32_t mem_limit) {
    M3Result m3_res;
    int32_t res = 0;

    wasme_ctx_t* ctx = malloc(sizeof(wasme_ctx_t));
    if(!ctx) {
        printf("Allocating wasme_ctx_t failed\r\n");
        res = -1;

        return NULL;
    }

    // Setup environment
    ctx->env = m3_NewEnvironment ();
    if (!ctx->env) {
        printf("NewEnvironment failed\r\n");
        res = -2;

        goto teardown_env;
    }

    // Setup runtime, binding wasme context as userdata
    ctx->rt = m3_NewRuntime(ctx->env, mem_limit, ctx);
    if (!ctx->rt) {
        printf("NewRuntime failed\r\n");
        res = -3;

        goto teardown_rt;
    }

    printf( "Loading WebAssembly (p: %p, %d bytes)...\r\n", (void*)task->data, task->data_len);

    // Parse module into environment
    m3_res = m3_ParseModule (ctx->env, &ctx->mod, task->data, task->data_len);
    if (m3_res) {
        printf("ParseModule failed: %s\r\n", m3_res);
        res = -4;

        // Only unloaded modules should be manually freed
        m3_FreeModule(ctx->mod);

        goto teardown_rt;
    }

    // Load module into runtime
    m3_res = m3_LoadModule(ctx->rt, ctx->mod);
    if (m3_res) {
        printf("LoadModule failed: %s\r\n", m3_res);
        res = -5;

        goto teardown_rt;
    }

    // Link WASI functions
    m3_res = m3_LinkWASI(ctx->mod);
    if (m3_res) {
        printf("LinkWasi failed: %s\r\n", m3_res);
        res = -6;

        goto teardown_rt;
    }

    return ctx;

teardown_rt:
    m3_FreeRuntime(ctx->rt);

teardown_env:
    m3_FreeEnvironment(ctx->env);

    free(ctx);
    
    return NULL;
}

void WASME_deinit(wasme_ctx_t** ctx) {
    if(!*ctx) {
        return;
    }

    // TODO: de-init module too

    if((*ctx)->rt) {
        m3_FreeRuntime((*ctx)->rt);
    }

    if((*ctx)->env) {
        m3_FreeEnvironment((*ctx)->env);
    }

    free(*ctx);

    *ctx = NULL;
}

int WASME_run(wasme_ctx_t* ctx, const char* name, int32_t argc, const char** argv) {

    // Locate function to call
    IM3Function f;
    M3Result m3_res = m3_FindFunction (&f, ctx->rt, name);
    if (m3_res) {
        printf("FindFunction failed: %s\r\n", m3_res);
        return -1;
    }

#ifdef WASIENV
    // Bind argc/argv via WASI
    m3_wasi_context_t* wasi_ctx = m3_GetWasiContext();
    wasi_ctx->argc = argc;
    wasi_ctx->argv = argv;
    // TODO: actually use this?
#endif

    // Call function
    m3_res = m3_Call(f, 0, NULL);
    if (m3_res) {
        printf("CallWithArgs failed: %s\r\n", m3_res);

        M3ErrorInfo error_info = { 0 };
        m3_GetErrorInfo(ctx->rt, &error_info);

        printf("message: %s\r\n", error_info.message);

        if (error_info.module) {
            printf("module: %s\r\n", m3_GetModuleName(error_info.module));
        }

        if (error_info.function) {
            printf("function: %s\r\n", m3_GetFunctionName(error_info.function));
        }

        m3_PrintM3Info();
        m3_PrintRuntimeInfo(ctx->rt);

        return -2;
    }


    // TODO: fetch result (int)

    return 0;
}

/// @brief bind a driver to the provided wasme context
int WASME_bind(wasme_ctx_t* ctx, uint32_t cla, core_driver_handle_f drv, void* drv_ctx) {
    // Check the context object exists
    if (!ctx) {
        return -1;
    }
    // Check we have driver slots available
    if (ctx->num_drivers >= CORE_MAX_DRIVERS) {
        return -2;
    }

    // Bind the driver to the next available slot
    int index = ctx->num_drivers;
    ctx->drivers[index].class = cla;
    ctx->drivers[index].handle = drv;
    ctx->drivers[index].ctx = drv_ctx;

    // Increment the driver count
    ctx->num_drivers += 1;

    return 0;
}

//! Execute the provided syscall via matching driver
m3ApiRawFunction(m3_core_exec)
{
    // Load arguments
    m3ApiReturnType  (int32_t)
    m3ApiGetArg      (uint32_t, cla)
    m3ApiGetArg      (uint32_t, ins)
    m3ApiGetArg      (uint32_t, flags)
    m3ApiGetArg      (int32_t, handle)
    m3ApiGetArg      (uint32_t, cmd_ptr)
    m3ApiGetArg      (uint32_t, resp_ptr);

    // Resolve command buffer memory offsets
    uint32_t* cmd_ptr_m3 = m3ApiOffsetToPtr(cmd_ptr);
    uint8_t* cmd_data = m3ApiOffsetToPtr(*cmd_ptr_m3);
    uint32_t* cmd_len = m3ApiOffsetToPtr(cmd_ptr+4);

    // Resolve response buffer memory offsets
    uint32_t* resp_ptr_m3 = m3ApiOffsetToPtr(resp_ptr);
    uint8_t* resp_data = m3ApiOffsetToPtr(*resp_ptr_m3);
    uint32_t* resp_len = m3ApiOffsetToPtr(resp_ptr+4);

    WASME_I2C_DEBUG_PRINTF("CORE exec CLA: %d INS: %d FLAGS: %p DEVICE: %d\r\n", cla, ins, flags, handle);

    // Check runtime and fetch context from userdata
    if (!runtime) { m3ApiReturn(__WASI_ERRNO_NOSYS); }
    if (!runtime->userdata) { m3ApiReturn(__WASI_ERRNO_NOSYS); }
    wasme_ctx_t* ctx = (wasme_ctx_t*) m3_GetUserData(runtime);

    // Locate driver instance by class
    wasme_driver_t* drv = NULL;
    for (int i=0; i<ctx->num_drivers; i++) {
        if (ctx->drivers[i].class == cla) {
            drv = &ctx->drivers[i];
            break;
        }
    }
    if (!drv) { 
        WASME_I2C_DEBUG_PRINTF("CORE no driver found for CLA: %d\r\n", cla);
        m3ApiReturn(__WASI_ERRNO_NOLINK);
    }

    // Execute driver operation
    int32_t res = drv->handle(ins, flags, handle, cmd_data, *cmd_len, resp_data, *resp_len);

    // Return driver result
    m3ApiReturn(res);
}
