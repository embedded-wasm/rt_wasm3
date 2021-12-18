//! Rust library wrapper for wasme/wasm3 API.
//! Provided for testing via embedded-wasm-rt, not intended for consumer use

#![no_std]
#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals, clippy::all)]

pub use cty::{c_char};

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));
