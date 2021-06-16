
const char test[] = "Hello from userspace!\n";
const char path[] = "/PRG2";

int main() {
    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (test)
    );
    asm("svc 30");

    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (path)
    );
    asm("svc 21");

    for(;;);
}
