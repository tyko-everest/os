#![no_std]
#![cfg_attr(not(test), no_main)]
#![feature(alloc_error_handler, panic_info_message)]
extern crate alloc;
use alloc::string::String;
use core::ffi::c_void;
use core::fmt;
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

fn print_str(string: &str) {
    for c in string.bytes() {
        putc(c);
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
fn panic(panic: &PanicInfo<'_>) -> ! {
    print_str("PANICKING!\n");
    print_str(panic.location().unwrap().file());
    print_str("\n");
    let mut msg = String::new();
    fmt::write(&mut msg, *panic.message().unwrap()).unwrap();
    print_str(&msg);
    loop {}
}

#[cfg(not(test))]
#[no_mangle]
pub extern "C" fn main() {
    use alloc::vec;

    print_str("Hello World!\n");
    let test = vec![1, 2, 3];
    // let mut output = String::with_capacity(256);
    let mut output = String::new();
    fmt::write(&mut output, format_args!("{:?}\n", test)).unwrap();
    print_str(&output);
    print_str("Done\n");
}
