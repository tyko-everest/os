#ifndef INCLUDE_ARM_H
#define INCLUDE_ARM_H

#define WRITE_SYS_REG(reg, val) asm("msr " #reg ", %[v]" : : [v] "r" (val));
#define READ_SYS_REG(reg, val) asm ("mrs %[v], " #reg : [v] "=r" (val)); 

#endif