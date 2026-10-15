#!/usr/bin/env python3
"""Compare two NE executables: whole file, then header / per-segment detail."""
import sys, struct

def segs(d):
    ne = struct.unpack_from('<H', d, 0x3c)[0]
    n = struct.unpack_from('<H', d, ne+0x1c)[0]
    st = ne + struct.unpack_from('<H', d, ne+0x22)[0]
    al = struct.unpack_from('<H', d, ne+0x32)[0]
    out = []
    for i in range(n):
        off, ln, fl, mn = struct.unpack_from('<HHHH', d, st+8*i)
        out.append((off << al, ln or 0x10000 if off else 0, fl, mn))
    return ne, out

def diffs(a, b, base=0, limit=16):
    res = [i for i in range(min(len(a), len(b))) if a[i] != b[i]]
    for i in res[:limit]:
        print('    +%04x: orig %02x  ours %02x' % (base+i, a[i], b[i]))
    if len(res) > limit: print('    ... %d more' % (len(res)-limit))
    if len(a) != len(b): print('    length: orig %#x  ours %#x' % (len(a), len(b)))
    return len(res) + abs(len(a)-len(b))

a = open(sys.argv[1], 'rb').read(); b = open(sys.argv[2], 'rb').read()
if a == b:
    print('MATCH: %s is byte-identical (%d bytes)' % (sys.argv[2], len(a))); sys.exit(0)
print('DIFFERENT (orig %d bytes, ours %d bytes)' % (len(a), len(b)))
nea, sa = segs(a); neb, sb = segs(b)
print('[MZ stub]');  diffs(a[:nea], b[:neb])
print('[NE header]'); diffs(a[nea:nea+0x40], b[neb:neb+0x40])
for i, (x, y) in enumerate(zip(sa, sb)):
    print('[seg%d] orig len %#x flags %#x min %#x | ours len %#x flags %#x min %#x'
          % (i+1, x[1], x[2], x[3], y[1], y[2], y[3]))
    da, db = a[x[0]:x[0]+x[1]], b[y[0]:y[0]+y[1]]
    same = sum(1 for p, q in zip(da, db) if p == q)
    print('    %d/%d bytes equal (%.1f%%)' % (same, len(da), 100.0*same/max(1, len(da))))
    diffs(da, db)

def tables(d, ne):
    h = lambda o: struct.unpack_from('<H', d, ne+o)[0]
    t = {}
    t['resource table'] = d[ne+h(0x24):ne+h(0x26)]
    t['resident names'] = d[ne+h(0x26):ne+h(0x28)]
    t['module refs']    = [d[ne+h(0x2a)+o+1:ne+h(0x2a)+o+1+d[ne+h(0x2a)+o]] for o in
                           struct.unpack_from('<%dH' % h(0x1e), d, ne+h(0x28))]
    t['imported names'] = d[ne+h(0x2a):ne+h(0x04)]
    t['entry table']    = d[ne+h(0x04):ne+h(0x04)+h(0x06)]
    nr = struct.unpack_from('<I', d, ne+0x2c)[0]
    t['nonresident names'] = d[nr:nr+h(0x20)]
    for i, (fo, ln, fl, mn) in enumerate(segs(d)[1]):
        if fl & 0x100:
            n = struct.unpack_from('<H', d, fo+ln)[0]
            t['seg%d relocs' % (i+1)] = d[fo+ln:fo+ln+2+8*n]
    return t
ta, tb = tables(a, nea), tables(b, neb)
for k in ta:
    print('[%s] %s' % (k, 'same' if ta[k] == tb.get(k) else 'DIFFERENT'))
    if ta[k] != tb.get(k) and isinstance(ta[k], list): print('    orig', ta[k], '\n    ours', tb.get(k))
sys.exit(1)
