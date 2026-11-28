# needle firmware

WCH CH572D

[ch32fun](https://github.com/cnlohr/ch32fun)


## Setup

### Toolchain

for macOS:

```bash
brew install riscv64-elf-gcc libusb
brew trust riscv-software-src/riscv
brew tap riscv-software-src/riscv
brew install riscv-tools
```

Otherwise, refer to https://github.com/cnlohr/ch32fun/wiki/Installation

### Submodule

```bash
git submodule update --init firmware/ch32fun
```

## Build and flash

```bash
cd firmware/needle
make build
```


1. Hold the BOOT button (solder jumper).
2. Plug in USB (or press RST while holding BOOT).
3. Run `make flash` in another terminal, then release BOOT.

### Reflash without BOOT

After the first flash, the firmware jumps to the bootloader when it receives `BOOT` over USB serial:

```bash
make autoflash
```

See comment in [`needle.c`](firmware/needle/needle.c) regarding `USB_SERIAL_DEBUG`

If the chip ever stops responding to USB flashing:

 - hold BOOT
 - plug in USB

```bash
make 5xx_unbrick
```
