#!/usr/bin/env python3
"""Rebuild an .ico file from an NE executable's GROUP_ICON resource.
Usage: extract_ico.py <file.exe> <group name or id> <out.ico>
Used at build time so original artwork never needs to be committed."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
ne = struct.unpack_from('<H', d, 0x3c)[0]
rt = ne + struct.unpack_from('<H', d, ne+0x24)[0]
shift = struct.unpack_from('<H', d, rt)[0]
res = {}
p = rt + 2
while True:
    t, cnt = struct.unpack_from('<HH', d, p); p += 8
    if not t: break
    for _ in range(cnt):
        off, ln, fl, rid = struct.unpack_from('<HHHH', d, p); p += 12
        key = rid & 0x7fff if rid & 0x8000 else d[rt+rid+1:rt+rid+1+d[rt+rid]].decode().upper()
        res[(t & 0x7fff, key)] = (off << shift, ln << shift)
want = sys.argv[2].upper()
want = int(want) if want.isdigit() else want
off, ln = res[(14, want)]
_, typ, n = struct.unpack_from('<HHH', d, off)
entries = [struct.unpack_from('<BBBBHHIH', d, off+6+14*i) for i in range(n)]
out = bytearray(struct.pack('<HHH', 0, 1, n)); images = bytearray()
base = 6 + 16*n
for w, h, c, r, planes, bpp, size, iid in entries:
    io, il = res[(3, iid)]
    out += struct.pack('<BBBBHHII', w, h, c, r, planes, bpp, size, base + len(images))
    images += d[io:io+size]
open(sys.argv[3], 'wb').write(out + images)
