#!/usr/bin/env python3
"""Rebuild .ico files from a PE image's GROUP_ICON resources.
Usage: peicons.py <file.exe> <outdir>   -> writes icon<N>.ico per group."""
import sys, struct, os
d = open(sys.argv[1], 'rb').read(); outdir = sys.argv[2]
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
base = r2o(struct.unpack_from('<I', d, pe+24+96+16)[0])
res = {}
def walk(off, path):
    nn, ni = struct.unpack_from('<HH', d, base+off+12)
    for i in range(nn+ni):
        nm, ent = struct.unpack_from('<II', d, base+off+16+8*i)
        if ent & 0x80000000: walk(ent & 0x7fffffff, path+[nm])
        else:
            drva, size, cp, _ = struct.unpack_from('<IIII', d, base+ent)
            res[(path[0], path[1])] = d[r2o(drva):r2o(drva)+size]
walk(0, [])
os.makedirs(outdir, exist_ok=True)
for (t, n), g in sorted(res.items()):
    if t != 14: continue
    _, typ, cnt = struct.unpack_from('<HHH', g, 0)
    ents = [struct.unpack_from('<BBBBHHIH', g, 6+14*i) for i in range(cnt)]
    hdr = bytearray(struct.pack('<HHH', 0, 1, cnt)); imgs = bytearray(); off = 6 + 16*cnt
    for w, h, c, r, planes, bpp, size, iid in ents:
        img = res[(3, iid)]
        hdr += struct.pack('<BBBBHHII', w, h, c, r, planes, bpp, len(img), off + len(imgs))
        imgs += img
    open(os.path.join(outdir, 'icon%d.ico' % n), 'wb').write(hdr + imgs)
    print('icon%d.ico: %d images' % (n, cnt))
