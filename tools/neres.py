#!/usr/bin/env python3
"""List NE resources; optionally dump one: neres.py file.exe [TYPE ID]"""
import sys, struct
d = open(sys.argv[1], 'rb').read()
ne = struct.unpack_from('<H', d, 0x3c)[0]
rt = ne + struct.unpack_from('<H', d, ne+0x24)[0]
shift = struct.unpack_from('<H', d, rt)[0]; p = rt + 2
TYPES = {1:'CURSOR',2:'BITMAP',3:'ICON',4:'MENU',5:'DIALOG',6:'STRING',7:'FONTDIR',8:'FONT',
         9:'ACCELERATOR',10:'RCDATA',12:'GROUP_CURSOR',14:'GROUP_ICON',16:'VERSION'}
def name(v):
    if v & 0x8000: return v & 0x7fff
    return d[rt+v+1:rt+v+1+d[rt+v]].decode()
want = sys.argv[2:]
print('align shift', shift)
while True:
    t, cnt = struct.unpack_from('<HH', d, p)
    if not t: break
    tn = TYPES.get(t & 0x7fff, t & 0x7fff) if t & 0x8000 else name(t)
    p += 8
    for i in range(cnt):
        off, ln, fl, rid = struct.unpack_from('<HHHH', d, p); p += 12
        rn = name(rid)
        print('%-12s %-8s file %#06x len %#06x flags %#06x' % (tn, rn, off << shift, ln << shift, fl))
        if want and str(tn) == want[0] and str(rn) == want[1]:
            blob = d[off << shift:(off << shift) + (ln << shift)]
            import binascii; print(binascii.hexlify(blob, ' ', 2).decode()); print(blob)
