#!/usr/bin/env python3
"""Compile a notepad source file at every NPPAD offset (0-31) and tabulate
per-function differing-byte counts (0 = match).

Usage: padscan.py <src file> [first-last]"""
import sys, os, re, subprocess, tempfile
from concurrent.futures import ThreadPoolExecutor
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = sys.argv[1]
a, _, b = (sys.argv[2] if len(sys.argv) > 2 else '0-31').partition('-')
pads = range(int(a), int(b or a) + 1)
text = re.sub(r'^#define NPPAD \d+\n', '', open(src).read(), flags=re.M)
inc = 'Z:' + os.path.dirname(os.path.abspath(src)).replace('/', '\\')
tmp = tempfile.mkdtemp(prefix='pad', dir=os.environ.get('TMPDIR'))

def run(n):
    d = os.path.join(tmp, str(n)); os.makedirs(d)
    i = text.index('#include "nppad.h"\n')
    f = os.path.join(d, os.path.basename(src))
    open(f, 'w').write(text[:i] + '#define NPPAD %d\n' % n + text[i:])
    subprocess.run([root + '/tools/cc32.sh', f, d, '/Ogsy', '/Gz', '/W3', '/I' + inc], capture_output=True)
    obj = os.path.join(d, os.path.splitext(os.path.basename(src))[0] + '.obj')
    out = subprocess.run(['python3', root + '/tools/coffcmp.py', obj, root + '/original/SYSTEM/notepad.exe',
                          root + '/src/notepad/symbols.txt'], capture_output=True, text=True).stdout
    res = []
    for l in out.splitlines():
        if 'functions match' in l: continue
        w = l.split()
        res.append((w[0], 0 if w[-1] == 'MATCH' else int(w[4])))
    return n, res

with ThreadPoolExecutor(8) as ex:
    rows = list(ex.map(run, pads))
names = [r[0] for r in rows[0][1]]
print('pad ' + ' '.join('%5.5s' % n for n in names) + '  match')
for n, res in rows:
    print('%3d ' % n + ' '.join('%5d' % v for _, v in res) + '  %d' % sum(v == 0 for _, v in res))
subprocess.run(['rm', '-rf', tmp])
