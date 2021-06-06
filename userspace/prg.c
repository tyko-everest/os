
const char test[] = "Hello from userspace!";

int sqr(int a) {
    return a*a;
}

int main() {

    volatile int b = sqr(10);

    asm(
        "mov x0, %[p]"
        :
        : [p] "r" (test)
    );
    asm("svc 30");
    for(;;);
}
