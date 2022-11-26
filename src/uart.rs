
use core::ffi::c_void;
use core::slice;

use log::debug;

use wasm_embedded_spec::{uart::Uart, api::uart_drv_t};

use crate::{Driver, Wasm3Runtime};

/// Driver adaptor to C/wasm3 I2C API
impl<T: Uart> Driver<uart_drv_t> for T {
    const DRIVER: uart_drv_t = uart_drv_t {
        init: Some(uart_init::<T>),
        deinit: Some(uart_deinit::<T>),
        write: Some(uart_write::<T>),
        read: Some(uart_read::<T>),
    };

    fn bind(&mut self, rt: &mut Wasm3Runtime) -> i32 {
    unsafe { crate::WASME_bind_uart(rt.ctx, &Self::DRIVER, self.context()) }
    }
}


pub extern "C" fn uart_init<T: Uart>(
    ctx: *const c_void,
    dev: u32,
    baud: u32,
    tx: i32,
    rx: i32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Uart::init(ctx, dev, baud, tx, rx) {
        Ok(i) => i,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("Uart::init failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn uart_deinit<T: Uart>(ctx: *const c_void, handle: i32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Uart::deinit(ctx, handle) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("Uart::deinit failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn uart_read<T: Uart>(
    ctx: *const c_void,
    handle: i32,
    flags: u32,
    data_in: *mut u8,
    length_in: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let buff = unsafe { slice::from_raw_parts_mut(data_in, length_in as usize) };

    match Uart::read(ctx, handle, flags, buff) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("Uart::read failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn uart_write<T: Uart>(
    ctx: *const c_void,
    handle: i32,
    flags: u32,
    data_out: *mut u8,
    length_out: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data_out, length_out as usize) };

    match Uart::write(ctx, handle, flags, data) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("Uart::write failed: {:?}", e);
            return -1;
        }
    }
}
