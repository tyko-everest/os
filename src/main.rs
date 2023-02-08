#![no_std]
#![cfg_attr(not(test), no_main)]
use core::ffi::c_void;
use core::panic::PanicInfo;

mod kalloc;

#[cfg(not(test))]
extern "C" {
    pub fn serial_putc(p: *mut c_void, c: u8);
}

#[cfg(test)]
fn serial_putc(_p: *mut c_void, _c: u8) {}

fn putc(c: u8) {
    let p = 0 as *mut c_void;
    unsafe {
        serial_putc(p, c);
    }
}

#[no_mangle]
pub extern "C" fn interrupt_handler() {
    for c in b"Interrupt!\n" {
        putc(*c);
    }
}

#[cfg(not(test))]
#[panic_handler]
fn panic(_panic: &PanicInfo<'_>) -> ! {
    loop {}
}

#[cfg(not(test))]
#[no_mangle]
pub extern "C" fn main() {
    for c in b"Hello, world!\n" {
        putc(*c);
    }
}
