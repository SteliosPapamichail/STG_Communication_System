//
// Created by parallels on 5/19/24.
//

#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <stdarg.h>
#include <stdio.h>

extern int debug_mode_enabled;

// Debug printf function
void debug_printf(const char *format, ...);
#endif //DEBUG_UTILS_H
