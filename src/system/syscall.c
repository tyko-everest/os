#include "syscall.h"

/**
 * TODO
 * 
 * - bunch of error checking
 * - to start, check pointers are within range of userspace memory
 * and that the last byte requested to be read/written is also in userspace
 */

ssize_t sys_read(const char *path, void *buf, size_t count, size_t from) {
    if ((uintptr_t) buf + count >= VM_USERSPACE_SIZE || (uintptr_t) path > VM_USERSPACE_SIZE) {
        return -1;
    }
    return readfile(path, from, count, buf);
}

ssize_t sys_write(const char *path, void *buf, size_t count, size_t from) {
    if ((uintptr_t) buf + count >= VM_USERSPACE_SIZE || (uintptr_t) path > VM_USERSPACE_SIZE) {
        return -1;
    }
    return writefile(path, from, count, buf);
}

ssize_t sys_make(const char *path, bool is_dir) {
    if ((uintptr_t) path > VM_USERSPACE_SIZE) {
        return -1;
    }
    return makefile(path, is_dir);
}

ssize_t sys_delete(const char *path) {
    if ((uintptr_t) path > VM_USERSPACE_SIZE) {
        return -1;
    }
    return deletefile(path);
}

void sys_exec(const char *path) {
    // need to make a copy of the path because the old program's mapping gets nuked here
    // so the path which existed in that mapping disappears
    size_t path_arr_len = strlen(path) + 1;
    char *safe_path = kmalloc(path_arr_len);
    memcpy(safe_path, path, path_arr_len);

    process_t proc;
    int pid = proc_new(safe_path);
    proc_switch(pid);

    kfree(safe_path);
}

void sys_start(const char *path, general_regs_t *gp_regs) {
    system_regs_t sys_regs;
    READ_SYS_REG(sp_el0, sys_regs.sp_el0);
    READ_SYS_REG(elr_el1, sys_regs.elr_el1);
    READ_SYS_REG(spsr_el1, sys_regs.spsr_el1);
    READ_SYS_REG(ttbr0_el1, sys_regs.ttbr0_el1);
    save_regs(gp_regs, &sys_regs);

    // need to make a copy of the path because the old program's mapping gets nuked here
    // so the path which existed in that mapping disappears
    size_t path_arr_len = strlen(path) + 1;
    char *safe_path = kmalloc(path_arr_len);
    memcpy(safe_path, path, path_arr_len);

    int pid = proc_new(safe_path);
    proc_switch(pid);

    kfree(safe_path);

}

void sys_exit(general_regs_t *regs) {
    size_t parent = get_parent(get_curr_pid());

    // cleanup current proc struct
    
    get_gp_regs(parent, regs);
    proc_load(parent);
    proc_switch(parent);
}

void sys_print(const char *str) {
    printf("%s", str);
}
