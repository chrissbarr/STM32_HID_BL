# !/usr/bin/python

from pathlib import Path
import shutil
import subprocess
import sys
import binascii

filename_in = "sketch_may9a.ino.bin"
filename_out = "sd_fw.bin"
prefix = b'\xAA\x3D\x00\x01'

buf = open(filename_in,'rb').read()
crc32 = (binascii.crc32(buf) & 0xFFFFFFFF)
print(f"CRC32: {hex(crc32)}")

with open(filename_in, "rb") as old, open(filename_out, "wb") as new:
    # for byte in prefix:
    #     new.write(bytes(byte))
    new.write(prefix)
    new.write(crc32.to_bytes(4, byteorder="little", signed=False))
    new.write(old.read())