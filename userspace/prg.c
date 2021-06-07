
const char test[] = "Hello from userspace!\n";
int a = 10;
int c[5];

int sqr(int a) {
    return a*a;
}

int main() {

    c[1] = sqr(a);

    asm(
        "mov x0, %[p]"
        :
        : [p] "r" (test)
    );
    asm("svc 30");
    for(;;);
}
