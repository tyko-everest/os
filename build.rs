fn main() {
    let files = [
        "src/start.s",
        "src/arch/delay.s",
        "src/arch/vector.s",
        "src/drivers/mmio.c",
        "src/drivers/serial.c",
    ];
    #[cfg(target_arch = "aarch64")]
    cc::Build::new()
        // this shouldn't be necessary, but gcc doesn't honour the target
        .compiler("aarch64-none-elf-gcc")
        .target("aarch64-unknown-none")
        .files(files)
        .include("src")
        .compile("os-cc");
    for file in files {
        println!("cargo:rerun-if-changed={}", file);
    }
}
