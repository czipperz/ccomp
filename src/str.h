/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_STR_H
#define HEADER_GUARD_STR_H

#include <stddef.h>

#include "vec.h"

/*! \brief A basic string implementation.
 *
 * It is compliant with the `vec` concept. */
typedef struct str {
    /* NOT null terminated: */ char* str;
    size_t len;
    size_t cap;
} str;

/*! \brief Non binding request to increase the capacity.
 *
 * Returns 1 on memory allocation error. */
int str_reserve(str* self, size_t new_cap);

/*! \brief Reserve an extra element, then insert `str` at the end. */
int str_push(str* self, char elem);

/*! \brief Non binding request to shrink to size. */
int str_shrink_to_size(str* self);

/*! Set len to new_len and assign */
void str_set_len(str* s, size_t new_len);

#endif
