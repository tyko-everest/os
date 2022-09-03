#![no_std]

use core::ffi::c_void;
use core::panic::PanicInfo;

extern "C" {
    pub fn serial_putc(p: *mut c_void, c: u8);
}

fn putc(c: u8) {
    let p = 0 as *mut c_void;
    unsafe {
        serial_putc(p, c);
    }
}

#[panic_handler]
fn panic(_panic: &PanicInfo<'_>) -> ! {
    loop {}
}
#[no_mangle]
pub extern "C" fn main() {
    for c in b"Hello, world!\n" {
        putc(*c);
    }
}
