#ifndef INCLUDE_SCHED
#define INCLUDE_SCHED

#include "clib/stddef.h"
#include "clib/stdint.h"
#include "system/proc.h"

void sched_start();
void yield();

#endif
