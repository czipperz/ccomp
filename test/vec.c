/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "test.h"
#include "vec.h"

#include <stdlib.h>

struct vec {
    int* ptr;
    size_t len;
    size_t cap;
};

TEST(_push) {
    struct vec v = VEC_INIT;
    int el;
    ASSERT(v.ptr == 0);
    ASSERT(v.len == 0);
    ASSERT(v.cap == 0);

    el = 1;
    if (vec_push(&v, sizeof(int), &el)) {
        ASSERT(v.ptr == 0);
        ASSERT(v.len == 0);
        ASSERT(v.cap == 0);
        return 0;
    }
    ASSERT(v.ptr);
    ASSERT(v.len == 1);
    ASSERT(v.cap >= 1);
    ASSERT(v.ptr[0] == 1);

    el = 2;
    if (vec_push(&v, sizeof(int), &el)) {
        ASSERT(v.ptr);
        ASSERT(v.len == 1);
        ASSERT(v.cap == 1);
        return 0;
    }
    ASSERT(v.ptr);
    ASSERT(v.len == 2);
    ASSERT(v.cap >= 2);
    ASSERT(v.ptr[0] == 1);
    ASSERT(v.ptr[1] == 2);

    free(v.ptr);
}
END_TEST

TEST(_reserve) {
    struct vec v = VEC_INIT;
    struct vec b;
    int el;
    if (vec_reserve(&v, sizeof(int), 3)) {
        return 0;
    }
    ASSERT(v.ptr);
    ASSERT(v.len == 0);
    ASSERT(v.cap == 3);
    b = v;

    el = 1;
    ASSERT(!vec_push(&v, sizeof(int), &el));
    el = 2000;
    ASSERT(!vec_push(&v, sizeof(int), &el));
    el = 12303;
    ASSERT(!vec_push(&v, sizeof(int), &el));

    ASSERT(v.ptr[0] == 1);
    ASSERT(v.ptr[1] == 2000);
    ASSERT(v.ptr[2] == 12303);

    ASSERT(v.ptr == b.ptr);
    ASSERT(v.len == 3);
    ASSERT(v.cap == 3);

    free(v.ptr);
}
END_TEST

static int (*tests[])() = {_push, _reserve};
RUN(test_vec)
