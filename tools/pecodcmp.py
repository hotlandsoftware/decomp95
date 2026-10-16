#!/usr/bin/env python3
"""Compare functions in a VC++ 2.0 /FAc listing with an original PE image.

Usage: pecodcmp.py <file.cod> <orig.exe> <symbols.txt> [-v]
symbols.txt lines: "<hex address> <name>" (C name, e.g. FreeIt).
Zero bytes in the listing (unresolved relocations) are wildcards.
Prints one line per function; -v shows the first mismatching instructions."""
import sys, re, struct
cod, exe, symf = sys.argv[1:4]; verbose = '-v' in sys.argv
d = open(exe, 'rb').read()
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]; base = struct.unpack_from('<I', d, pe+52)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def va2off(va):
    rva = va - base
    for vs, sva, rs, ro in secs:
        if sva <= rva < sva + rs: return rva - sva + ro
syms = {}
for line in open(symf):
    line = line.split('#')[0].split()
    if len(line) >= 2: syms[line[1]] = int(line[0], 16)
funcs = []; cur = None
for raw in open(cod, encoding='latin-1'):
    line = raw.rstrip('\r\n')
    m = re.match(r'^_?(\w+?)(@\d+)?\s+PROC NEAR', line)
    if m: cur = [m.group(1), []]; funcs.append(cur); continue
    if re.match(r'^\S+\s+ENDP', line): cur = None; continue
    if cur is None: continue
    m = re.match(r'^  ([0-9a-f]{5})\t((?:[0-9a-f]{2} ?)+)\t*(.*)$', line)
    if m:
        cur[1].append([int(m.group(1), 16), bytes.fromhex(m.group(2).replace(' ', '')), m.group(3).strip()]); continue
    m = re.match(r'^\t((?:[0-9a-f]{2} ?)+)\t(.*)$', line)
    if m and cur[1]:
        cur[1][-1][1] += bytes.fromhex(m.group(1).replace(' ', ''))
        cur[1][-1][2] = (cur[1][-1][2] + ' ' + m.group(2).strip()).strip()
total = ok_funcs = 0
for name, ins in funcs:
    ins = [i for i in ins if not i[2].startswith('npad')]
    if not ins: continue
    start = ins[0][0]
    if name not in syms: print('%-28s (no address in symbols.txt)' % name); continue
    o = va2off(syms[name]); bad = []
    for off, by, txt in ins:
        orig = d[o + off - start:o + off - start + len(by)]
        if not all(x == y or y == 0 for x, y in zip(orig, by)): bad.append((off, by, orig, txt))
    size = ins[-1][0] + len(ins[-1][1]) - start
    total += 1; ok_funcs += not bad
    print('%-28s %08x %5d bytes  %s' % (name, syms[name], size,
          'MATCH' if not bad else '%d/%d instructions differ' % (len(bad), len(ins))))
    if verbose:
        for off, by, orig, txt in bad[:12]:
            print('      %05x ours %-18s orig %-18s %s' % (off, by.hex(), orig.hex(), txt))
print('%d/%d functions match' % (ok_funcs, total))
