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
cd firmware/blink
make build
```


1. Hold the BOOT button (solder jumper).
2. Plug in USB (or press RST while holding BOOT).
3. Run `make flash` in another terminal, then release BOOT.

If the chip ever stops responding to USB flashing, recover with:

```bash
make 5xx_unbrick    # hold BOOT, plug in USB, then run this
```
