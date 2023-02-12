

use core::ffi::c_void;
use core::slice;

use log::{warn};

use wasm_embedded_spec::{Spi, bindgen::spi_drv_t};

use crate::{Driver, Wasm3Runtime};

/// Driver adaptor to C/wasm3 SPI API
impl<T: Spi> Driver<spi_drv_t> for T {
    const DRIVER: spi_drv_t= spi_drv_t {
        init: Some(spi_init::<T>),
        deinit: Some(spi_deinit::<T>),
        read: Some(spi_read::<T>),
        write: Some(spi_write::<T>),
        transfer: Some(spi_transfer::<T>),
        transfer_inplace: Some(spi_transfer_inplace::<T>),
        exec: None,
    };

    fn bind(&mut self, rt: &mut Wasm3Runtime) -> i32 {
        unsafe { crate::WASME_bind_spi(rt.ctx, &Self::DRIVER, self.context()) }
    }
}


// C handlers for WASM3 SPI API

pub extern "C" fn spi_init<T: Spi>(
    ctx: *const c_void,
    dev: u32,
    baud: u32,
    mosi: i32,
    miso: i32,
    sck: i32,
    cs: i32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Spi::init(ctx, dev, baud, mosi, miso, sck, cs) {
        Ok(i) => i,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("spi_init failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn spi_deinit<T: Spi>(ctx: *const c_void, handle: i32) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    match Spi::deinit(ctx, handle) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("spi_deinit failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn spi_read<T: Spi>(
    ctx: *const c_void,
    handle: i32,
    data_in: *mut u8,
    length_in: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data_in, length_in as usize) };

    match Spi::read(ctx, handle, data) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("read failed: {:?}", e);
            return -1;
        }
    }
}


pub extern "C" fn spi_write<T: Spi>(
    ctx: *const c_void,
    handle: i32,
    data_out: *mut u8,
    length_out: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data_out, length_out as usize) };

    match Spi::write(ctx, handle, data) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("spi_write failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn spi_transfer<T: Spi>(
    ctx: *const c_void,
    handle: i32,
    read: *mut u8,
    write: *mut u8,
    length: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let read = unsafe { slice::from_raw_parts_mut(read, length as usize) };
    let write = unsafe { slice::from_raw_parts_mut(write, length as usize) };

    match Spi::transfer(ctx, handle, read, write) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("spi_transfer failed: {:?}", e);
            return -1;
        }
    }
}

pub extern "C" fn spi_transfer_inplace<T: Spi>(
    ctx: *const c_void,
    handle: i32,
    data: *mut u8,
    length: u32,
) -> i32 {
    let ctx: &mut T = unsafe { &mut *(ctx as *mut T) };
    let data = unsafe { slice::from_raw_parts_mut(data, length as usize) };

    match Spi::transfer_inplace(ctx, handle, data) {
        Ok(_) => 0,
        // TODO: not sure how to manage this yet
        Err(e) => {
            warn!("spi_transfer failed: {:?}", e);
            return -1;
        }
    }
}
