#!/usr/bin/env python3
"""Side-by-side disassembly diff of one function: ours (.obj) vs original PE.

Usage: fdiff.py <file.obj> <orig.exe> <symbols.txt> <name> [-a]
Shows instructions (mnemonic + operands, addresses stripped) with a '|' marker
on differing lines; -a shows all lines (default: only around differences)."""
import sys, re, subprocess, difflib
objf, exe, symf, name = sys.argv[1:5]; show_all = '-a' in sys.argv
syms = {}
for line in open(symf):
    f = line.split('#')[0].split()
    if len(f) >= 2: syms[f[1]] = int(f[0], 16)
addrs = sorted(syms.values())
start = syms[name]
def norm(ins):
    ins = re.sub(r'\s+', ' ', ins.strip())
    ins = re.sub(r'0x40[0-9a-f]{4}\b', 'ADDR', ins)          # absolute addresses
    ins = re.sub(r'ds:0x[0-9a-f]+', 'ds:ADDR', ins)
    return ins
def dis_obj():
    """Disassemble our function from the COFF object (bytes + relocations)."""
    import struct, tempfile, os
    o = open(objf, 'rb').read()
    mach, ns, ts, symptr, nsym, ohsz, ch = struct.unpack_from('<HHIIIHH', o, 0)
    strtab = symptr + 18*nsym
    def symname(p):
        if o[p:p+4] == b'\0\0\0\0':
            so = strtab + struct.unpack_from('<I', o, p+4)[0]; return o[so:o.index(b'\0', so)].decode()
        return o[p:p+8].rstrip(b'\0').decode('latin-1')
    syms = []; i = 0
    while i < nsym:
        p = symptr + 18*i
        val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', o, p+8)
        syms.append((symname(p), sec, val, typ, cls)); i += 1 + naux
        syms.extend([None]*naux)
    target = None
    for e in syms:
        if e and e[1] > 0 and (e[3] & 0x30) == 0x20:
            n = e[0]; n = n[1:] if n[:1] in '_@?' else n
            if n.split('@')[0] == name: target = e
    if not target: return []
    secidx = target[1] - 1
    p = 20 + ohsz + 40*secidx
    vs, va, rs, rp, relp, lnp, nrel, nln, sch = struct.unpack_from('<IIIIIIHHI', o, p+8)
    data = o[rp:rp+rs]
    ends = sorted(e[2] for e in syms if e and e[1] == target[1] and (e[3] & 0x30) == 0x20 and e[2] > target[2])
    end = ends[0] if ends else rs
    relocs = {}
    for r in range(nrel):
        roff, rsym, rtype = struct.unpack_from('<IIH', o, relp+10*r)
        relocs[roff] = syms[rsym][0] if syms[rsym] else '?'
    with tempfile.NamedTemporaryFile(suffix='.bin', delete=False) as f:
        f.write(data[target[2]:end]); tmp = f.name
    out = subprocess.run(['objdump', '-D', '-b', 'binary', '-m', 'i386', '-M', 'intel',
                          '--no-show-raw-insn', tmp], capture_output=True, text=True).stdout
    os.unlink(tmp)
    lines = []
    raw = [(int(m.group(1), 16), m.group(2)) for m in
           (re.match(r'\s+([0-9a-f]+):\s+(.*)$', l) for l in out.splitlines()) if m]
    for k, (off, ins) in enumerate(raw):
        nxt = raw[k+1][0] if k+1 < len(raw) else end - target[2]
        rel = [relocs[x] for x in range(target[2]+off, target[2]+nxt) if x in relocs]
        lines.append((off, ins + ('  <' + rel[0] + '>' if rel else '')))
    return lines
ours = dis_obj()
size = (ours[-1][0] + 16) if ours else 64
nxt = [a for a in addrs if a > start]
end = nxt[0] if nxt else start + size
out = subprocess.run(['objdump', '-d', '-M', 'intel', '--no-show-raw-insn',
                      '--start-address=%#x' % start, '--stop-address=%#x' % end, exe],
                     capture_output=True, text=True).stdout
orig = []
for l in out.splitlines():
    m = re.match(r'\s+([0-9a-f]+):\s+(.*)$', l)
    if m: orig.append((int(m.group(1), 16) - start, m.group(2)))
# branch targets: compare as relative offsets
def fixbr(lst, base):
    res = []
    for off, ins in lst:
        ins = re.sub(r'\s+', ' ', ins.strip())
        rel = ''
        m = re.search(r'  <(.+)>$', ins)
        if m: rel = m.group(1); ins = ins[:m.start()]
        ins = re.sub(r' <[^>]*>$', '', ins)
        m = re.match(r'(j\w+|call|loop\w*) (0x)?([0-9a-f]+)$', ins)
        if m and m.group(1) == 'call':
            n = 'call REL'
        elif m:
            n = '%s +%#x' % (m.group(1), int(m.group(3), 16) - base)
        else:
            n = norm(ins)
        if rel:
            n = re.sub(r'(?<![\w\[+-])0x[0-9a-f]+(?![\w\]])', 'ADDR', n)
            n += '  ' + rel
        res.append(n)
    return res
a = fixbr(orig, start); b = fixbr(ours, 0)
sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
W = 60
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal' and not show_all:
        if i2 - i1 > 4:
            for k in range(i1, i1+2): print('  %04x %-*s   %s' % (orig[k][0], W, a[k][:W], b[j1+k-i1][:W]))
            print('  ...')
            for k in range(i2-2, i2): print('  %04x %-*s   %s' % (orig[k][0], W, a[k][:W], b[j1+k-i1][:W]))
            continue
    n = max(i2-i1, j2-j1)
    for k in range(n):
        x = a[i1+k] if i1+k < i2 else ''; y = b[j1+k] if j1+k < j2 else ''
        off = '%04x' % orig[i1+k][0] if i1+k < i2 else '    '
        print('%s %s %-*s %s %s' % (' ' if tag == 'equal' else '|', off, W, x[:W], '|' if tag != 'equal' else ' ', y[:W]))
