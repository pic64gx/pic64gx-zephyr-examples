import argparse
import shutil

def main(args):
    in_file = open(args.payload, "rb")
    out_file = open(args.block_device,"wb")

    shutil.copyfileobj(in_file, out_file)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="""script to write to eMMC.
Requires to be run with elevated privileges"""
    )

    parser.add_argument(
        "payload", type=str, help="The payload to write: ex, output.bin"
    )
    parser.add_argument(
        "block_device", type=str, help="The block device name (or devlink)"
    )

    args = parser.parse_args()

    main(args)
