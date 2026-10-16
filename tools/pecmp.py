#!/usr/bin/env python3
"""Compare two PE images: whole file, headers, and per-section byte counts."""
import sys, struct
a = open(sys.argv[1], 'rb').read(); b = open(sys.argv[2], 'rb').read()
if a == b:
    print('MATCH: %s is byte-identical (%d bytes)' % (sys.argv[2], len(a))); sys.exit(0)
print('DIFFERENT (orig %d bytes, ours %d bytes)' % (len(a), len(b)))
def secs(d):
    pe = struct.unpack_from('<I', d, 0x3c)[0]; n = struct.unpack_from('<H', d, pe+6)[0]
    osz = struct.unpack_from('<H', d, pe+20)[0]
    out = {}
    for i in range(n):
        o = pe+24+osz+40*i
        name = d[o:o+8].rstrip(b'\0').decode()
        vs, va, rs, ro = struct.unpack_from('<IIII', d, o+8)
        out[name] = (va, vs, ro, rs)
    return pe, out
pa, sa = secs(a); pb, sb = secs(b)
hdr = [i for i in range(min(0x400, len(a), len(b))) if a[i] != b[i]]
print('[headers] %d bytes differ%s' % (len(hdr), (' (first at %#x)' % hdr[0]) if hdr else ''))
for name in list(sa) + [n for n in sb if n not in sa]:
    if name not in sa or name not in sb:
        print('[%s] only in %s' % (name, 'original' if name in sa else 'ours')); continue
    va, vs, ro, rs = sa[name]; vb, vsb, rob, rsb = sb[name]
    x = a[ro:ro+rs]; y = b[rob:rob+rsb]
    same = sum(1 for p, q in zip(x, y) if p == q)
    print('[%-7s] orig va %06x vsize %05x raw %05x | ours va %06x vsize %05x raw %05x | %d/%d equal'
          % (name, va, vs, rs, vb, vsb, rsb, same, len(x)))
sys.exit(1)
