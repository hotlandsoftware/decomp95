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
