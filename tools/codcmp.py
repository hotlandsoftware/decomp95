#!/usr/bin/env python3
"""Compare code bytes from a CL /Fc listing against a range of an original file.
Zero bytes in the listing (unresolved fixups) are treated as wildcards.
Usage: codcmp.py <file.COD> <orig.exe> <fileoffset hex>"""
import sys, re
b = bytearray(); rows = []
for line in open(sys.argv[1], encoding='latin-1'):
    m = re.match(r'\s+\*\*\* ([0-9a-f]{6})\t((?:[0-9a-f]{2} ?)+)\s*\t(.*)', line.rstrip('\r\n'))
    if m:
        off = int(m.group(1), 16); by = bytes.fromhex(m.group(2).replace(' ', ''))
        rows.append((off, by, m.group(3))); b[off:off+len(by)] = by
o = open(sys.argv[2], 'rb').read(); base = int(sys.argv[3], 16); a = o[base:base+len(b)]
bad = 0
for off, by, txt in rows:
    orig = a[off:off+len(by)]
    ok = all(x == y or y == 0 for x, y in zip(orig, by))
    if not ok:
        bad += 1
        if bad <= 25: print('%04x  ours %-16s orig %-16s %s' % (off, by.hex(), orig.hex(), txt))
print('%d/%d instructions match' % (len(rows)-bad, len(rows)))
