/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_ALLOC_H
#define HEADER_GUARD_ALLOC_H

#include <stdlib.h>

/* BUFFER_SIZE is the maximum size of a string.  == 1KB */
#define BUFFER_SIZE (1 << 10)
/* BUFFER_LIST_SIZE is the size of a list of strings used for
 * read_file(). */
#define BUFFER_LIST_SIZE (32)

#define REALLOC(var, size, handle)                                   \
    do {                                                             \
        void* __temp = realloc((var), (size));                       \
        if (__temp) {                                                \
            (var) = __temp;                                          \
        } else {                                                     \
            handle;                                                  \
        }                                                            \
    } while (0)

/* /\* If there is not enough space in memory for another element, */
/*  * reallocate it via a doubling scheme. *\/ */
/* int vector_reserve_another(void* *memory, int *len, int *max, */
/*                            size_t size); */

#endif
