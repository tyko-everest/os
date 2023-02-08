BUILD_DIR := target/aarch64-unknown-none/debug/

QEMU_FLAGS := -M raspi3b -vnc :0 \
	-kernel $(BUILD_DIR)/kernel8.img \
	-serial null -serial stdio

run: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 $(QEMU_FLAGS)

debug: $(BUILD_DIR)/kernel8.img
	qemu-system-aarch64 -s -S $(QEMU_FLAGS)

$(BUILD_DIR)/kernel8.img: $(BUILD_DIR)/os
	rust-objcopy -O binary $< $@

$(BUILD_DIR)/os:
	cargo build --target=aarch64-unknown-none

.PHONY: run debug