# Copyright (c) 2024 Microchip Technologies
# SPDX-License-Identifier: Apache-2.0


from west.commands import WestCommand
import os
from pathlib import Path
import serial
import time
import subprocess
import multiprocessing
import pyudev
import getpass
import sys
import textwrap


class FlashPayload(WestCommand):
    def __init__(self):
        super().__init__(
            "flash-payload",
            "an example west extension command",
            """ \
            flash-payload is an interactive command that will guide a
            user through flashing a payload on to a board's eMMC.
            It does this through interacting with the Hart Software
            Services (HSS), by:
            1. Prompting the user to power on the device
            2. Interrupting the HSS from booting
            3. Issues the commands to mount the eMMC as a block device
                on the host
            4. Finds the device on the host and prompts the user to
               verify the device
            5. Prompts the user to enter their password, which is
                required to write to the eMMC
            6. On completion, sends the boot command to the HSS to boot
                the payload
            """,
        ),
        self.base_path = Path.cwd()

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name, help=self.help, description=self.description
        )
        parser.add_argument("payload", help="payload binary, ex: output.bin")

        parser.add_argument(
            "serial", help="the UART serial port for the HSS, ex: /dev/ttyUSBX"
        )
        return parser

    def device_settling_time(self, my_time=1):
        """
        A wrapper around the sleep method. Used to aid comprehension of
        the script
        """
        time.sleep(my_time)

    def read_phrase(self, serial_port, phrase):
        """
        Reads lines coming from the serial port, looking for a secific
        phrase. Loop until phrase is found.
        """
        while True:
            data_received = serial_port.readline().decode()
            if phrase in data_received:
                print("\nhss: {}".format(phrase))
                return data_received

    def send_ctrl_c(self, ser):
        """
        Sending CTRL+C
        """
        ser.write(b"\x03")
        print("sent CTR+C\r\n")
        self.device_settling_time(5)

    def send_phrase(self, serial_port, phrase):
        """
        Writes a phrase to an open serial port. Checks if the phrase
        is a 'Bytes' or 'String' object and will encode if necessary.
        """
        if isinstance(phrase, bytes):
            bytes_written = serial_port.write(phrase)
        else:
            bytes_written = serial_port.write(phrase.encode())

        if bytes_written == None:
            return -1
        else:
            print("sent: {}".format(phrase))
            time.sleep(1)
            self.device_settling_time(5)
            return bytes_written

    def power_on_message_prompt(self):
        """
        Message printed to the console prompting a user to power on
        their device. Gets called in a process, so it's non blocking
        """
        print("power on your device:")
        start_time = time.time()
        while time.time() - start_time < 60:
            time.sleep(1)
            print(".", end="", flush=True)

    def hss_confirm_connection(self):
        """
        The first output from the Hart Software Services (HSS) application.
        When this is captured from the serial port, we can terminate
        the power on message prompt process
        """
        self.read_phrase(
            self.ser, "HSS: decompressing from eNVM to L2 Scratch ... Passed"
        )

    def hss_interrupt(self):
        """
        The HSS gives the user an oppertunity to interrupt the boot procedure
        by pressing any key within a given time period. Wait to be prompted,
        then send a character.
        """
        self.read_phrase(self.ser, "Press a key to enter CLI, ESC to skip")
        self.send_phrase(self.ser, "c\r\n")
        self.read_phrase(self.ser, "Type HELP for list of commands")

    def hss_mount_mmc(self):
        """
        The HSS can choose to unpack from the eMMC or SD Card. In this
        case, we want to boot from eMMC
        """
        self.send_phrase(self.ser, "mmc\r\n")
        self.read_phrase(
            self.ser, "Selecting SDCARD/MMC (fallback) as boot source"
        )
        self.send_phrase(self.ser, "usbdmsc\r\n")
        self.read_phrase(self.ser, "Attempting to select eMMC")

    def hss_boot(self):
        """
        Writing to the eMMC is done, send the ctrl+c sequence, then
        instruct the HSS to boot
        """
        self.send_ctrl_c(self.ser)
        self.send_phrase(self.ser, "boot\r\n")

    def get_user_input(self):
        while True:
            user_input = input(
                "Please enter 'y' for yes or 'n' for no: "
            ).lower()
            if user_input == "y" or user_input == "n":
                return user_input
            else:
                print("Invalid input. Please enter 'y' for yes or 'n' for no.")

    def get_block_device_info(self):
        """
        Finds the block device on the host machine. Prompts the user
        to verify if it is correct.
        """
        for device in pyudev.Context().list_devices(
            subsystem="block", ID_MODEL="PIC64GXSoC_msd"
        ):
            print(
                "Found device:\nID_SERIAL={},\nDEVNAME={}\n".format(
                    device.get("ID_SERIAL"), device.get("DEVNAME")
                )
            )
            print("Choose this Device?\r\n")
            if self.get_user_input() == "y":
                return device.get("DEVNAME")
            else:
                continue
        print("Could not find eMMC Mass Storage Device")
        exit(-1)

    def set_serial(self, baudrate=115200):
        """
        sets up the serial connection for the HSS.
        """
        try:
            self.ser = serial.Serial(self.port_name, baudrate)
            self.ser.is_open
            return 0
        except Exception as e:
            print(
                "please check your serial connection: {}".format(
                    self.port_namet
                ),
                e,
            )
        return e.errno

    def get_writer_path(self):
        """
        returns the path to an auxillary script included in the scripts/
        directory. This script will be run in a subprocess with elevated
        privileges
        """
        script_path = os.path.join(
            self.manifest.repo_abspath, "scripts", "write_to_emmc.py"
        )
        return script_path

    def write_to_emmc(self, devname):
        """
        Calls a subprocess to write to the eMMC. Prompts the user to
        enter their password which gets used to elevate the privilegs
        of the subprocess. Checks if the password is incorrect and returns
        the return code, which in turns allows this function to be called
        again.
        """
        s_path = self.get_writer_path()

        password = getpass.getpass(
            prompt="\nEnter your password (required to write to block device):\n"
        )

        command = ["sudo", "-S", sys.executable, s_path, self.payload, devname]
        try:
            retval = subprocess.run(
                command,
                input=password.encode(),
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=True,
            )
            return retval.returncode
        except Exception as e:
            caught_error = e.stderr.decode()
            if (
                "no password" in caught_error
                or "incorrect password" in caught_error
            ):
                return e.returncode
            else:
                raise Exception(
                    """
                    Failue to write to eMMC.
                    Check connections and power cycle before trying again\n
                    """,
                    e.stderr.decode()
                )

    def do_run(self, args, unknown_args):
        self.port_name = args.serial
        self.payload = args.payload
        self.set_serial()

        power_on_prompt = multiprocessing.Process(
            target=self.power_on_message_prompt, daemon=True
        )
        power_on_prompt.start()

        process2 = multiprocessing.Process(target=self.hss_confirm_connection)
        process2.start()
        process2.join()

        power_on_prompt.terminate()

        self.hss_interrupt()
        self.hss_mount_mmc()
        self.device_settling_time(5)

        devname = self.get_block_device_info()

        while self.write_to_emmc(devname):
            print("...\n")

        self.device_settling_time(5)
        self.hss_boot()

        print("how about that!")
