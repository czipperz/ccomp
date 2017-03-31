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

static void fputsize_t(size_t s, FILE* f) {
    if (s == 0) {
        fputc('0', f);
    } else {
        size_t x = s / 10;
        fputsize_t(x, f);
        switch (s - x * 10) {
        case 0:
            fputc('0', f);
            break;
        case 1:
            fputc('1', f);
            break;
        case 2:
            fputc('2', f);
            break;
        case 3:
            fputc('3', f);
            break;
        case 4:
            fputc('4', f);
            break;
        case 5:
            fputc('5', f);
            break;
        case 6:
            fputc('6', f);
            break;
        case 7:
            fputc('7', f);
            break;
        case 8:
            fputc('8', f);
            break;
        case 9:
            fputc('9', f);
            break;
        }
    }
}

static void print_line_number(const file_position* position) {
    fputs(position->file_name, stderr);
    fputc(':', stderr);
    fputsize_t(position->y + 1, stderr);
    fputc(':', stderr);
    fputsize_t(position->x + 1, stderr);
    fputs(": ", stderr);
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

void print_malloc_error(const char* fname, size_t line) {
    terminal_stderr_set_foreground(terminal_red);
    terminal_stderr_set_bold();

    fputs("Fatal Error: Unable to allocate memory in ", stderr);
    fputs(fname, stderr);
    fputc(':', stderr);
    fputsize_t(line, stderr);
    fputc('\n', stderr);

    terminal_stderr_reset();
}
