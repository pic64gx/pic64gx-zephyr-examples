# Copyright (c) 2024 Microchip Technologies
# SPDX-License-Identifier: Apache-2.0


from west.commands import WestCommand
from west import log
import os
from pathlib import Path
import sys
import zipfile
import subprocess


class GeneratePayload(WestCommand):
    def __init__(self):
        super().__init__(
            "generate-payload",
            "an example west extension command",
            """\
generate-payload will invoke the payload-generator executable to
generate a payload for Microchip's PIC64GX SoC. This payload can
then be flashed to the eMMC of a given board. The example application is
required to provide a configuration file written in yaml. Documentation
on the payload generator and the Hart Software Services can be found at:
https://github.com/pic64gx/pic64gx-hart-software-services.git""",
        )
        self.base_path = Path.cwd()

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(
            self.name, help=self.help, description=self.description
        )
        parser.add_argument(
            "config_file",
            help="the configuration yaml file for the payload generator",
        )

        parser.add_argument(
            "payload_name", help="give a neame to the generated payload"
        )

        return parser

    def verify_config_file(self, config_file):
        """Verifies that there is a payload generator yaml configuration
        file. If there is no valid configuration file, the script exits
        and prints the config file path that was passed to it.
        """
        full_path = os.path.join(self.base_path, config_file)
        if os.path.isfile(full_path):
            print("found configuration file: {}".format(full_path))
            self.config_file = full_path
        else:
            print(
                """
                  {} is not a valid path to a config file
                  Hint: make sure you invoke 'west generate-payload' from the
                  zephyr workspace directory, (i.e, zephyrproject/)
                  """.format(
                    full_path
                )
            )
            exit(-2)

    def verify_payload_generator(self):
        """
        First, searches for the binaries/* directory. If successful it
        will store the path to the executable as a data member. If
        unsuccessful, it calls out to look in the blobs directory for
        the zipped file. If thats found, it will be unzipped. If it's
        not found, it prompts the user to invoke 'west blobs fetch'.
        Finally, if running on Linux, it will set the execute bit in the
        exucatable's file permissions
        """
        executable = ""
        executable_path = ""

        if sys.platform.startswith("linux"):
            executable = "hss-payload-generator"
        elif sys.platform.startswith("win"):
            executable = "hss-payload-generator.exe"

        lst = list(self.base_path.glob("**/binaries/{}".format(executable)))

        if len(lst) > 0:
            executable_path = list(
                self.base_path.glob("**/binaries/{}".format(executable))
            )[0]
            print(
                "found payload generator exectuable: {}".format(
                    executable_path
                )
            )
            self.executable = executable_path
        else:
            print(
                """
                  Cannot find the payload generator executable,
                  Trying to find hss-payload-generator.zip:
                  """
            )
            self.unzip_payload_generator()

        if sys.platform.startswith("linux"):
            print("the executable path")
            os.chmod(self.executable, 0o777)

    def unzip_payload_generator(self):
        zip_fp = Path.joinpath(
            self.base_path,
            self.manifest.repo_path,
            "zephyr",
            "blobs",
            "payload-generator",
            "hss-payload-generator.zip",
        )
        print(zip_fp)
        if zip_fp.exists():
            print("found hss-payload-generator.zip: unzipping: ...")
        else:
            print(
                """
                  Cannot find hss-payload-generator.zip:
                  Hint: make sure you have fetched it using 'west blobs fetch',
                  (after you have executed 'west update') and that you are
                  executing this command from the zephyr workspace
                  (i.e, zephyrproject)
                  """
            )
            exit(-2)

        with zipfile.ZipFile(zip_fp, "r") as zip:
            zip.extractall()

        self.verify_payload_generator()

    def invoke_payload_generator(self, payload):
        subprocess.run([self.executable, "-vvvwc", self.config_file, payload])

    def do_run(self, args, unknown_args):
        self.verify_config_file(args.config_file)
        self.verify_payload_generator()
        self.invoke_payload_generator(args.payload_name)
