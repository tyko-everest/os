#include <stdio.h>

#include "../../src/system/pm.h"

int main() {
    print_free_memory();
    uintptr_t p1 = pm_get_page();
    print_free_memory();
    uintptr_t p2 = pm_get_page();
    print_free_memory();
    pm_free_page(p1);
    print_free_memory();
}
