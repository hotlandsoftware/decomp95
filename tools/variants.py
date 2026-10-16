#!/usr/bin/env python3
"""Try source variants of one function inside its full file and report matches.

Usage: variants.py <src file> <func name> <variants.py-file> [cl flags...]
The variants file defines VARIANTS = {label: full function source text}.
The function in <src file> (from its signature line to the closing brace at
column 0) is replaced by each variant; the file is compiled with cc32.sh and
compared with tools/coffcmp.py.  Prints the per-variant result for <func> and
the file's total match count."""
import sys, os, re, subprocess, tempfile
src, func, varf = sys.argv[1:4]; flags = sys.argv[4:] or ['/Ogsy', '/Gz']
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
text = open(src).read()
m = re.search(r'^\S[^\n]*\b%s\s*\(' % re.escape(func), text, re.M)
start = m.start(); end = text.index('\n}\n', start) + 3
ns = {}; exec(open(varf).read(), ns)
out = tempfile.mkdtemp(prefix='var', dir=os.environ.get('TMPDIR'))
ext = os.path.splitext(src)[1]
def padded(s, n):
    s = re.sub(r'^#define NPPAD \d+\n#include "nppad.h"\n', '', s, flags=re.M)
    if n is None: return s
    i = [m.end() for m in re.finditer(r'^#include[^\n]*\n', s, re.M)][-1]
    return s[:i] + '#define NPPAD %d\n#include "nppad.h"\n' % n + s[i:]
pads = [None]
if os.environ.get('PADS'):
    a, _, b = os.environ['PADS'].partition('-'); pads = list(range(int(a), int(b or a) + 1))
jobs = [(l + ('' if p is None else '@%d' % p), b, p) for l, b in ns['VARIANTS'].items() for p in pads]
for label, body, pad in jobs:
    f = os.path.join(out, os.path.basename(src))
    open(f, 'w').write(padded(text[:start] + body.strip('\n') + '\n' + text[end:], pad))
    inc = 'Z:' + os.path.dirname(os.path.abspath(src)).replace('/', '\\')
    subprocess.run([root + '/tools/cc32.sh', f, out] + flags + ['/I' + inc], capture_output=True)
    obj = os.path.join(out, os.path.splitext(os.path.basename(src))[0] + '.obj')
    r = subprocess.run(['python3', root + '/tools/coffcmp.py', obj, root + '/original/SYSTEM/notepad.exe',
                        root + '/src/notepad/symbols.txt'], capture_output=True, text=True).stdout
    line = [l for l in r.splitlines() if l.startswith(func + ' ')]
    total = r.strip().splitlines()[-1] if r.strip() else '?'
    extra = ''
    if os.environ.get('FDIFF') and os.path.exists(obj):
        d = subprocess.run(['python3', root + '/tools/fdiff.py', obj, root + '/original/SYSTEM/notepad.exe',
                            root + '/src/notepad/symbols.txt', func, '-a'], capture_output=True, text=True).stdout
        extra = ' [%d diff lines]' % sum(1 for l in d.splitlines() if l.startswith('|'))
    print('%-12s %-60s %s%s' % (label, (line[0][29:] if line else 'missing').strip(), total, extra))
    if os.path.exists(obj): os.remove(obj)
