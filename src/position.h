/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_POSITION_H
#define HEADER_GUARD_POSITION_H

#include <stddef.h>

#include "str.h"
#include "vec.h"

/*! \brief The position in the file, note that y and x are always off by 1. */
typedef struct file_position {
    /* NOT owning, NOT null: */ const char* file_name;
    /* Add one to get the value to print */ size_t y, x;
} file_position;

/*! \brief A tagged_str is a str that includes a fpos for diagnosage purposes. */
typedef struct tagged_str {
    str str;
    file_position fpos;
} tagged_str;

#define TAGGED_STR_INIT {VEC_INIT, {0, 0, 0}}

typedef struct vec_tagged_str {
    struct tagged_str* ptr;
    size_t len;
    size_t cap;
} vec_tagged_str;

typedef struct vec_vec_tagged_str {
    vec_tagged_str* ptr;
    size_t len;
    size_t cap;
} vec_vec_tagged_str;

void destroy_vec_tagged_str(vec_tagged_str* self);
void destroy_vec_vec_tagged_str(vec_vec_tagged_str* self);

/*! \brief Used during preprocessing and tokenizing phase as an iterator
 * through the file. */
typedef struct raw_position {
    /* NOT owning: */ vec_tagged_str* list;
    size_t buffer;
    size_t index;
} raw_position;

char rpos_c(const raw_position* rpos);
tagged_str* rpos_str(const raw_position* rpos);
void rpos_bounds_check(const raw_position* rpos);
void rpos_bounds_check_end(const raw_position* rpos, vec_tagged_str* end);

/* Go to the next character in the file.
 * Correctly handles cases where we are at the end of an embedded
 * array.
 * Return if the cursor is at EOF (strings_end). */
int forward_char(file_position *fpos, raw_position *rpos,
                 vec_tagged_str* strings_end);

/* Run forward_char() until all space/tab characters are exhausted.
 * Return if the cursor is at EOF (strings_end). */
int forward_through_blanks(file_position *fpos,
                           raw_position *rpos,
                           vec_tagged_str* strings_end);

/* Run forward_char() until all whitespace characters are exhausted.
 * Return if the cursor is at EOF (strings_end). */
int forward_through_whitespace(file_position *fpos,
                               raw_position *rpos,
                               vec_tagged_str* strings_end);

void walk_fpos(file_position* fpos, const char* begin, size_t len);

#endif
