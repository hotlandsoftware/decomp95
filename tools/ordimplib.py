#!/usr/bin/env python3
"""Build an import library for functions imported by ordinal only.

Usage: ordimplib.py <sdk.lib> <out.lib> <name>@<ordinal>[:data] ...
  <name> is the decorated C name without the leading underscore, e.g.
  PathIsUNC@4 or ShellMessageBoxA (cdecl).

The library keeps the SDK library's import-descriptor members (so the
descriptor and null-thunk symbols are named the way LINK 2.50 expects from
the other SDK libraries) and adds one long-format import member per function,
laid out like the SDK's own members: a 'jmp [__imp_x]' thunk in .text and the
ordinal (0x80000000 | n) in .idata$5 / .idata$4, without a hint/name entry."""
import sys, struct

src, dst = sys.argv[1:3]
funcs = []
for a in sys.argv[3:]:
    name, _, ordn = a.rpartition('@') if a.count('@') == 2 or ':' in a else (None, None, None)
    # name may itself contain '@N'; the ordinal is the last '@'
    i = a.rindex('@'); name, ordn = a[:i], int(a[i+1:])
    funcs.append((name, ordn))

d = open(src, 'rb').read()
assert d[:8] == b'!<arch>\n'

# --- public symbols of every member (for the linker members)
def publics(body):
    symptr, nsym = struct.unpack_from('<II', body, 8)
    strtab = symptr + 18 * nsym; out = []; i = 0
    while i < nsym:
        q = symptr + 18 * i
        val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', body, q + 8)
        if cls == 2 and sec != 0:
            if body[q:q+4] == b'\0\0\0\0':
                so = strtab + struct.unpack_from('<I', body, q + 4)[0]
                n = body[so:body.index(b'\0', so)]
            else:
                n = body[q:q+8].rstrip(b'\0')
            out.append(n.decode())
        i += 1 + naux
    return out


# --- keep the descriptor members of the SDK library
members = []                       # (name field, body)
p = 8; dllname = None
while p < len(d):
    hdr = d[p:p+60]; size = int(hdr[48:58]); body = d[p+60:p+60+size]
    p += 60 + size + (size & 1)
    nm = hdr[:16].decode('latin-1').rstrip()
    if nm in ('/', '//'): continue
    if body[:2] != b'\x4c\x01': continue
    ns = struct.unpack_from('<H', body, 2)[0]; ohsz = struct.unpack_from('<H', body, 16)[0]
    secnames = [body[20+ohsz+40*s:20+ohsz+40*s+8].rstrip(b'\0') for s in range(ns)]
    if b'.text' in secnames: continue                      # a function member
    pub = publics(body)
    if not any(s.startswith('__IMPORT_DESCRIPTOR_') or s == '__NULL_IMPORT_DESCRIPTOR' or
               s.endswith('_NULL_THUNK_DATA') for s in pub):
        continue                                           # data imports etc.
    members.append((hdr[:16], body))
    dllname = nm.rstrip('/')
ts = struct.unpack_from('<I', members[0][1], 4)[0]
desc = '__IMPORT_DESCRIPTOR_' + dllname.rsplit('.', 1)[0]

