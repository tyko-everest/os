
const char test[] = "Hello from PRG2!\n";

int main() {
    asm(
        "mov x0, %[x]"
        :
        : [x] "r" (test)
    );
    asm("svc 30");
    
    for(;;);
}
