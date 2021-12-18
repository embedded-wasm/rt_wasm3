//! WASME type header for internal use only
#ifndef WASME_INT_H
#define WASME_INT_H

#include "wasm3.h"

struct wasme_ctx_s {
    IM3Environment env;
    IM3Runtime rt;
    IM3Module mod;
};

#endif
