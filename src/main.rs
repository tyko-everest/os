#![no_std]
#![cfg_attr(not(test), no_main)]
#![feature(alloc_error_handler, panic_info_message)]
extern crate alloc;
use alloc::string::String;
use core::ffi::c_void;
use core::fmt::Write;
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

macro_rules! print {
    ($s:expr) => {
        for c in $s.bytes() {
            putc(c);
        }
    };
    ($($args:tt)*) => {
        let mut s = String::new();
        write!(s, $($args)*).unwrap();
        for c in s.bytes() {
            putc(c);
        }
    };
}

#[no_mangle]
pub extern "C" fn interrupt_handler() {
    print_str("Interrupt!\n");
}

#[cfg(not(test))]
#[panic_handler]
fn panic(panic: &PanicInfo<'_>) -> ! {
    print!("PANICKING!\n");
    print!("File: {}\n", panic.location().unwrap().file());
    print!("Reason: {}\n", *panic.message().unwrap());
    loop {}
}

#[cfg(not(test))]
#[no_mangle]
pub extern "C" fn main() {
    use alloc::vec;

    print!("Hello World!\n");

    let test = vec![1, 2, 3];
    print!("{:?}\n", test);

    panic!("BOO");

    print!("Done\n");
}
