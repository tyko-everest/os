
const char test[] = "Hello from userspace!\n";
int a = 12;
int c[5000];

int sqr(int a) {
    return a*a;
}

int main() {
    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (test)
    );
    asm("svc 30");
    c[4000] = sqr(a);
    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (c[4000])
    );
    for(;;);
}
