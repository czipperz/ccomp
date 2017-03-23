/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_STR_H
#define HEADER_GUARD_STR_H

#include <stdlib.h>
#include <stdio.h>

/* A generic raw string and length pair. */
typedef struct str {
    /* May be not null terminated: */ char* str;
    int len;
} str;

static void fputstr(str *string, FILE* file) {
    int i;
    for (i = 0; i != string->len; ++i) {
        putc(string->str[i], file);
    }
}

#endif