def coff_member(name, ordn):
    """Long-format import member for _name importing ordinal ordn."""
    cname = '_' + name; iname = '__imp_' + cname
    secs = [(b'.text', bytes.fromhex('ff2500000000'), 0x60201020, [(2, 4, 6)]),
            (b'.idata$5', struct.pack('<I', 0x80000000 | ordn), 0xc0301040, []),
            (b'.idata$4', struct.pack('<I', 0x80000000 | ordn), 0xc0301040, [])]
    # symbols: 0 .text(aux) 2 _name 3 .idata$5(aux) 5 __imp_name 6 .idata$4(aux) 8 descriptor
    strtab = b''
    def sym(n, val, sec, typ, cls, naux):
        nonlocal strtab
        nb = n.encode()
        if len(nb) <= 8: field = nb.ljust(8, b'\0')
        else:
            field = struct.pack('<II', 0, 4 + len(strtab)); strtab += nb + b'\0'
        return field + struct.pack('<IhHBB', val, sec, typ, cls, naux)
    def secaux(length, nrel, sel=0, num=0):
        return struct.pack('<IHHIHB3x', length, nrel, 0, 0, num, sel)
    syms = b''
    syms += sym('.text', 0, 1, 0, 3, 1) + secaux(6, 1, 1, 0)        # COMDAT, no duplicates
    syms += sym(cname, 0, 1, 0x20, 2, 0)
    syms += sym('.idata$5', 0, 2, 0, 3, 1) + secaux(4, 0, 1, 0)     # COMDAT, no duplicates
    syms += sym(iname, 0, 2, 0, 2, 0)
    syms += sym('.idata$4', 0, 3, 0, 3, 1) + secaux(4, 0, 5, 2)     # associative with .idata$5
    syms += sym(desc, 0, 0, 0, 2, 0)
    nsym = len(syms) // 18
    # layout: file header, section headers, raw data + relocs, symbols, strings
    hdrsz = 20 + 40 * len(secs)
    raw = b''; shdrs = b''; off = hdrsz
    for nm, data, ch, rel in secs:
        rawoff = off; off += len(data)
        reloff = off if rel else 0; off += 10 * len(rel)
        shdrs += nm.ljust(8, b'\0') + struct.pack('<IIIIIIHHI', 0, 0, len(data), rawoff, reloff, 0, len(rel), 0, ch)
        raw += data
        for ro, si, rt in rel:
            # thunk relocation points at the __imp_ symbol (index 5)
            raw += struct.pack('<IIH', ro - 0 if False else 2, 5, rt)
    symptr = off
    fh = struct.pack('<HHIIIHH', 0x14c, len(secs), ts, symptr, nsym, 0, 0x100)
    body = fh + shdrs + raw + syms + struct.pack('<I', 4 + len(strtab)) + strtab
    return body, [cname, iname]

newmembers = []
for name, ordn in funcs:
    body, pubs = coff_member(name, ordn)
    newmembers.append(((dllname + '/').encode().ljust(16), body, pubs))

allm = [(h, b, publics(b)) for h, b in members] + newmembers

def mhdr(name, size):
    return name.ljust(16) + b'%-12d%-6s%-6s%-8s%-10d`\n' % (ts, b'', b'', b'0', size)

# offsets: first linker member, second linker member, then members
entries = [(s, i) for i, (_, _, pubs) in enumerate(allm) for s in pubs]
def lm1(offs):
    names = b''.join(s.encode() + b'\0' for s, i in entries)
    return struct.pack('>I', len(entries)) + b''.join(struct.pack('>I', offs[i]) for s, i in entries) + names
def lm2(offs):
    srt = sorted(entries, key=lambda e: e[0].encode())
    return (struct.pack('<I', len(allm)) + b''.join(struct.pack('<I', o) for o in offs) +
            struct.pack('<I', len(srt)) + b''.join(struct.pack('<H', i + 1) for s, i in srt) +
            b''.join(s.encode() + b'\0' for s, i in srt))
def build(offs):
    out = b'!<arch>\n'
    for body in (lm1(offs), lm2(offs)):
        out += mhdr(b'/', len(body)) + body + (b'\n' if len(body) & 1 else b'')
    real = []
    for h, b, _ in allm:
        real.append(len(out))
        out += mhdr(h.rstrip(b' '), len(b)) + b + (b'\n' if len(b) & 1 else b'')
    return out, real
offs = [0] * len(allm)
for _ in range(3):
    out, offs2 = build(offs)
    if offs2 == offs: break
    offs = offs2
out, _ = build(offs)
open(dst, 'wb').write(out)
print('%s: %d descriptor members + %d ordinal imports for %s' % (dst, len(members), len(funcs), dllname))
