#!/usr/bin/env python3
"""Dump a PE32 image: sections, imports (with bound IAT values), relocs summary."""
import sys, struct
d = open(sys.argv[1], 'rb').read()
pe = struct.unpack_from('<I', d, 0x3c)[0]
nsec = struct.unpack_from('<H', d, pe+6)[0]; opt = pe+24
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = []
for i in range(nsec):
    o = opt + osz + 40*i
    name = d[o:o+8].rstrip(b'\0').decode()
    vs, va, rs, ro = struct.unpack_from('<IIII', d, o+8)
    ch = struct.unpack_from('<I', d, o+36)[0]
    secs.append((name, va, vs, ro, rs, ch))
    print('%-8s va %06x vsize %06x raw %06x rsize %06x chars %08x' % (name, va, vs, ro, rs, ch))
def r2o(rva):
    for n, va, vs, ro, rs, ch in secs:
        if va <= rva < va + max(vs, rs): return rva - va + ro
def cstr(rva):
    o = r2o(rva); return d[o:d.index(b'\0', o)].decode()
dd = lambda i: struct.unpack_from('<II', d, opt+96+8*i)
imp, _ = dd(1)
p = r2o(imp)
while True:
    oft, ts, fwd, nm, ft = struct.unpack_from('<IIIII', d, p); p += 20
    if not nm: break
    print('\n%s  (ILT %x IAT %x timestamp %x)' % (cstr(nm), oft, ft, ts))
    i = 0
    while True:
        e = struct.unpack_from('<I', d, r2o(oft)+4*i)[0]
        if not e: break
        iat = struct.unpack_from('<I', d, r2o(ft)+4*i)[0]
        if e & 0x80000000: s = 'ord %d' % (e & 0xffff)
        else: s = '%s (hint %d)' % (cstr(e+2), struct.unpack_from('<H', d, r2o(e))[0])
        print('  %-40s IAT=%08x' % (s, iat)); i += 1
