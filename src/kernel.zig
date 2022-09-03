const c = @cImport({
    @cInclude("drivers/serial.h");
});

export fn main() void {
    for ("Hello from Zig!\n") |char| {
        c.serial_putc(null, char);
    }
}
