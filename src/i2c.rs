
use core::ffi::c_void;
use core::slice;

use log::debug;

use wasm_embedded_spec::{i2c::I2c, api::i2c_drv_t};

use crate::{Driver, Wasm3Runtime};

/// Driver adaptor to C/wasm3 I2C API
impl<T: I2c> Driver<i2c_drv_t> for T {
    const DRIVER: i2c_drv_t = i2c_drv_t {
        init: Some(i2c_init::<T>),
        deinit: Some(i2c_deinit::<T>),
        write: Some(i2c_write::<T>),
        read: Some(i2c_read::<T>),
        write_read: Some(i2c_write_read::<T>),
    };

    fn bind(&mut self, rt: &mut Wasm3Runtime) -> i32 {
    unsafe { crate::WASME_bind_i2c(rt.ctx, &Self::DRIVER, self.context()) }
    }
}


pub extern "C" fn i2c_init<T: I2c>(
    ctx: *const c_void,
    dev: u32,
    baud: u32,
    sda: i32,
    scl: i32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match I2c::init(ctx, dev, baud, sda, scl) {
        Ok(i) => i,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("I2c::init failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn i2c_deinit<T: I2c>(ctx: *const c_void, handle: i32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match I2c::deinit(ctx, handle) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("I2c::deinit failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn i2c_read<T: I2c>(
    ctx: *const c_void,
    handle: i32,
    address: u16,
    data_in: *mut u8,
    length_in: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let buff = unsafe { slice::from_raw_parts_mut(data_in, length_in as usize) };

    match I2c::read(ctx, handle, address, buff) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("I2c::read failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn i2c_write<T: I2c>(
    ctx: *const c_void,
    handle: i32,
    address: u16,
    data_out: *mut u8,
    length_out: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data_out, length_out as usize) };

    match I2c::write(ctx, handle, address, data) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("I2c::write failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn i2c_write_read<T: I2c>(
    ctx: *const c_void,
    handle: i32,
    address: u16,
    data_out: *mut u8,
    length_out: u32,
    data_in: *mut u8,
    length_in: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data_out, length_out as usize) };
    let buff = unsafe { slice::from_raw_parts_mut(data_in, length_in as usize) };

    match I2c::write_read(ctx, handle, address, data, buff) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            debug!("I2c::write_read failed: {:?}", e);
            return -1;
        }
    }
}
