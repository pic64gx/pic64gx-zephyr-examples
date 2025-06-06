# Microchip PIC64GX SoC Zephyr Support

This repository provides Microchip PIC64GX SoC support for the Zephyr RTOS project with:

- Sample Zephyr applications for Microchip's PIC64GX SoC
- Extended `West` Commands to generate payload for SD card compatible with HSS bootloader
## Prerequisites

This section describes the requirements needed before building and flashing
a Zephyr application.

### Supported Build Hosts

This document assumes you are running on a modern Linux system. The process documented here was tested using Ubuntu 20.04/18.04 LTS.

### Install Zephyr SDK and Build System on Host PC

To build Zephyr applications on a host PC, please follow the installation instructions for the Zephyr SDK and build system found in the [Zephyr OS Install Guide](https://docs.zephyrproject.org/latest/getting_started/index.html)

## Build Instructions

The following commands make a workspace and clone this repository into a directory called `pic64gx-soc`:

```bash
mkdir zephyr_workspace && cd zephyr_workspace
git clone https://github.com/pic64gx/pic64gx-zephyr-examples.git -b pic64gx pic64gx-soc
```

The next step initializes the workspace:

```bash
cd pic64gx-soc
west init -l
```

Now using Zephyr's build-system, `West`, pull in Zephyr's code base:

```bash
cd ../
west update
west zephyr-export
source zephyr/zephyr-env.sh
```

To build a Zephyr application, use the command:

```bash
west build -p -b pic64gx_curiosity_kit <application>
```

Or to build a Zephyr application and generate an Eclipse CDT-4 based project
which can be imported into Microchip's `SoftConsole` IDE:

```bash
west build -p -b pic64gx_curiosity_kit <application> -G "Eclipse CDT4 - Unix Makefiles"
```

Where `pic64gx_curiosity_kit` is the supported board for PIC64GX Curiosity Kit, `<application>` is the Zephyr OS application.

The table below lists the applications available for PIC64GX Curiosity Kit that demonstrate various functionallity:

| `application` |
| --- |
| `apps/amp_example` |
| `apps/amp_example_openamp` |
| `apps/blinky` |
| `apps/button` |

## Using `West` Commands for PIC64GX SoC

### Python Requirements

This repository comes with scripts that extend Zephyr's build system,
`West`. The scripts are written in `python` and can be found in the `scripts/` directory.
The scripts have python packages requirements which can be installed with `pip`:

```bash
pip install -r scripts/requirements.txt
```

Information on the `West` extensions for PIC64GX SoC can be seen by:

```bash
west help
```

The output should contain:

```bash
extension commands from project manifest (path: pic64gx-soc):
  generate-payload:     invoke the Hart Software Services (HSS) payload
                        generator to generate a payload suitable for
                        flashing to the boards eMMC
  flash-payload:        flash a payload onto the boards eMMC.
```

### Fetching Blobs With `West`

Zephyr's build system `West` has a mechanism for fetching binaries and executables from source control. These are known as `blobs`. This repository is capable of fetching the `Hart Software Services (HSS)`, Payload Generator executable. To do this:

```bash
west blobs fetch
```

This will retrieve the the Payload Generator Executable. Note, this is required to use the West extension `generate-payload`

## Generating a payload

The `generate-payload` West command utilizes the Hart Software Services Payload Generator to generate a binary that is suitable to be flashed to a devices eMMC. More information on the HSS Payload Generator can be found [here](https://github.com/pic64gx/pic64gx-hart-software-services/blob/pic64gx/tools/hss-payload-generator/README.md)

To generate a payload, a compiled Zephyr application (ex: zephyr.elf) and a configuration file written in yaml are required. Example configuration files are provided in `payload-configs/`.

The instructions below demonstrates how to use `generate-payload`:

```bash
# from the zephyr workspace:
#
# build the smp example
west build -p -b pic64gx_curiosity_kit pic64gx-soc/apps/blinky

# generate a payload for an SMP, DDR, application
west generate-payload pic64gx-soc/payload-configs/single_hart_ddr.yaml output.bin
```

## Flashing a payload

Once the payload is generated the easiest way to flash is to use,
assuming sdcard is on /dev/sda
```
sudo dd if=output.bin of=/dev/sda
```
to check your sdcard directory (before after/after pluging the card)
```
lsblk
```
Warning:
PLease be extremely careful about the dd command, you can wipe your entire system disk with it, please double check sdcard location

## debugging

### 1st terminal
In one terminal or in background, run openocd
Download and untar the PIC64GX OpenOCD release from https://github.com/microchip-fpga/openocd/releases/tag/v0.12.0-mchp.0.0.1

```
cd xpack-openocd-0.12.0-3
./bin/openocd --command "set DEVICE pic64gx" -f board/microchip_riscv_efp5.cfg
```

In case PATH have been updated (ex: $HOME/.local/xPacks/openocd/xpack-openocd-0.12.0-3/bin)
```
openocd --command "set DEVICE pic64gx" -f board/microchip_riscv_efp5.cfg
```

### 2nd terminal
In a second terminal, run gdb, also available with xPacks
(ie: https://xpack-dev-tools.github.io/riscv-none-elf-gcc-xpack/)
Or using the zephyr sdk toolchain
(ex: $HOME/.local/opt/zephyr-sdk-0.16.5-1/riscv64-zephyr-elf/bin/riscv64-zephyr-elf-gdb
The patch depends on the install location)

```
riscv-none-elf-gdb
```
In GDB
```
target extended-remote localhost:3333
```

ex:
```
phm@ph-emdalo:~/.local/xPacks/@xpack-dev-tools/riscv-none-elf-gcc/13.2.0-2.1/.content/bin$ riscv-none-elf-gdb
GNU gdb (xPack GNU RISC-V Embedded GCC x86_64) 13.2
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-pc-linux-gnu --target=riscv-none-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
(gdb) target remote localhost:3333
Remote debugging using localhost:3333
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
warning: multi-threaded target stopped without sending a thread-id, using first non-exited thread
0x000000000a003e3e in ?? ()
(gdb) thread 5
[Switching to thread 5 (Thread 5)]
#0  0x0000000091c025a8 in ?? ()
(gdb) symbol-file /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/build/openamp_linux/zephyr/zephyr_openamp_rsc_table.elf
Reading symbols from /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/build/openamp_linux/zephyr/zephyr_openamp_rsc_table.elf...
(gdb) bt
#0  0x0000000091c025a8 in arch_cpu_idle ()
    at /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/zephyr/arch/riscv/core/cpu_idle.c:15
#1  0x0000000091c0631e in k_cpu_idle ()
    at /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/zephyr/include/zephyr/kernel.h:5849
#2  idle (unused1=unused1@entry=0x91c09ea0 <_kernel>, unused2=unused2@entry=0x0, unused3=unused3@entry=0x0)
    at /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/zephyr/kernel/idle.c:89
#3  0x0000000091c017e2 in z_thread_entry (entry=0x91c06312 <idle>, p1=0x91c09ea0 <_kernel>, p2=0x0, p3=0x0)
    at /home/phm/Projects/Microchip/zephyr/zephyr_pic64gx/zephyr/lib/os/thread_entry.c:48
#4  0xaaaaaaaaaaaaaaaa in ?? ()
Backtrace stopped: frame did not save the PC
(gdb)
```
Note: The current example is a amp_example running on hart 5 (u54_4)

## UART
### On linux
You might need to add specific udev rules to be able to use the pic64gx-curiosity-kit embedded JTAG/UART
Create /etc/udev/rules.d/70-microchip.rules :
```
# Update right for flashpro 6
ACTION=="add", ATTRS{idVendor}=="1514", ATTRS{idProduct}=="200b", \
GROUP="dialout" MODE="0666"

# Bind ftdi_sio driver to all input
ACTION=="add", ATTRS{idVendor}=="1514", ATTRS{idProduct}=="2008", \
ATTRS{product}=="Embedded FlashPro5", ATTR{bInterfaceNumber}!="00", \
RUN+="/sbin/modprobe ftdi_sio", RUN+="/bin/sh -c 'echo 1514 2008 > /sys/bus/usb-serial/drivers/ftdi_sio/new_id'"

# Unbind ftdi_sio driver for channel A which should be the JTAG
SUBSYSTEM=="usb", DRIVER=="ftdi_sio", ATTR{bInterfaceNumber}=="00", ATTR{interface}=="Embedded FlashPro5",\
RUN+="/bin/sh -c 'echo $kernel > /sys/bus/usb/drivers/ftdi_sio/unbind'"

# Helper (optional)
KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="01", \
SYMLINK+="ttyUSB-FlashPro5B" GROUP="dialout" MODE="0666"

KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="02", \
SYMLINK+="ttyUSB-FlashPro5C" GROUP="dialout" MODE="0666"

KERNEL=="ttyUSB[0-9]*", SUBSYSTEM=="tty", SUBSYSTEMS=="usb", \
ATTRS{interface}=="Embedded FlashPro5", ATTRS{bInterfaceNumber}=="03", \
SYMLINK+="ttyUSB-FlashPro5D" GROUP="dialout" MODE="0666"
```

## Additional Reading

[Zephyr User Manual](https://docs.zephyrproject.org/latest/)

[Zephyr Project - Github](https://github.com/zephyrproject-rtos/zephyr)
