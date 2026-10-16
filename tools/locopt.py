#!/usr/bin/env python3
"""Greedy NPLOCn tuning for one notepad source file.

Usage: locopt.py <src file> [max n=31] [depth=1] [--write]

Walks the functions in file order.  For each one that does not match, tries
NPLOC1..NPLOCmax at the top of its body (a function's code depends only on
what precedes it, so earlier choices are never disturbed by later ones) and
keeps the smallest n that makes it byte-identical.  Functions that cannot be
matched this way keep no NPLOC.  With --write the result is saved to the file."""
import sys, os, re, subprocess, tempfile
from concurrent.futures import ThreadPoolExecutor
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
args = [a for a in sys.argv[1:] if not a.startswith('--')]
src = args[0]; nmax = int(args[1]) if len(args) > 1 else 31
depth = int(args[2]) if len(args) > 2 else 1
inc = 'Z:' + os.path.dirname(os.path.abspath(src)).replace('/', '\\')
exe = os.environ.get('EXE', root + '/original/SYSTEM/notepad.exe')
syms = os.environ.get('SYMS', os.path.join(os.path.dirname(os.path.abspath(src)), 'symbols.txt'))
tmp = tempfile.mkdtemp(prefix='loc', dir=os.environ.get('TMPDIR'))
cnt = [0]

def compile_cmp(text):
    cnt[0] += 1
    d = os.path.join(tmp, str(cnt[0]) + '_' + str(os.getpid()) + '_' + str(id(text)))
    os.makedirs(d)
    f = os.path.join(d, os.path.basename(src)); open(f, 'w').write(text)
    subprocess.run([root + '/tools/cc32.sh', f, d] + os.environ.get('CL_FLAGS', '/Ogsy /Gz /W3').split() + ['/I' + inc], capture_output=True)
    obj = os.path.join(d, os.path.splitext(os.path.basename(src))[0] + '.obj')
    out = subprocess.run(['python3', root + '/tools/coffcmp.py', obj, exe, syms], capture_output=True, text=True).stdout
    subprocess.run(['rm', '-rf', d])
    res = {}
    for l in out.splitlines():
        if 'functions match' in l: continue
        w = l.split(); res[w[0]] = (w[-1] == 'MATCH')
    return res, [l.split()[0] for l in out.splitlines() if 'functions match' not in l]

def body_span(text, func):
    name = func.split('@')[0].lstrip('?')
    m = re.search(r'^\S[^\n]*\b%s\s*\(' % re.escape(name), text, re.M)
    return m.start(), text.index('\n}\n', m.start()) + 3

def setloc(text, func, n):
    s, e = body_span(text, func)
    body = re.sub(r'^    NPLOC\d+\n', '', text[s:e], flags=re.M)
    if n:
        i = body.index('{\n') + 2
        body = body[:i] + '    NPLOC%d\n' % n + body[i:]
    return text[:s] + body + text[e:]

text = open(src).read()
res, order = compile_cmp(text)
print('start: %d/%d' % (sum(res.values()), len(res)))
def curloc(text, func):
    s, e = body_span(text, func)
    m = re.search(r'^    NPLOC(\d+)\n', text[s:e], re.M)
    return int(m.group(1)) if m else 0

for k, func in enumerate(order):
    if res.get(func): continue
    # candidates: NPLOCn in the function itself, or (keeping earlier matches)
    # in one of the `depth` functions before it
    cands = [(func, n, setloc(text, func, n)) for n in range(0, nmax + 1)]
    for prev in order[max(0, k - depth):k]:
        p0 = curloc(text, prev)
        cands += [(prev, n, setloc(text, prev, n)) for n in range(0, nmax + 1) if n != p0]
    with ThreadPoolExecutor(8) as ex:
        results = list(ex.map(compile_cmp, [c[2] for c in cands]))
    ok = [i for i, (r, _) in enumerate(results)
          if r.get(func) and all(r.get(f) or not res.get(f) for f in order[:k])]
    if ok:
        i = min(ok, key=lambda i: (cands[i][0] != func, cands[i][1]))
        text = cands[i][2]; res = results[i][0]
        print('%-20s NPLOC%d in %s -> %d/%d' % (func, cands[i][1], cands[i][0], sum(res.values()), len(res)))
    else:
        print('%-20s no n matches' % func)
print('final: %d/%d' % (sum(res.values()), len(res)))
if '--write' in sys.argv:
    open(src, 'w').write(text)
subprocess.run(['rm', '-rf', tmp])
