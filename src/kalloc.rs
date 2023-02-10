use core::alloc::{GlobalAlloc, Layout};
use core::cell::UnsafeCell;
use core::option::Option;
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
    fn set_range(&mut self, start: usize, end: usize, val: bool) {
        for index in start..=end {
            self.set(index, val);
        }
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
        let bit = self.bit_array.get(self.index);
        self.index += 1;
        bit
    }
}

fn round_up(num: usize, interval: usize) -> usize {
    (num + interval - 1) & !(interval - 1)
}

fn round_to_bucket(num: usize) -> usize {
    if num <= 8 {
        8
    } else if num <= 64 {
        64
    } else if num <= 512 {
        512
    } else {
        round_up(num, 4096)
    }
}

#[repr(C, align(4096))]
struct KernelAllocator {
    data: UnsafeCell<[u8; HEAP_SIZE]>,
    free8: UnsafeCell<bit_array_type!(HEAP_SIZE / 8)>,
    free64: UnsafeCell<bit_array_type!(HEAP_SIZE / 64)>,
    free512: UnsafeCell<bit_array_type!(HEAP_SIZE / 512)>,
    free4096: UnsafeCell<bit_array_type!(HEAP_SIZE / 4096)>,
}

unsafe impl Sync for KernelAllocator {}

// keep track of free chunks of memory at 8, 64, 512, and 4096 bytes of granularuty using bitmaps
// always round up to the nearest size on alloc/dealloc
unsafe impl GlobalAlloc for KernelAllocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let align = layout.align();
        if align > MAX_ALIGN {
            return null_mut();
        }
        let align = round_to_bucket(align);
        let size = round_up(layout.size(), align);

        if align == 8 {
            let mut free_count = 0;
            for (index, bit) in (*self.free8.get()).into_iter().enumerate() {
                if bit == false {
                    free_count += 1;
                } else {
                    free_count = 0;
                }
                if free_count >= size / 8 {
                    let start_chunk = index + 1 - free_count;
                    let end_chunk = index;
                    (*self.free8.get()).set_range(start_chunk, end_chunk, true);
                    return self.data.get().cast::<u8>().add(start_chunk * 8);
                }
            }
        } else if align == 64 {
            let mut free_count = 0;
            for (index, bit) in (*self.free64.get()).into_iter().enumerate() {
                if bit == false {
                    free_count += 1;
                } else {
                    free_count = 0;
                }
                if free_count >= size / 64 {
                    let start_chunk = index + 1 - free_count;
                    let end_chunk = index;
                    (*self.free64.get()).set_range(start_chunk, end_chunk, true);
                    return self.data.get().cast::<u8>().add(start_chunk * 64);
                }
            }
        } else if align == 512 {
            let mut free_count = 0;
            for (index, bit) in (*self.free512.get()).into_iter().enumerate() {
                if bit == false {
                    free_count += 1;
                } else {
                    free_count = 0;
                }
                if free_count >= size / 512 {
                    let start_chunk = index + 1 - free_count;
                    let end_chunk = index;
                    (*self.free512.get()).set_range(start_chunk, end_chunk, true);
                    return self.data.get().cast::<u8>().add(start_chunk * 512);
                }
            }
        } else if align == 4096 {
            let mut free_count = 0;
            for (index, bit) in (*self.free4096.get()).into_iter().enumerate() {
                if bit == false {
                    free_count += 1;
                } else {
                    free_count = 0;
                }
                if free_count >= size / 4096 {
                    let start_chunk = index + 1 - free_count;
                    let end_chunk = index;
                    (*self.free4096.get()).set_range(start_chunk, end_chunk, true);
                    return self.data.get().cast::<u8>().add(start_chunk * 4096);
                }
            }
        } else {
            return null_mut();
        };
        null_mut()
    }
    unsafe fn dealloc(&self, ptr: *mut u8, layout: Layout) {
        let size = round_to_bucket(layout.size());
        let align = round_to_bucket(layout.align());
        // todo!()
    }
}

#[cfg(not(test))]
#[global_allocator]
static ALLOCATOR: KernelAllocator = KernelAllocator {
    data: UnsafeCell::new([0; HEAP_SIZE]),
    free8: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 8)),
    free64: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 64)),
    free512: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 512)),
    free4096: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 4096)),
};

#[cfg(test)]
mod tests {
    use alloc::vec;

    use super::*;

    #[test]
    fn bit_array() {
        let mut bit_array = bit_array_new!(256);
        assert_eq!(false, bit_array.get(100).unwrap());
        bit_array.set(100, true);
        assert_eq!(true, bit_array.get(100).unwrap());

        let bit_array = bit_array_new!(65);
        assert_eq!(false, bit_array.get(65).unwrap());
    }

    #[test]
    fn alloc() {
        let allocator: KernelAllocator = KernelAllocator {
            data: UnsafeCell::new([0; HEAP_SIZE]),
            free8: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 8)),
            free64: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 64)),
            free512: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 512)),
            free4096: UnsafeCell::new(bit_array_new!(HEAP_SIZE / 4096)),
        };

        let layout = Layout::from_size_align(64, 8).unwrap();
        unsafe {
            let ptr = allocator.alloc(layout);
            *ptr = 8;
        }

        let mut vec = vec![1, 2, 3];

        let i = 0;
    }
}
