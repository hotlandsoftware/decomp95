# welcome.exe (Windows 95, 4.00.950) — work in progress

Target: `original/SYSTEM/welcome.exe` (16384 bytes). The Welcome screen: a tip of the day from the
registry, plus buttons for the Windows Tour, What's New, Online Registration and the Product Catalog.

## Status (2026-10-16)
`tools/build.sh welcome` builds a working WELCOME.EXE with the same size and section layout as the
original.

| Part | State |
|---|---|
| .text | 9/12 functions byte-identical; the other three differ by 31, 32 and 12 bytes |
| .data | identical |
| .rsrc | identical |
| .reloc | identical |
| .idata | same layout; the order of a few thunks follows the unmatched code |
| headers | identical apart from the checksum |

Not yet matching:
- **RunFromSetupSource:** stack-slot layout. In the original the drive-root buffer and the drive mask
  share a slot, and the frame has one extra 4-byte slot.
- **WelcomeCommand:** the `id` and `fOK` registers are swapped, and the slots of the per-case locals
  differ.
- **RunFromCD:** one register pair is swapped.

## Source layout and build
| File | Contents |
|---|---|
| `wlcmstr.asm` | read-only strings at the start of `.text` (4-byte aligned) |
| `welcome.c` | SwapWindows, InitApplication, ShowNextTip, RunFromSetupSource |
| `wlcmstr2.asm` | "SourcePathType", which sits between the two C modules |
| `wlcmdlg.c` | dialog, WM_COMMAND handling, command line, ModuleEntry and WinMain |
| `welcome.h` | shared declarations (including the undocumented SHELL32 ordinals) |
| `welcome.rc` | resources (DIALOGEX, bitmaps, icon, strings) |

Findings:
- The read-only strings are in the code section, but CL 9.00 can't produce them. It aligns
  code-section data to 16 and arrays of 8 or more bytes to 8, while the original packs them at
  4-byte alignment. They come from assembly string modules.
- `/Gf`: string literals become COMDATs, which LINK lays out in reverse order within a function.
  This reproduces the original `.data` exactly.
- `/MERGE:.bss=.data`: `g_hinst` sits at the end of `.data`, and the image has no `.bss` section.
- SHELL32 is imported by ordinal: PathBuildRoot 30, PathFindFileName 34, PathAppend 36, PathIsUNC 39,
  PathFileExists 45 and ShellMessageBoxA 183. ShellMessageBoxA is declared without `dllimport`, so
  it goes through a `jmp [IAT]` stub. `tools/ordimplib.py` builds the import library from the SDK
  library's descriptor members. The descriptor naming has to match the other SDK libraries, or
  LINK emits a second null descriptor.
- Post-link steps as for notepad: timestamps, `BIND -o` and the checksum.
- `nppad.h` is a copy of notepad's symbol-padding header. welcome.c uses offset 0; wlcmdlg.c uses
  offset 7 plus NPLOC18 in InitWelcomeDialog.
