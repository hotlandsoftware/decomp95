#!/usr/bin/env python3
"""Decompile the resources of a PE image into RC script text (menus, dialogs,
string tables, accelerators, version info).  Icons and bitmaps are referenced
by file name (extract them with tools/peicons.py and tools/pebitmaps.py).
Usage: res2rc.py <file.exe> [codepage=1252]"""
import sys, struct
d = open(sys.argv[1], 'rb').read()
CP = 'cp' + (sys.argv[2] if len(sys.argv) > 2 else '1252')
pe = struct.unpack_from('<I', d, 0x3c)[0]; nsec = struct.unpack_from('<H', d, pe+6)[0]
osz = struct.unpack_from('<H', d, pe+20)[0]
secs = [struct.unpack_from('<IIII', d, pe+24+osz+40*i+8) for i in range(nsec)]
def r2o(r):
    for vs, va, rs, ro in secs:
        if va <= r < va + max(vs, rs): return r - va + ro
base = r2o(struct.unpack_from('<I', d, pe+24+96+16)[0])
def rname(v):
    if v & 0x80000000:
        o = base + (v & 0x7fffffff); n = struct.unpack_from('<H', d, o)[0]
        return d[o+2:o+2+2*n].decode('utf-16le')
    return v
def walk(off, path):
    nn, ni = struct.unpack_from('<HH', d, base+off+12)
    for i in range(nn+ni):
        nm, ent = struct.unpack_from('<II', d, base+off+16+8*i)
        if ent & 0x80000000: yield from walk(ent & 0x7fffffff, path+[rname(nm)])
        else:
            drva, size, cp, _ = struct.unpack_from('<IIII', d, base+ent)
            yield path+[rname(nm)], d[r2o(drva):r2o(drva)+size]
def q(s):
    out = ''
    for ch in s:
        if ch == '"': out += '""'
        elif ch == '\\': out += '\\\\'
        elif ch == '\n': out += '\\n'
        elif ch == '\r': out += '\\r'
        elif ch == '\t': out += '\\t'
        elif ch == '\0': out += '\\0'
        elif ord(ch) < 32: out += '\\%03o' % ord(ch)
        else: out += ch
    return '"' + out + '"'
def ustr(b, o):
    e = o
    while b[e:e+2] != b'\0\0': e += 2
    return b[o:e].decode('utf-16le'), e + 2
def sz_or_ord(b, o):
    if b[o:o+2] == b'\xff\xff': return struct.unpack_from('<H', b, o+2)[0], o + 4
    return ustr(b, o)
