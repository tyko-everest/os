
const char test[] = "Hello!";
char data[100];

int main() {
    volatile int num = 10;
    num += 10;
    asm("svc 0x45");
    for(;;);
}
