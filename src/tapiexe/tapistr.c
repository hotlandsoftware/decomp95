/*
 * Window class name.  Kept in its own module so it is linked ahead of the
 * code in tapiexe.c and lands at offset 0 of the code segment.
 */

#include <windows.h>

char __based(__segname("_CODE")) szClassName[] = "TapiAppWinClass";
