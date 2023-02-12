//! Rust library wrapper for wasme/wasm3 API.
//! 
//! This may be prefered to rt_wasmtime for embedded device use

#![cfg_attr(not(feature="std"), no_std)]
#![allow(non_snake_case, non_camel_case_types, non_upper_case_globals, clippy::all)]

use core::ptr;

use log::{debug};
pub use cty::{c_char};

// Driver types defined in wasm_embedded_spec
pub use wasm_embedded_spec::{self as spec};
pub use spec::{
    Engine,
    Error,
    bindgen::{gpio_drv_t, spi_drv_t, i2c_drv_t, uart_drv_t},
};

// Driver modules
mod gpio;
mod spi;
mod i2c;
mod uart;


// Rust bindings for wasm3 C library
include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

// Start symbol name in wasm binary
const START_STR: &'static [u8] = b"_start\0";

/// WASM3 runtime errors
#[derive(Debug, Clone, PartialEq)]
#[cfg_attr(feature="thiserror", derive(thiserror::Error))]
#[cfg_attr(feature="defmt", derive(defmt::Format))]
pub enum Wasm3Err {
    #[cfg_attr(feature="thiserror", error("Failed to create context"))]
    Ctx,
    #[cfg_attr(feature="thiserror", error("I2C init error: {0}"))]
    I2c(i32),
    #[cfg_attr(feature="thiserror", error("SPI init error: {0}"))]
    Spi(i32),
    #[cfg_attr(feature="thiserror", error("GPIO init error: {0}"))]
    Gpio(i32),
    #[cfg_attr(feature="thiserror", error("Execution error: {0}"))]
    Exec(i32),
    #[cfg_attr(feature="thiserror", error("Driver binding error: {0}"))]
    Bind(i32),
}

/// WASM3 runtime instance
pub struct Wasm3Runtime {
    _task: wasme_task_t,
    ctx: *mut wasme_ctx_t,
}

impl Wasm3Runtime {
    /// Create new WASM3 runtime instance with the provided app
    pub fn new<E: Engine>(engine: &mut E, data: &[u8]) -> Result<Self, Wasm3Err> {
        // Setup WASME task
        let task = wasme_task_t{
            data: data.as_ptr(),
            data_len: data.len() as u32,
        };
    
        // Initialise WASME context
        let ctx = unsafe { WASME_init(&task, 10 * 1024) };
        if ctx.is_null() {
            return Err(Wasm3Err::Ctx);
        }

        let mut rt = Self{
            _task: task,
            ctx,
        };

        // Bind drivers
        if let Some(gpio) = engine.gpio() {
            rt.bind::<gpio_drv_t, _>(gpio)?;
        }
        if let Some(spi) = engine.spi() {
            rt.bind::<spi_drv_t, _>(spi)?;
        }
        if let Some(i2c) = engine.i2c() {
            rt.bind::<i2c_drv_t, _>(i2c)?;
        }
        if let Some(uart) = engine.uart() {
            rt.bind::<uart_drv_t, _>(uart)?;
        }

        Ok(rt)
    }

    pub fn bind<I, D: Driver<I>>(&mut self, driver: &mut D) -> Result<(), Wasm3Err> {
        debug!("Binding {} driver: {}", 
            core::any::type_name::<I>(),
            core::any::type_name::<D>(),
        );

        let res = driver.bind(self);
        if res < 0 {
            Err(Wasm3Err::Bind(res))
        } else {
            Ok(())
        }
    }

    /// Run task in WASM3 runtime
    pub fn run(&mut self) -> Result<(), Wasm3Err> {
        let entry = START_STR.as_ptr() as *const c_char;

        let res = unsafe { WASME_run(self.ctx, entry, 0, ptr::null_mut()) };
        if res < 0 {
            return Err(Wasm3Err::Exec(res));
        }

        debug!("WASME execution complete!");

        Ok(())
    }
}

impl Drop for Wasm3Runtime {
    /// Cleanup wasm3 runtime
    fn drop(&mut self) {
        unsafe { WASME_deinit(&mut self.ctx) }
        self.ctx = core::ptr::null_mut();
    }
}

/// Driver trait implemented for each API
pub trait Driver<Inner>: Sized {
    /// Typed driver object for binding
    const DRIVER: Inner;

    /// Driver context pointer for binding
    // TODO: this is definitely technically unsafe...
    // TODO: should this be pinned somehow?
    fn context(&mut self) -> *mut cty::c_void {
        self as *mut _ as *mut cty::c_void
    }

    /// Bind to runtime using generated WASME_bind_x call
    fn bind(&mut self, rt: &mut Wasm3Runtime) -> i32;
}
