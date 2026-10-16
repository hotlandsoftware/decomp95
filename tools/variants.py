#!/usr/bin/env python3
"""Try source variants of one function inside its full file and report matches.

Usage: variants.py <src file> <func name> <variants.py-file> [cl flags...]
The variants file defines VARIANTS = {label: full function source text}.
The function in <src file> (from its signature line to the closing brace at
column 0) is replaced by each variant; the file is compiled with cc32.sh and
compared with tools/coffcmp.py.  Prints the per-variant result for <func> and
the file's total match count.

Environment:
  PADS=a-b   also try each NPPAD symbol-padding offset in a..b (see nppad.h)
  FDIFF=1    append the number of differing instruction lines (finer metric)
  JOBS=n     parallel compiles (default 8)
  LOCS=a-b   also try NPLOCn (n unused locals, see nppad.h) at the top of the
             function body for each n in a..b (replacing any NPLOC already there)
  BEST=n     print only the n best results (sorted by FDIFF lines, then bytes)"""
import sys, os, re, subprocess, tempfile
from concurrent.futures import ThreadPoolExecutor
src, func, varf = sys.argv[1:4]; flags = sys.argv[4:] or ['/Ogsy', '/Gz']
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
text = open(src).read()
m = re.search(r'^\S[^\n]*\b%s\s*\(' % re.escape(func), text, re.M)
start = m.start(); end = text.index('\n}\n', start) + 3
ns = {}; exec(open(varf).read(), ns)
out = tempfile.mkdtemp(prefix='var', dir=os.environ.get('TMPDIR'))
inc = 'Z:' + os.path.dirname(os.path.abspath(src)).replace('/', '\\')

def padded(s, n):
    if n is None: return s
    s = re.sub(r'^#define NPPAD \d+\n', '', s, flags=re.M)
    i = s.index('#include "nppad.h"\n')
    return s[:i] + '#define NPPAD %d\n' % n + s[i:]

pads = [None]
if os.environ.get('PADS'):
    a, _, b = os.environ['PADS'].partition('-'); pads = list(range(int(a), int(b or a) + 1))
def withlocs(body, n):
    body = re.sub(r'^    NPLOC\d+\n', '', body, flags=re.M)
    if n == 0: return body
    i = body.index('{\n') + 2
    return body[:i] + '    NPLOC%d\n' % n + body[i:]
locs = [None]
if os.environ.get('LOCS'):
    a, _, b = os.environ['LOCS'].partition('-'); locs = list(range(int(a), int(b or a) + 1))
jobs = [(l + ('' if q is None else '+L%d' % q) + ('' if p is None else '@%d' % p),
         b if q is None else withlocs(b, q), p)
        for l, b in ns['VARIANTS'].items() for q in locs for p in pads]

def run(job):
    i, (label, body, pad) = job
    d = os.path.join(out, str(i)); os.makedirs(d)
    f = os.path.join(d, os.path.basename(src))
    open(f, 'w').write(padded(text[:start] + body.strip('\n') + '\n' + text[end:], pad))
    subprocess.run([root + '/tools/cc32.sh', f, d] + flags + ['/I' + inc], capture_output=True)
    obj = os.path.join(d, os.path.splitext(os.path.basename(src))[0] + '.obj')
    r = subprocess.run(['python3', root + '/tools/coffcmp.py', obj, root + '/original/SYSTEM/notepad.exe',
                        root + '/src/notepad/symbols.txt'], capture_output=True, text=True).stdout
    line = [l for l in r.splitlines() if l.startswith(func + ' ')]
    total = r.strip().splitlines()[-1] if r.strip() else '?'
    nd = None
    if os.environ.get('FDIFF') and os.path.exists(obj):
        dd = subprocess.run(['python3', root + '/tools/fdiff.py', obj, root + '/original/SYSTEM/notepad.exe',
                             root + '/src/notepad/symbols.txt', func, '-a'], capture_output=True, text=True).stdout
        nd = sum(1 for l in dd.splitlines() if l.startswith('|'))
    res = (line[0][29:] if line else 'missing').strip()
    mb = re.search(r'(\d+) bytes differ', res)
    key = (nd if nd is not None else 0, int(mb.group(1)) if mb else (0 if 'MATCH' in res else 1 << 30), i)
    subprocess.run(['rm', '-rf', d])
    return key, '%-12s %-60s %s%s' % (label, res, total, '' if nd is None else ' [%d diff lines]' % nd)

with ThreadPoolExecutor(int(os.environ.get('JOBS', 8))) as ex:
    results = list(ex.map(run, enumerate(jobs)))
if os.environ.get('BEST'):
    results = sorted(results)[:int(os.environ['BEST'])]
for _, s in results:
    print(s)
subprocess.run(['rm', '-rf', out])
