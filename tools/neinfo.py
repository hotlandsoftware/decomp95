#!/usr/bin/env python3
"""Print NE header, segments, imports and relocations (resolved by chain)."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
ne = struct.unpack_from('<H', d, 0x3c)[0]
H = lambda o, f='<H': struct.unpack_from(f, d, ne+o)[0]
print('linker %d.%d  flags %#x  autodata %d  heap %#x  stack %#x  cs:ip %d:%#x  ss:sp %d:%#x  expver %#x  align %d'
      % (d[ne+2], d[ne+3], H(0xc), H(0xe), H(0x10), H(0x12), H(0x16), H(0x14), H(0x1a), H(0x18), H(0x3e), H(0x32)))
it = ne + H(0x2a); mr = ne + H(0x28)
mods = [d[it+o+1:it+o+1+d[it+o]].decode() for o in struct.unpack_from('<%dH' % H(0x1e), d, mr)]
print('modules', mods)
rn = ne + H(0x26); print('resident names', d[rn:ne+H(0x28)])
nr = H(0x2c, '<I'); print('nonresident', d[nr:nr+H(0x20)])
et = ne + H(4); print('entry table', d[et:et+H(6)].hex())
al = H(0x32)
for i in range(H(0x1c)):
    off, ln, fl, mn = struct.unpack_from('<HHHH', d, ne+H(0x22)+8*i)
    fo = off << al
    print('seg%d file %#x len %#x flags %#x minalloc %#x' % (i+1, fo, ln, fl, mn))
    if not fl & 0x100: continue
    seg = d[fo:fo+ln]; n = struct.unpack_from('<H', d, fo+ln)[0]; p = fo+ln+2
    for j in range(n):
        t, f, o, a, b = struct.unpack_from('<BBHHH', d, p); p += 8
        if f & 3 == 1: tgt = '%s.%d' % (mods[a-1], b)
        elif f & 3 == 2: tgt = '%s!%s' % (mods[a-1], d[it+b+1:it+b+1+d[it+b]].decode())
        elif f & 3 == 0: tgt = 'seg%d:%#x' % (a & 0xff, b)
        else: tgt = 'osfixup %d' % a
        locs = [o]
        while not f & 4:
            nx = struct.unpack_from('<H', seg, locs[-1])[0]
            if nx == 0xffff or len(locs) > 64: break
            locs.append(nx)
        print('   type%d%s %-28s at %s' % (t, '+' if f & 4 else ' ', tgt, ' '.join('%#x' % x for x in locs)))
