# Decomp95
The goal of this project is to study and create a byte-matched decompilation of the Windows 95 operating system: C and assembly sources that rebuild each original binary byte for byte. This is based primarily on Windows 95 Build 950.6 (``en-US_x86chk_950_win95.rtm_chicago.iso``). Some debug symbols are from Build 490 (``4.00.490_x86chk_client_en-us-WIN95_DEBUG.iso``).

# Requirements
Toolchains are not committed. Place them as follows:
- `msvc/`: Microsoft Visual C++ 1.52 (`BIN`, `INCLUDE`, `LIB`), for 16-bit (NE) modules. CL 8.00c, LINK 5.60.339
  and IMPLIB are 32-bit console tools and run under Wine.
- `msvc20/`: Microsoft Visual C++ 2.0 (CL 9.00, LINK 2.50), for 32-bit (PE) modules. `msvc21/` and `msvc22/`
  (CL 9.10) are optional; they produce the same code.
- `win32sdk/`: Win32 SDK `MSTOOLS` (headers and import libraries, plus `BIND.EXE` and `IMAGEHLP.DLL` used
  to bind PE imports).
- `rc4/` and `rc4/BINW16/`: Resource Compiler 4.00 from the Win32 SDK (July 1995). The 32-bit `RC.EXE` runs under
  Wine. The 16-bit one is a DOS program and runs in DOSBox.
- Host packages: `wine`, `dosbox`, `uasm`, `python3`.

All Wine use goes through `tools/wineenv.sh`: a private, headless prefix in `build/wineprefix` with no display
drivers. Never run the toolchains in your default Wine prefix. MSVC there can freeze the desktop.

# Building
`tools/build.sh [module...]` builds each module (default: all) into `build/<module>/` from a clean state and
compares it with `original/`. Each module has a `src/<module>/build.sh` that uses the helpers in `tools/buildlib.sh`.
Artwork (icons etc.) is extracted from `original/` at build time and is never committed.

Post-link steps that the original build also performed (timestamps, `BIND`, PE checksum) are reproduced by
`tools/pestamp.py`, `BIND.EXE -o` and `tools/pechecksum.py`.

# Layout
- `original/`: original Windows 95 files, laid out like the Windows directory (not committed)
- `src/crt/`: shared 16-bit startup code (`crt0w.asm`, medium-model no-CRT Windows startup)
- `src/<module>/`: decompiled sources, `.def`/`.rc` files and `build.sh` per module; some modules have a
  `README.md` with notes and status
- `tools/build.sh`, `tools/buildlib.sh`: build driver and helpers
- `tools/wineenv.sh`: headless Wine environment
- `tools/dosrun.sh`: runs DOS commands in headless DOSBox

## 16-bit (NE) tools
- `tools/cc.sh`: compiles one C file with MSVC 1.52 (writes a `.COD` listing)
- `tools/codcmp.py`: compares a `.COD` listing with the original, instruction by instruction
- `tools/necmp.py`: NE executable comparison
- `tools/neinfo.py`: NE header/segment/relocation dump
- `tools/neres.py`: NE resource listing/dump
- `tools/extract_ico.py`: rebuilds an .ico from an NE icon group
- `tools/omfdump.py`: OMF object dump
- `tools/symdump.py`: MAPSYM .SYM dump

## 32-bit (PE) tools
- `tools/cc32.sh`: compiles one C/C++ file with VC++ 2.0 (writes `.cod` and `.obj`)
- `tools/coffcmp.py`: compares a COFF `.obj` with the original, function by function (relocations masked)
- `tools/fdiff.py`: side-by-side instruction diff of one function (object vs. original)
- `tools/variants.py`: compiles source variants of one function and reports which match;
  `PADS=a-b`/`LOCS=a-b` also try symbol-padding settings (see `src/notepad/README.md`)
- `tools/padscan.py`: per-function results at every symbol-padding offset
- `tools/locopt.py`: greedy per-function padding (`NPLOCn`) tuning in file order
- `tools/pedis.py`: annotated disassembly of the original using a module's `symbols.txt`/`data.txt`
- `tools/pecodcmp.py`: compares a `.cod` listing with the original PE
- `tools/pecmp.py`: whole-image PE comparison (headers and per-section counts)
- `tools/peinfo.py`: PE dump (sections, imports with bound values, relocations)
- `tools/peres.py`: PE resource listing/string dump
- `tools/res2rc.py`: decompiles PE resources into an `.rc` script
- `tools/peicons.py`: rebuilds .ico files from PE icon groups
- `tools/libdump.py`: COFF archive (.lib) member dump
- `tools/patchhints.py`: copies an SDK import library with its hints patched to match the original
- `tools/pestamp.py`: sets the link/resource timestamps
- `tools/pechecksum.py`: sets the PE checksum
- `tools/ghidra/`: headless Ghidra decompile script

# Status
| Module | Description | Format | Decompiled | Matching |
|---|---|---|---|---|
| tapiexe.exe | Windows Telephony DLL hidden app | NE (16-bit) | 100% | 100% |
| winver.exe | Windows Version program | NE (16-bit) | 100% | 100% |
| msmixmgr.dll | Audio Mixer Manager (forwards to MMSYSTEM) | NE (16-bit) | 100% | 100% |
| rsrc16.dll | Resource Meter 16-bit thunks | NE (16-bit) | 100% | 100% |
| notepad.exe | Notepad | PE (32-bit) | 100% | 65% |

*Decompiled*: share of the module's code that has C/assembly source and builds into a runnable binary.
*Matching*: share of code bytes in functions that compile byte-identically to the original (100% means the
whole file is byte-identical).

notepad.exe has 48 of its 57 functions matching. It builds to the same size and section layout as the original. `.data`, `.bss` and `.rsrc` are
identical, and import descriptors, hints and bound values are too. The remaining differences are in `.text`
(9 functions not yet matching), the import order and relocations that follow from it, and the header fields that depend on them.
See `src/notepad/README.md` for details.
