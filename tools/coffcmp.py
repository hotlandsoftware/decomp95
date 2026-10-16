#!/usr/bin/env python3
"""Compare functions in a COFF .obj (VC++ 2.0) with an original PE image.

Usage: coffcmp.py <file.obj> <orig.exe> <symbols.txt> [-v] [-q]
symbols.txt: "<hex VA> <name>" per line (undecorated C name; '#' comments).
Bytes covered by relocations in the .obj are masked.  Each function is
matched by name against symbols.txt; -v prints mismatching byte ranges with
disassembly hints, -q prints only the summary line."""
import sys, struct
objf, exe, symf = sys.argv[1:4]; verbose = '-v' in sys.argv; quiet = '-q' in sys.argv
o = open(objf, 'rb').read(); d = open(exe, 'rb').read()
# --- PE
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]; base = struct.unpack_from('<I', d, pe+52)[0]
psecs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def va2off(va):
    rva = va - base
    for vs, sva, rs, ro in psecs:
        if sva <= rva < sva + rs: return rva - sva + ro
syms = {}
for line in open(symf):
    f = line.split('#')[0].split()
    if len(f) >= 2: syms[f[1]] = int(f[0], 16)
# --- COFF
mach, ns, ts, symptr, nsym, ohsz, ch = struct.unpack_from('<HHIIIHH', o, 0)
secs = []
for i in range(ns):
    p = 20 + ohsz + 40*i
    name = o[p:p+8].rstrip(b'\0').decode('latin-1')
    vs, va, rs, rp, relp, lnp, nrel, nln, sch = struct.unpack_from('<IIIIIIHHI', o, p+8)
    data = o[rp:rp+rs]; mask = bytearray(rs)
    for r in range(nrel):
        roff, rsym, rtype = struct.unpack_from('<IIH', o, relp+10*r)
        n = 4 if rtype in (6, 7, 0x14) else 2
        for k in range(n): mask[roff+k] = 1
    secs.append((name, data, mask, sch))
strtab = symptr + 18*nsym
def symname(p):
    if o[p:p+4] == b'\0\0\0\0':
        so = strtab + struct.unpack_from('<I', o, p+4)[0]; return o[so:o.index(b'\0', so)].decode()
    return o[p:p+8].rstrip(b'\0').decode('latin-1')
funcs = []; i = 0
while i < nsym:
    p = symptr + 18*i
    val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', o, p+8)
    nm = symname(p)
    if sec > 0 and (typ & 0x30) == 0x20 and cls in (2, 3):    # function symbol
        funcs.append((nm, sec-1, val))
    i += 1 + naux
def undecorate(n):
    n = n[1:] if n[:1] in '_@?' else n
    return n.split('@')[0]
total = good = 0
for nm, si, val in funcs:
    sname, data, mask, sch = secs[si]
    # function extent: up to next function in same section or section end
    ends = sorted(v for n2, s2, v in funcs if s2 == si and v > val)
    end = ends[0] if ends else len(data)
    cname = undecorate(nm)
    if cname not in syms:
        if not quiet: print('%-28s (no address in symbols file)' % cname)
        continue
    po = va2off(syms[cname]); orig = d[po:po + end - val]
    diffs = [k for k in range(end - val) if not mask[val+k] and data[val+k] != orig[k]]
    total += 1; good += not diffs
    if not quiet:
        print('%-28s %08x %5d bytes  %s' % (cname, syms[cname], end - val,
              'MATCH' if not diffs else '%d bytes differ (first at +%#x)' % (len(diffs), diffs[0])))
    if verbose and diffs:
        k0 = max(0, diffs[0] - 8)
        print('      ours %s' % ''.join('..' if mask[val+k] else '%02x' % data[val+k] for k in range(k0, min(end-val, k0+40))))
        print('      orig %s' % orig[k0:k0+40].hex())
print('%d/%d functions match' % (good, total))
sys.exit(0 if good == total else 1)
