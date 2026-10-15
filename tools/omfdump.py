#!/usr/bin/env python3
"""Dump LEDATA bytes per segment from an OMF .obj (fixup locations unresolved)."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
p = 0; lnames = ['']; segs = []; data = {}
def idx(b, i):
    v = b[i]
    return ((v & 0x7f) << 8 | b[i+1], i+2) if v & 0x80 else (v, i+1)
while p < len(d):
    t = d[p]; n = struct.unpack_from('<H', d, p+1)[0]; r = d[p+3:p+3+n-1]; p += 3+n
    if t == 0x96:
        i = 0
        while i < len(r): l = r[i]; lnames.append(r[i+1:i+1+l].decode()); i += 1+l
    elif t in (0x98, 0x99):
        i = 1; ln = struct.unpack_from('<H', r, i)[0]; i += 2
        nm, i = idx(r, i); segs.append(lnames[nm])
    elif t in (0xa0, 0xa1):
        s, i = idx(r, 0); off = struct.unpack_from('<H', r, i)[0]; i += 2
        buf = data.setdefault(segs[s-1], bytearray())
        if len(buf) < off: buf.extend(b'\0'*(off-len(buf)))
        buf[off:off+len(r)-i] = r[i:]
for s, b in data.items():
    print(s, len(b)); print(b.hex())
