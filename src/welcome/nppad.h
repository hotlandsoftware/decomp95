/*
 * nppad.h - symbol-table padding.
 *
 * CL 9.00's register allocator and operand ordering break ties using the
 * compiler's internal symbol numbering, which depends (mod 32) on how many
 * symbols were declared before each function.  The original notepad was
 * built with Microsoft's internal Win95 headers, which we do not have; each
 * source file defines NPPAD (0-31) before including this file to stand in
 * for the difference in declared-symbol count.  The placeholders are never
 * referenced and emit nothing into the object file.
 *
 * The same numbering also advances with each local variable, so a function
 * can begin with NPLOCn (n unused locals) to stand in for locals the original
 * declared but that left no trace in the code (e.g. debug-only variables).
 */
#ifndef NPPAD
#define NPPAD 0
#endif

#if NPPAD & 1
extern int npPad1_0;
#endif

#if NPPAD & 2
extern int npPad2_0;
extern int npPad2_1;
#endif

#if NPPAD & 4
extern int npPad4_0;
extern int npPad4_1;
extern int npPad4_2;
extern int npPad4_3;
#endif

#if NPPAD & 8
extern int npPad8_0;
extern int npPad8_1;
extern int npPad8_2;
extern int npPad8_3;
extern int npPad8_4;
extern int npPad8_5;
extern int npPad8_6;
extern int npPad8_7;
#endif

#if NPPAD & 16
extern int npPad16_0;
extern int npPad16_1;
extern int npPad16_2;
extern int npPad16_3;
extern int npPad16_4;
extern int npPad16_5;
extern int npPad16_6;
extern int npPad16_7;
extern int npPad16_8;
extern int npPad16_9;
extern int npPad16_10;
extern int npPad16_11;
extern int npPad16_12;
extern int npPad16_13;
extern int npPad16_14;
extern int npPad16_15;
#endif

#pragma warning(disable: 4101)   /* unreferenced local variable */
#define NPLOC0
#define NPLOC1 int npLoc0;
#define NPLOC2 int npLoc0; int npLoc1;
#define NPLOC3 int npLoc0; int npLoc1; int npLoc2;
#define NPLOC4 int npLoc0; int npLoc1; int npLoc2; int npLoc3;
#define NPLOC5 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4;
#define NPLOC6 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5;
#define NPLOC7 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6;
#define NPLOC8 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7;
#define NPLOC9 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8;
#define NPLOC10 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9;
#define NPLOC11 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10;
#define NPLOC12 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11;
#define NPLOC13 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12;
#define NPLOC14 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13;
#define NPLOC15 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14;
#define NPLOC16 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15;
#define NPLOC17 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16;
#define NPLOC18 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17;
#define NPLOC19 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18;
#define NPLOC20 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19;
#define NPLOC21 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20;
#define NPLOC22 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21;
#define NPLOC23 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22;
#define NPLOC24 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23;
#define NPLOC25 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24;
#define NPLOC26 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25;
#define NPLOC27 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25; int npLoc26;
#define NPLOC28 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25; int npLoc26; int npLoc27;
#define NPLOC29 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25; int npLoc26; int npLoc27; int npLoc28;
#define NPLOC30 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25; int npLoc26; int npLoc27; int npLoc28; int npLoc29;
#define NPLOC31 int npLoc0; int npLoc1; int npLoc2; int npLoc3; int npLoc4; int npLoc5; int npLoc6; int npLoc7; int npLoc8; int npLoc9; int npLoc10; int npLoc11; int npLoc12; int npLoc13; int npLoc14; int npLoc15; int npLoc16; int npLoc17; int npLoc18; int npLoc19; int npLoc20; int npLoc21; int npLoc22; int npLoc23; int npLoc24; int npLoc25; int npLoc26; int npLoc27; int npLoc28; int npLoc29; int npLoc30;
