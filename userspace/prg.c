
const char test[] = "Hello from userspace!\n";
const char back[] = "Back to PRG!\n";
const char path[] = "/PRG2";

int main() {
    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (test)
        : "x0"
    );
    asm("svc 30");

    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (path)
        : "x0"
    );
    asm("svc 22");

    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (back)
        : "x0"
    );
    asm("svc 30");

    for(;;);
}
