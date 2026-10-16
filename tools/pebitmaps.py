#!/usr/bin/env python3
"""Write each BITMAP resource of a PE image as a .bmp file.
Usage: pebitmaps.py <file.exe> <outdir>   -> writes bitmap<N>.bmp per resource."""
import sys, struct, os
d = open(sys.argv[1], 'rb').read(); outdir = sys.argv[2]
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
base = r2o(struct.unpack_from('<I', d, pe+24+96+16)[0])
def entries(off):
    nn, ni = struct.unpack_from('<HH', d, base+off+12)
    return [struct.unpack_from('<II', d, base+off+16+8*i) for i in range(nn+ni)]
for typ, ent in entries(0):
    if typ != 2: continue
    for nm, e2 in entries(ent & 0x7fffffff):
        for lang, e3 in entries(e2 & 0x7fffffff):
            drva, size = struct.unpack_from('<II', d, base+e3)
            dib = d[r2o(drva):r2o(drva)+size]
            hsz, = struct.unpack_from('<I', dib, 0)
            bits, clr = struct.unpack_from('<HI', dib, 14)[0], struct.unpack_from('<I', dib, 32)[0]
            ncol = clr or (1 << bits if bits <= 8 else 0)
            off = 14 + hsz + 4 * ncol
            hdr = b'BM' + struct.pack('<IHHI', 14 + len(dib), 0, 0, off)
            name = os.path.join(outdir, 'bitmap%d.bmp' % nm)
            open(name, 'wb').write(hdr + dib)
            print(name)
