#!/usr/bin/env python3
"""Recompute the PE OptionalHeader.CheckSum (imagehlp CheckSumMappedFile algorithm).
Usage: pechecksum.py <file.exe>"""
import sys, struct
f = sys.argv[1]; d = bytearray(open(f, 'rb').read())
pe = struct.unpack_from('<I', d, 0x3c)[0]; off = pe + 24 + 64
struct.pack_into('<I', d, off, 0)
s = 0
data = d + (b'\0' if len(d) & 1 else b'')
for (w,) in struct.iter_unpack('<H', data):
    s += w; s = (s & 0xffff) + (s >> 16)
s = (s & 0xffff) + (s >> 16)
struct.pack_into('<I', d, off, (s + len(d)) & 0xffffffff)
open(f, 'wb').write(d)
