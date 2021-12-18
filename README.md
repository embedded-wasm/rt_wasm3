# Embedded WASM WASM3 Runtime

A WASM3 runtime based on the embedded-wasm [spec](https://github.com/embedded-wasm/spec). This implements the WITX API for drivers matching the C driver interface included in the spec.

## Status

WIP. Extremely alpha, expect changes and breakages while we develop the ecosystem.

[![ci](https://github.com/embedded-wasm/rt_wasm3/actions/workflows/ci.yml/badge.svg)](https://github.com/embedded-wasm/rt_wasm3/actions/workflows/ci.yml)
[![Crates.io](https://img.shields.io/crates/v/wasm-embedded-rt-wasm3.svg)](https://crates.io/crates/wasm-embedded-rt-wasm3)
[![Docs.rs](https://docs.rs/wasm-embedded-rt-wasm3/badge.svg)](https://docs.rs/wasm-embedded-rt-wasm3)


## Usage

The runtime library is primarily built using [cmake](), though it is relatively straightforward to port this to other mechanisms.

- `mkdir build && cd build` to create a build directory
- `cmake ..` to setup the build
  - Add `-DWASME_SPEC_DIR=something` to use a local source for the spec headers
  - Use `-DWASME_BUILD_WASM3=off` to disable building wasm3 (you will need to provide wasm3 headers via `-DWASME_WASM3_DIR=something`)
- `make -j` to build the library

A [cargo]() based build for rust is also provided to simplify integration with rust components.
