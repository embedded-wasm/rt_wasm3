
#include "wasm_embedded/wasm3/core.h"
#include "wasm_embedded/wasm3/internal.h"

#include <stdio.h>

#include "wasm3.h"
#include "m3_api_wasi.h"

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

    // Setup runtime
    ctx->rt = m3_NewRuntime(ctx->env, mem_limit, NULL);
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
