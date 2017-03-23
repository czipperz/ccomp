/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "diagnostics.h"

#include <stdio.h>
#include <stdarg.h>

#include "terminal.h"

static void print_line_number(const file_position* position) {
    fprintf(stderr, "%s:%d:%d: ", position->file_name,
            position->y + 1, position->x + 1);
}

void print_error(const file_position* position,
                 const char* format, ...) {
    va_list list;

    terminal_stderr_set_foreground(terminal_red);
    terminal_stderr_set_bold();

    print_line_number(position);
    fputs("Error: ", stderr);

    va_start(list, format);
    vfprintf(stderr, format, list);
    va_end(list);
    putc('\n', stderr);

    terminal_stderr_reset();
}

void print_fatal(const file_position* position,
                 const char* format, ...) {
    va_list list;

    terminal_stderr_set_foreground(terminal_red);
    terminal_stderr_set_bold();

    print_line_number(position);
    fputs("Fatal Error: ", stderr);

    va_start(list, format);
    vfprintf(stderr, format, list);
    va_end(list);
    putc('\n', stderr);

    terminal_stderr_reset();
}

void print_warning(const file_position* position,
                   const char* format, ...) {
    va_list list;

    terminal_stderr_set_foreground(terminal_blue);
    terminal_stderr_set_bold();

    print_line_number(position);
    fputs("Warning: ", stderr);

    va_start(list, format);
    vfprintf(stderr, format, list);
    va_end(list);
    putc('\n', stderr);

    terminal_stderr_reset();
}