def idn(v): return str(v) if isinstance(v, int) else v
res = list(walk(0, []))
out = []
for path, b in res:
    t, n = path[0], path[1]
    if t == 4:                                      # MENU
        out.append('%s MENU' % idn(n)); out.append('BEGIN')
        o = 4; depth = [1]; ind = '    '
        stack = []
        while o < len(b):
            flags = struct.unpack_from('<H', b, o)[0]; o += 2
            if flags & 0x10:                        # MF_POPUP
                text, o = ustr(b, o)
                out.append(ind * len(depth) + 'POPUP %s' % q(text)); out.append(ind * len(depth) + 'BEGIN')
                depth.append(flags & 0x80)
                continue
            mid = struct.unpack_from('<H', b, o)[0]; o += 2
            text, o = ustr(b, o)
            opts = []
            if flags & 1: opts.append('GRAYED')
            if flags & 2: opts.append('INACTIVE')
            if flags & 8: opts.append('CHECKED')
            if flags & 0x40: opts.append('MENUBARBREAK')
            if flags & 0x20: opts.append('MENUBREAK')
            if not text and mid == 0 and not (flags & ~0x80):
                out.append(ind * len(depth) + 'MENUITEM SEPARATOR')
            else:
                out.append(ind * len(depth) + 'MENUITEM %s, %d%s' % (q(text), mid, ''.join(', ' + x for x in opts)))
            last = flags & 0x80
            while last and len(depth) > 1:
                out.append(ind * (len(depth) - 1) + 'END')
                last = depth.pop()
            if last and len(depth) == 1: break
        out.append('END'); out.append('')
    elif t == 5 and b[:4] == b'\x01\x00\xff\xff':     # DIALOGEX
        hid, exstyle, style, cdit, x, y, cx, cy = struct.unpack_from('<IIIHhhhh', b, 4)
        o = 26
        menu, o = sz_or_ord(b, o); cls, o = sz_or_ord(b, o); cap, o = ustr(b, o)
        out.append('%s DIALOGEX %d, %d, %d, %d%s' % (idn(n), x, y, cx, cy, (', %d' % hid) if hid else ''))
        out.append('STYLE 0x%08x' % style)
        if exstyle: out.append('EXSTYLE 0x%08x' % exstyle)
        if cap: out.append('CAPTION %s' % q(cap))
        if menu: out.append('MENU %s' % idn(menu))
        if cls: out.append('CLASS %s' % (q(cls) if isinstance(cls, str) else cls))
        if style & 0x40:
            pt, wt, it, cs = struct.unpack_from('<HHBB', b, o); face, o = ustr(b, o + 6)
            out.append('FONT %d, %s, %d, %d, 0x%x' % (pt, q(face), wt, it, cs))
        out.append('BEGIN')
        CLS = {0x80: 'Button', 0x81: 'Edit', 0x82: 'Static', 0x83: 'ListBox', 0x84: 'ScrollBar', 0x85: 'ComboBox'}
        for i in range(cdit):
            o = (o + 3) & ~3
            ihid, ex, st, x, y, cx, cy, cid = struct.unpack_from('<IIIhhhhI', b, o); o += 24
            c, o = sz_or_ord(b, o); text, o = sz_or_ord(b, o)
            extra = struct.unpack_from('<H', b, o)[0]; o += 2 + extra
            cname = CLS.get(c, c) if isinstance(c, int) else c
            tx = q(text) if isinstance(text, str) else str(text)
            cid_s = '-1' if cid in (0xffff, 0xffffffff) else str(cid)
            out.append('    CONTROL %s, %s, %s, 0x%08x, %d, %d, %d, %d%s%s' % (tx, cid_s, q(cname) if isinstance(cname, str) else cname,
                       st, x, y, cx, cy, (', 0x%08x' % ex) if ex or ihid else '', (', %d' % ihid) if ihid else ''))
        out.append('END'); out.append('')
    elif t == 5:                                    # DIALOG
        style, exstyle, cdit, x, y, cx, cy = struct.unpack_from('<IIHhhhh', b, 0)
        o = 18
        menu, o = sz_or_ord(b, o); cls, o = sz_or_ord(b, o); cap, o = ustr(b, o)
        line = '%s DIALOG%s %d, %d, %d, %d' % (idn(n), ' DISCARDABLE' if True else '', x, y, cx, cy)
        out.append(line)
        out.append('STYLE 0x%08x' % style)
        if exstyle: out.append('EXSTYLE 0x%08x' % exstyle)
        if cap: out.append('CAPTION %s' % q(cap))
        if menu: out.append('MENU %s' % idn(menu))
        if cls: out.append('CLASS %s' % (q(cls) if isinstance(cls, str) else cls))
        if style & 0x40:
            pt = struct.unpack_from('<H', b, o)[0]; face, o = ustr(b, o+2)
            out.append('FONT %d, %s' % (pt, q(face)))
        out.append('BEGIN')
        CLS = {0x80: 'Button', 0x81: 'Edit', 0x82: 'Static', 0x83: 'ListBox', 0x84: 'ScrollBar', 0x85: 'ComboBox'}
        for i in range(cdit):
            o = (o + 3) & ~3
            st, ex, x, y, cx, cy, cid = struct.unpack_from('<IIhhhhH', b, o); o += 18
            c, o = sz_or_ord(b, o); text, o = sz_or_ord(b, o)
            extra = struct.unpack_from('<H', b, o)[0]; o += 2 + extra
            cname = CLS.get(c, c) if isinstance(c, int) else c
            tx = q(text) if isinstance(text, str) else str(text)
            cid_s = '-1' if cid == 0xffff else str(cid)
            out.append('    CONTROL %s, %s, %s, 0x%08x, %d, %d, %d, %d%s' % (tx, cid_s, q(cname) if isinstance(cname, str) else cname, st, x, y, cx, cy, (', 0x%08x' % ex) if ex else ''))
        out.append('END'); out.append('')
    elif t == 6:                                    # STRINGTABLE block
        o = 0; ents = []
        for i in range(16):
            ln = struct.unpack_from('<H', b, o)[0]; s = b[o+2:o+2+2*ln].decode('utf-16le'); o += 2 + 2*ln
            if ln: ents.append(((n-1)*16+i, s))
        out.append('STRINGTABLE'); out.append('BEGIN')
        for k, s in ents: out.append('    %d, %s' % (k, q(s)))
        out.append('END'); out.append('')
    elif t == 9:                                    # ACCELERATORS
        out.append('%s ACCELERATORS' % idn(n)); out.append('BEGIN')
        for o in range(0, len(b), 8):
            fl, key, cmd, _ = struct.unpack_from('<HHHH', b, o)
            opts = []
            if fl & 1: opts.append('VIRTKEY')
            else: opts.append('ASCII')
            if fl & 2: opts.append('NOINVERT')
            if fl & 4: opts.append('SHIFT')
            if fl & 8: opts.append('CONTROL')
            if fl & 16: opts.append('ALT')
            out.append('    %d, %d, %s' % (key, cmd, ', '.join(opts)))
            if fl & 0x80: break
        out.append('END'); out.append('')
    elif t == 2:                                    # BITMAP
        out.append('%s BITMAP bitmap%s.bmp' % (idn(n), idn(n))); out.append('')
    elif t == 14:                                   # GROUP_ICON
        out.append('%s ICON icon%s.ico' % (idn(n), idn(n))); out.append('')
    elif t == 16:                                   # VERSIONINFO
        def node(o):
            ln, vln, typ = struct.unpack_from('<HHH', b, o)
            key, p = ustr(b, o + 6); p = (p + 3) & ~3
            return ln, vln, typ, key, p
        ln, vln, typ, key, p = node(0)
        f = struct.unpack_from('<13I', b, p)
        ver = lambda ms, ls: '%d,%d,%d,%d' % (ms >> 16, ms & 0xffff, ls >> 16, ls & 0xffff)
        out.append('%s VERSIONINFO' % idn(n))
        out.append('FILEVERSION     %s' % ver(f[2], f[3]))
        out.append('PRODUCTVERSION  %s' % ver(f[4], f[5]))
        out.append('FILEFLAGSMASK   0x%xL' % f[6])
        out.append('FILEFLAGS       0x%xL' % f[7])
        out.append('FILEOS          0x%xL' % f[8])
        out.append('FILETYPE        0x%xL' % f[9])
        out.append('FILESUBTYPE     0x%xL' % f[10])
        out.append('BEGIN')
        p = (p + vln + 3) & ~3
        while p < ln:
            cl, cvl, ct, ckey, cp = node(p)
            out.append('    BLOCK %s' % q(ckey)); out.append('    BEGIN')
            if ckey == 'StringFileInfo':
                tp = cp
                while tp < p + cl:
                    tl, tvl, tt, tkey, sp = node(tp)
                    out.append('        BLOCK %s' % q(tkey)); out.append('        BEGIN')
                    while sp < tp + tl:
                        sl, svl, st, skey, vp = node(sp)
                        val = b[vp:vp + 2 * svl].decode('utf-16le') if st == 1 else ''
                        out.append('            VALUE %s, %s' % (q(skey), q(val.rstrip('\0') + '\0' if val.endswith('\0') else val)))
                        sp = (sp + sl + 3) & ~3
                    out.append('        END')
                    tp = (tp + tl + 3) & ~3
            else:
                vp = cp
                while vp < p + cl:
                    vl, vvl, vt, vkey, dp = node(vp)
                    words = struct.unpack_from('<%dH' % (vvl // 2), b, dp)
                    out.append('        VALUE %s, %s' % (q(vkey), ', '.join('0x%x' % w for w in words)))
                    vp = (vp + vl + 3) & ~3
            out.append('    END')
            p = (p + cl + 3) & ~3
        out.append('END'); out.append('')
print('\n'.join(out))
