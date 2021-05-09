#include <stdio.h>

#include "../../src/clib/string.h"

int main() {
    char test[] = "this is a test";
    char keys[] = "16ai90";

    char *res = memchr(test, 's', 10);
    printf("%d\n", res - test);

    return 0;
}