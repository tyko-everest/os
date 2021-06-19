#include "sched.h"



void sched_start() {
    size_t pid = proc_new("/PRG");
    proc_switch(pid);
    asm("eret");
}

void yield() {
    
}
