use core::sync::atomic::{AtomicBool, Ordering};

struct Spinlock {
    lock: AtomicBool,
}

impl Spinlock {
    fn lock(&mut self) {
        while self
            .lock
            .compare_exchange(false, true, Ordering::Acquire, Ordering::Relaxed)
        {}
    }
}
