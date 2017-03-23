/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_POSITION_H
#define HEADER_GUARD_POSITION_H

#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

/* The position in the file, note that y and x are always off by 1. */
typedef struct file_position {
    /* NOT owning, NOT null: */ const char* file_name;
    /* Add one to get the value to print */ int y, x;
} file_position;

/* A tagged_str is a str that includes a fpos for debugging purposes. */
typedef struct tagged_str {
    /* Not null terminated, NULL only if len == 0: */ char* str;
    int len;
    file_position fpos;
} tagged_str;

void shrink_tagged_str(tagged_str*);

typedef struct tagged_str_list {
    /* NULL only if len == 0: */ tagged_str* strs;
    int len;
    int cap;
} tagged_str_list;

void destroy_tagged_str_list(tagged_str_list* list);

/* Used during preprocessing and tokenizing phase as an iterator
 * through the file. */
typedef struct raw_position {
    /* NOT owning: */ struct tagged_str_list* list;
    /* NOT owning: */ struct tagged_str* buffer;
    int index;
} raw_position;

/* Go to the next character in the file.
 * Correctly handles cases where we are at the end of an embedded
 * array.
 * Return if the cursor is at EOF (strings_end). */
int forward_char(file_position *fpos, raw_position *rpos,
                 tagged_str_list* strings_end);

/* Run forward_char() until all space/tab characters are exhausted.
 * Return if the cursor is at EOF (strings_end). */
int forward_through_blanks(file_position *fpos,
                           raw_position *rpos,
                           tagged_str_list* strings_end);

/* Run forward_char() until all whitespace characters are exhausted.
 * Return if the cursor is at EOF (strings_end). */
int forward_through_whitespace(file_position *fpos,
                               raw_position *rpos,
                               tagged_str_list* strings_end);

void walk_fpos(file_position* fpos, const char* begin, int len);

#endif
