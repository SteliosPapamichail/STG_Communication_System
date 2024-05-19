//
// Created by parallels on 5/19/24.
//

#include "debug_utils.h"

int debug_mode_enabled = 0;

// Debug printf function
void debug_printf(const char *format, ...) {
    if (debug_mode_enabled) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}