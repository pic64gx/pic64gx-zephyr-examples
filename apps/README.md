# PIC64GX SoC Zephyr Applications

## Blinky

The `blinky` application is a very basic example designed to familarize the user
with the concepts of Zephyr such as using a peripherals API, and compiling applications using `Devicetree overlays`.

The application blink's an LED forever using the GPIO API.
The application comes with multiple Devicetree overlay files, located in the `boards/` directory.
The `pic64gx_curiosity_kit.overlay` file is chosen by default.
This overlay disables the `e51` hart, configures the console on `uart 1` and chooses a memory region in DDR to execute from.

## Button

A simple button demo showcasing the use of GPIO input with interrupts.
The sample prints a message to the console each time a button is pressed.

## Zephyr OS Application Build Directory

The output of the build process can be found in:

```bash
build/zephyr/
```

Here you will find the application ELF binary, zephyr.elf, etc.

## Debugging a Zephyr OS Application using Microchip SoftConsole IDE

`Eclipse CDT - 4` style projects can be built for any of the examples above by appending the argument below:

```bash
 - G "Eclipse CDT4 - Unix Makefiles"
```

[Follow the instructions here](../softconsole-launch-configs/README.md) to start a debug session using Microchips SoftConsole IDE.
