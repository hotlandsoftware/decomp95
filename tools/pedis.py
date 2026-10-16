#!/usr/bin/env python3
"""Disassemble a range of the original PE with symbol annotation.
Usage: pedis.py <exe> <symbols.txt> <data.txt> <func name | start-end hex>"""
import sys, re, subprocess, bisect
exe, symf, dataf, what = sys.argv[1:5]
def load(f):
    m = {}
    for l in open(f):
        x = l.split('#')[0].split()
        if len(x) >= 2: m[int(x[0], 16)] = x[1]
    return m
funcs = load(symf); data = load(dataf)
allsyms = sorted({**data, **funcs}.items())
keys = [a for a, n in allsyms]
if '-' in what:
    s, e = (int(x, 16) for x in what.split('-'))
else:
    s = [a for a, n in funcs.items() if n == what][0]
    e = min([a for a in funcs if a > s] or [s + 0x400])
def name(v):
    i = bisect.bisect_right(keys, v) - 1
    if i < 0: return None
    a, n = allsyms[i]
    if v == a: return n
    if v - a < 0x110 and a >= 0x405000: return '%s+%#x' % (n, v - a)
    return None
out = subprocess.run(['objdump', '-d', '-M', 'intel', '--no-show-raw-insn',
                      '--start-address=%#x' % s, '--stop-address=%#x' % e, exe],
                     capture_output=True, text=True).stdout
def sub(m):
    v = int(m.group(0), 16); n = name(v)
    if n is None and 0x401000 <= v < 0x405000:
        n = funcs.get(v)
    return n if n else m.group(0)
for l in out.splitlines():
    m = re.match(r'\s+([0-9a-f]+):\s+(.*)$', l)
    if not m: continue
    a = int(m.group(1), 16)
    ins = re.sub(r'0x40[0-9a-f]{4}', sub, re.sub(r'\s+', ' ', m.group(2)))
    lab = funcs.get(a)
    if lab and a != s: print('%s:' % lab)
    print('  %04x %s' % (a - s, ins))
