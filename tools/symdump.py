#!/usr/bin/env python3
"""Dump a 16-bit MAPSYM .SYM file: segments and their symbols (offset, name)."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
def pstr(o): return d[o+1:o+1+d[o]].decode('latin-1')
nxt, flags, _, entseg, ncon, pcon, nseg, pseg = struct.unpack_from('<HBBHHHHH', d, 0)
print('module %s  (%d segments)' % (pstr(0x0f), nseg))
p = pseg
for _ in range(nseg):
    o = p << 4
    nxt, nsym, psym, segnum = struct.unpack_from('<HHHH', d, o)
    big = d[o+0x0e] & 1
    print('seg %d %s (%d syms)' % (segnum, pstr(o+0x14), nsym))
    for i in range(nsym):
        so = o + struct.unpack_from('<H', d, o + psym + 2*i)[0]
        if big: off, nm = struct.unpack_from('<I', d, so)[0], pstr(so+4)
        else:   off, nm = struct.unpack_from('<H', d, so)[0], pstr(so+2)
        print('  %04x %s' % (off, nm))
    p = nxt
