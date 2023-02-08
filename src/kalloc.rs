use core::alloc::{GlobalAlloc, Layout};
use core::cell::UnsafeCell;
use core::ptr::null_mut;
use intbits::Bits;

const PG_SIZE: usize = 4096;
const HEAP_SIZE: usize = 32 * PG_SIZE;
const MAX_ALIGN: usize = PG_SIZE;

// note N is number of u64s, not bits
// could use generic_const_exprs to fix this but don't want to use nightly for now
// using the macros below for now to get around this
struct BitArray<const N: usize> {
    data: [u64; N],
}

impl<const N: usize> BitArray<N> {
    fn get(&self, index: usize) -> Option<bool> {
        if let Some(num) = self.data.get(index >> 6) {
            Some(num.bit(index & (64 - 1)))
        } else {
            None
        }
    }
    fn set(&mut self, index: usize, val: bool) {
        self.data[index / 64].set_bit(index % 64, val);
    }
}

macro_rules! bit_array_type {
    ($num_bits:expr) => {
        BitArray<{($num_bits + 63) / 64}>
    };
}

macro_rules! bit_array_new {
    ($num_bits:expr) => {
        BitArray::<{ ($num_bits + 63) / 64 }> {
            data: [0; ($num_bits + 63) / 64],
        }
    };
}

impl<'a, const N: usize> IntoIterator for &'a BitArray<N> {
    type Item = bool;
    type IntoIter = BitArrayIter<'a, N>;

    fn into_iter(self) -> Self::IntoIter {
        BitArrayIter::new(&self)
    }
}

struct BitArrayIter<'a, const N: usize> {
    bit_array: &'a BitArray<N>,
    index: usize,
}

impl<'a, const N: usize> BitArrayIter<'a, N> {
    fn new(bit_array: &'a BitArray<N>) -> Self {
        Self {
            bit_array,
            index: 0,
        }
    }
}

impl<'a, const N: usize> Iterator for BitArrayIter<'a, N> {
    type Item = bool;
    fn next(&mut self) -> Option<Self::Item> {
        self.bit_array.get(self.index)
    }
}

fn round_num_up(num: usize, interval: usize) -> usize {
    return (num + interval - 1) & !(interval - 1);
}

fn round_to_bucket(size: usize) -> usize {
    if size <= 8 {
        round_num_up(size, 8)
    } else if size <= 64 {
        round_num_up(size, 64)
    } else if size <= 512 {
        round_num_up(size, 512)
    } else {
        round_num_up(size, 4096)
    }
}

#[repr(C, align(4096))]
struct KernelAllocator {
    data: UnsafeCell<[u8; HEAP_SIZE]>,
    free8: bit_array_type!(HEAP_SIZE / 8),
    free64: bit_array_type!(HEAP_SIZE / 64),
    free512: bit_array_type!(HEAP_SIZE / 512),
    free4096: bit_array_type!(HEAP_SIZE / 4096),
}

unsafe impl Sync for KernelAllocator {}

// keep track of free chunks of memory at 8, 64, 512, and 4096 bytes of granularuty using bitmaps
// always round up to the nearest size on alloc/dealloc
unsafe impl GlobalAlloc for KernelAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let size = round_to_bucket(layout.size());
        let align = round_to_bucket(layout.align());

        if size <= 8 {}

        todo!()
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        let size = round_to_bucket(layout.size());
        let align = round_to_bucket(layout.align());
        todo!()
    }
}

#[cfg(not(test))]
#[global_allocator]
static ALLOCATOR: KernelAllocator = KernelAllocator {
    data: UnsafeCell::new([0; HEAP_SIZE]),
    free8: bit_array_new!(HEAP_SIZE / 8),
    free64: bit_array_new!(HEAP_SIZE / 64),
    free512: bit_array_new!(HEAP_SIZE / 512),
    free4096: bit_array_new!(HEAP_SIZE / 4096),
};

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn bit_array() {
        let mut bit_array = bit_array_new!(256);
        assert_eq!(false, bit_array.get(100).unwrap());
        bit_array.set(100, true);
        assert_eq!(true, bit_array.get(100).unwrap());
    }
}
