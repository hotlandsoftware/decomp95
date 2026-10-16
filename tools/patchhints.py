#!/usr/bin/env python3
"""Copy an import library and patch the hint values of its import members.

Usage: patchhints.py <in.lib> <out.lib> <reference.exe>
The hints are taken from <reference.exe>'s import table (for the DLL named by
the library), i.e. the values the original import library carried.  Only the
2-byte hint in each member's .idata$6 record is changed; sizes are unchanged."""
import sys, struct
src, dst, ref = sys.argv[1:4]
lib = bytearray(open(src, 'rb').read())

# --- hints wanted, from the reference image: {dllname_lower: {name: hint}}
d = open(ref, 'rb').read()
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
def cstr(o): return d[o:d.index(b'\0', o)].decode()
want = {}
p = r2o(struct.unpack_from('<I', d, pe+24+96+8)[0])
while True:
    oft, ts, fwd, nm, ft = struct.unpack_from('<IIIII', d, p); p += 20
    if not nm: break
    dll = cstr(r2o(nm)).lower(); i = 0
    while True:
        e = struct.unpack_from('<I', d, r2o(oft) + 4*i)[0]
        if not e: break
        if not e & 0x80000000:
            o = r2o(e); want.setdefault(dll, {})[cstr(o+2)] = struct.unpack_from('<H', d, o)[0]
        i += 1

# --- walk archive members
import os
p = 8; patched = 0; dll = os.path.basename(src).lower().rsplit('.', 1)[0] + '.dll'
while p < len(lib):
    size = int(lib[p+48:p+58]); body = p + 60
    if lib[body:body+2] == b'\x4c\x01':                     # i386 COFF object member
        ns = struct.unpack_from('<H', lib, body+2)[0]
        ohsz = struct.unpack_from('<H', lib, body+16)[0]
        for s in range(ns):
            q = body + 20 + ohsz + 40*s
            name = bytes(lib[q:q+8]).rstrip(b'\0')
            rs, rp = struct.unpack_from('<II', lib, q+16)
            if name == b'.idata$6' and rs >= 2:
                raw = bytes(lib[body+rp:body+rp+rs])
                if raw.lower().rstrip(b'\0').endswith(b'.dll'):
                    continue                                    # import descriptor: DLL name
                fname = raw[2:raw.index(b'\0', 2)].decode()
                if dll and fname in want.get(dll, {}):
                    struct.pack_into('<H', lib, body+rp, want[dll][fname]); patched += 1
    p = body + size + (size & 1)
open(dst, 'wb').write(lib)
print('%s: patched %d hints (wanted %d for %s)' % (dst, patched, len(want.get(dll, {})), dll))
