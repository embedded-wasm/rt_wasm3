
use core::ffi::c_void;

use wasm_embedded_spec::{gpio::Gpio, api::gpio_drv_t};
use embedded_hal::digital::PinState;

use crate::{Driver, Wasm3Runtime};

/// Driver adaptor to C/wasm3 I2C API
impl<T: Gpio> Driver<gpio_drv_t> for T {
    /// Typed driver object
    const DRIVER: gpio_drv_t = gpio_drv_t {
        init: Some(gpio_init::<T>),
        deinit: Some(gpio_deinit::<T>),
        set: Some(gpio_set::<T>),
        get: Some(gpio_get::<T>),
    };

    fn bind(&mut self, rt: &mut Wasm3Runtime) -> i32 {
        unsafe { crate::WASME_bind_gpio(rt.ctx, &Self::DRIVER, self.context()) }
    }
}

pub extern "C" fn gpio_init<T: Gpio>(
    ctx: *const c_void,
    port: u32,
    pin: u32,
    output: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Gpio::init(ctx, port, pin, output != 0) {
        Ok(i) => i,
        // TODO: not sure how to manage this yet
        Err(e) => {
            log::debug!("gpio_init error: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn gpio_deinit<T: Gpio>(ctx: *const c_void, handle: i32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Gpio::deinit(ctx, handle) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            log::debug!("gpio_deinit error: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn gpio_get<T: Gpio>(ctx: *const c_void, handle: i32, value: *mut u32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };

    match Gpio::get(ctx, handle) {
        Ok(PinState::High) => {
            unsafe { *value = 1 };
            0
        }
        Ok(PinState::Low) => {
            unsafe { *value = 0 };
            0
        }
        // TODO: not sure how to manage this yet
        Err(e) => {
            log::debug!("gpio_get error: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn gpio_set<T: Gpio>(ctx: *const c_void, handle: i32, value: u32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };

    let state = if value == 0 {
        PinState::Low
    } else {
        PinState::High
    };

    match Gpio::set(ctx, handle, state) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            log::debug!("gpio_set error: {:?}", e);
            return -1;
        }
    }
}

#[cfg(test)]
mod test {
    use wasm_embedded_spec::gpio::MockGpio;
    use super::*;

    #[test]
    fn test_gpio_set() {
        let mut gpio = MockGpio::new();
        let ptr = MockGpio::context(&mut gpio);

        // Set high
        gpio.expect_set()
            .withf(|h: &i32, v: &PinState| *h == 2 && *v == PinState::High )
            .return_const(Ok(()));

        let res = unsafe { MockGpio::DRIVER.set.unwrap()(ptr, 2, PinState::High as u32) };

        assert_eq!(res, 0);

        // Set low
        gpio.expect_set()
            .withf(|h: &i32, v: &PinState| *h == 2 && *v == PinState::Low )
            .return_const(Ok(()));

        let res = unsafe { MockGpio::DRIVER.set.unwrap()(ptr, 2, PinState::Low as u32) };

        assert_eq!(res, 0);
    }
}

