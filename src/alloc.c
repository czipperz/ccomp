/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "alloc.h"

int vector_reserve_another(void* *memory, int *len, int *max,
                           size_t size) {
    /* If not enough space, realloc. */
    if (*len == *max - 1) {
        void* temp = realloc(*memory, *max * 2 * size);
        if (!temp) { return 1; }
        *max *= 2;
        *memory = temp;
    }
    ++*len;
    return 0;
}
