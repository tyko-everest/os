fn main() {
    let files = [
        "src/start.s",
        "src/arch/delay.s",
        "src/arch/vector.s",
        "src/drivers/mmio.c",
        "src/drivers/serial.c",
    ];
    cc::Build::new()
        // this shouldn't be necessary, but gcc doesn't honour the target
        .compiler("aarch64-none-elf-gcc")
        .target("aarch64-unknown-none")
        .files(files)
        .include("src")
        // .no_default_flags(true)
        // .flag("-c")
        // .flag("-mstrict-align")
        // .flag("-fPIC")
        // .flag("-ffreestanding")
        // .static_flag(true)
        // .shared_flag(true)
        .compile("os-cc");
    for file in files {
        println!("cargo:rerun-if-changed={}", file);
    }
}
