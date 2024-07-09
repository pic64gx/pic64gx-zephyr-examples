# Microchip PIC64GX SoC Zephyr Support

This repository provides Microchip PIC64GX SoC support for the Zephyr RTOS project with:

- Sample Zephyr applications for Microchip's PIC64GX SoC
- Extended `West` Commands to generate and flash a payload to the eMMC

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
| `apps/amp_exemple` |
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

The instructions above generate a payload called `output.bin`

## Additional Reading

[Zephyr User Manual](https://docs.zephyrproject.org/latest/)

[Zephyr Project - Github](https://github.com/zephyrproject-rtos/zephyr)
