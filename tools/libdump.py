#!/usr/bin/env python3
"""Dump members of a COFF archive (.lib): names, sections, symbols, relocs.
Usage: libdump.py <file.lib> [member-substring]"""
import sys, struct
d = open(sys.argv[1], 'rb').read(); want = sys.argv[2] if len(sys.argv) > 2 else None
p = 8; longnames = b''; idx = 0
while p < len(d):
    hdr = d[p:p+60]; name = hdr[:16].decode('latin-1').rstrip(); size = int(hdr[48:58])
    body = d[p+60:p+60+size]; off = p
    p += 60 + size + (size & 1); idx += 1
    if name == '//': longnames = body; continue
    if name.startswith('/') and name[1:].strip().isdigit():
        o = int(name[1:]); name = longnames[o:longnames.index(b'\0', o) if b'\0' in longnames[o:] else longnames.index(b'\n', o)].decode()
    if name == '/': print('[linker member %d bytes]' % size); continue
    if body[:4] == b'\0\0\xff\xff':
        print('member %d %s: SHORT import' % (idx, name)); continue
    mach, ns, ts, symptr, nsym, ohsz, ch = struct.unpack_from('<HHIIIHH', body, 0)
    strtab = symptr + 18*nsym
    def sn(q):
        if body[q:q+4] == b'\0\0\0\0':
            so = strtab + struct.unpack_from('<I', body, q+4)[0]; return body[so:body.index(b'\0', so)].decode()
        return body[q:q+8].rstrip(b'\0').decode('latin-1')
    syms = []; i = 0
    while i < nsym:
        q = symptr + 18*i; val, sec, typ, cls, naux = struct.unpack_from('<IhHBB', body, q+8)
        syms.append((i, sn(q), val, sec, typ, cls, naux, body[q+18:q+18+18*naux])); i += 1 + naux
    if want and not any(want in s[1] for s in syms): continue
    print('member %d at %#x %s: mach %#x nsec %d ts %#x nsym %d chars %#x' % (idx, off, name, mach, ns, ts, nsym, ch))
    for s in range(ns):
        q = 20 + ohsz + 40*s
        sname = body[q:q+8].rstrip(b'\0').decode()
        vs, va, rs, rp, relp, lnp, nrel, nln, sch = struct.unpack_from('<IIIIIIHHI', body, q+8)
        print('   sec %d %-8s size %3d chars %08x relocs %d data %s' % (s+1, sname, rs, sch, nrel, body[rp:rp+rs].hex()))
        for r in range(nrel):
            ro, rs_, rt = struct.unpack_from('<IIH', body, relp+10*r)
            print('        reloc off %d sym %d type %d' % (ro, rs_, rt))
    for i, n, val, sec, typ, cls, naux, aux in syms:
        print('   sym %2d %-36s val %d sec %d typ %#x cls %d aux %s' % (i, n, val, sec, typ, cls, aux.hex()))
