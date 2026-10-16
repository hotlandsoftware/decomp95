#!/usr/bin/env python3
"""List PE resources; with 'strings' dump the string table. peres.py file.exe [strings]"""
import sys, struct
d = open(sys.argv[1], 'rb').read()
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
rsrc = struct.unpack_from('<I', d, pe+24+96+16)[0]; base = r2o(rsrc)
def name(v):
    if v & 0x80000000:
        o = base + (v & 0x7fffffff); n = struct.unpack_from('<H', d, o)[0]
        return d[o+2:o+2+2*n].decode('utf-16le')
    return v
def walk(off, path):
    nn, ni = struct.unpack_from('<HH', d, base+off+12)
    for i in range(nn+ni):
        nm, ent = struct.unpack_from('<II', d, base+off+16+8*i)
        if ent & 0x80000000: yield from walk(ent & 0x7fffffff, path+[name(nm)])
        else:
            drva, size, cp, _ = struct.unpack_from('<IIII', d, base+ent)
            yield path+[name(nm)], drva, size
TYPES = {1:'CURSOR',2:'BITMAP',3:'ICON',4:'MENU',5:'DIALOG',6:'STRING',9:'ACCELERATOR',12:'GROUP_CURSOR',14:'GROUP_ICON',16:'VERSION'}
for path, drva, size in walk(0, []):
    t = TYPES.get(path[0], path[0])
    if len(sys.argv) > 2 and sys.argv[2] == 'strings':
        if t != 'STRING': continue
        o = r2o(drva); blk = path[1]
        for i in range(16):
            n = struct.unpack_from('<H', d, o)[0]; s = d[o+2:o+2+2*n].decode('utf-16le'); o += 2+2*n
            if n: print('%4d %r' % ((blk-1)*16+i, s))
    else:
        print('%-12s %-10s lang %-6s rva %06x size %5d' % (t, path[1], path[2], drva, size))
