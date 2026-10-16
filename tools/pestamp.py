#!/usr/bin/env python3
"""Set build timestamps in a PE image: the file header TimeDateStamp and the
resource directory TimeDateStamps (which CVTRES sets to its run time).
Usage: pestamp.py <file.exe> <header stamp hex> [resource stamp hex]"""
import sys, struct
f = sys.argv[1]; hts = int(sys.argv[2], 16); rts = int(sys.argv[3], 16) if len(sys.argv) > 3 else hts
d = bytearray(open(f, 'rb').read())
pe = struct.unpack_from('<I', d, 0x3c)[0]; struct.pack_into('<I', d, pe+8, hts)
nsec = struct.unpack_from('<H', d, pe+6)[0]; osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
rva = struct.unpack_from('<I', d, pe+24+96+16)[0]
if rva:
    base = r2o(rva)
    def walk(off):
        struct.pack_into('<I', d, base+off+4, rts)
        nn, ni = struct.unpack_from('<HH', d, base+off+12)
        for i in range(nn+ni):
            nm, ent = struct.unpack_from('<II', d, base+off+16+8*i)
            if ent & 0x80000000: walk(ent & 0x7fffffff)
    walk(0)
open(f, 'wb').write(d)
