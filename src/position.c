/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "position.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "alloc.h"

void destroy_vec_tagged_str(vec_tagged_str* self) {
    size_t i;
    assert(self);
    assert(self->ptr || self->len == 0);
    for (i = 0; i != self->len; ++i) {
        free(self->ptr[i].str.str);
    }
    free(self->ptr);
}

void destroy_vec_vec_tagged_str(vec_vec_tagged_str* self) {
    size_t i;
    assert(self);
    assert(self->ptr || self->len == 0);
    for (i = 0; i != self->len; ++i) {
        destroy_vec_tagged_str(self->ptr + i);
    }
    free(self->ptr);
}

void rpos_bounds_check(const raw_position* rpos) {
    assert(rpos);
    assert(rpos->list);
    assert(rpos->buffer < rpos->list->len);
    assert(rpos->index < rpos->list->ptr[rpos->buffer].str.len);
}
void rpos_bounds_check_end(const raw_position* rpos, vec_tagged_str* end) {
    rpos_bounds_check(rpos);
    assert(end);
    assert(rpos->list <= end);
}

tagged_str* rpos_str(const raw_position* rpos) {
    rpos_bounds_check(rpos);
    return &rpos->list->ptr[rpos->buffer];
}
char rpos_c(const raw_position* rpos) {
    rpos_bounds_check(rpos);
    return rpos_str(rpos)->str.str[rpos->index];
}

static int
stupid_forward_char(file_position* fpos, raw_position* rpos,
                    vec_tagged_str* end) {
    /* Handle end of file gracefully */
    if (rpos->list == end) {
        return 0;
    }

    /* Walk file position forward */
    if (rpos_c(rpos) == '\n') {
        ++fpos->y;
        fpos->x = 0;
    } else {
        ++fpos->x;
    }

    /* Goto next character */
    ++rpos->index;
    if (rpos->index == rpos_str(rpos)->str.len) {
        ++rpos->buffer;
        rpos->index = 0;
        if (rpos->buffer == rpos->list->len) {
            ++rpos->list;
            rpos->buffer = 0;
            if (rpos->list == end) {
                return 1;
            }
        }
        /* There was a break in the source code at this point.  Reset
         * the fpos so that error messages print correctly. */
        *fpos = rpos_str(rpos)->fpos;
    }
    return 0;
}

/*! \brief Walk forward a logical character.
 *
 * For example, if point is before "a\\\nb", move point to before
 * "b". */
int forward_char(file_position* fpos, raw_position* rpos,
                 vec_tagged_str* end) {
    assert(fpos);
    rpos_bounds_check_end(rpos, end);

    if (stupid_forward_char(fpos, rpos, end)) {
        return 1;
    }

    if (rpos_c(rpos) == '\\') {
        file_position bfpos = *fpos;
        raw_position brpos = *rpos;
        if (stupid_forward_char(&bfpos, &brpos, end)) {
            return 0;
        }
        if (rpos_c(&brpos) == '\n') {
            /* This time we might recurse, to handle multiple
             * "\\\n\\\n".  We would want to go after both. */
            int eof = forward_char(&bfpos, &brpos, end);
            *fpos = bfpos;
            *rpos = brpos;
            return eof;
        }
    }
    return 0;
}

int forward_through_blanks(file_position *fpos,
                           raw_position *rpos,
                           vec_tagged_str* end) {
    assert(fpos);
    rpos_bounds_check_end(rpos, end);

    while (rpos_c(rpos) == ' ' ||
           rpos_c(rpos) == '\t') {
        if (forward_char(fpos, rpos, end)) {
            return 1;
        }

        assert(rpos->index < rpos_str(rpos)->str.len);
    }
    return 0;
}

int forward_through_whitespace(file_position *fpos,
                               raw_position *rpos,
                               vec_tagged_str* end) {
    assert(fpos);
    rpos_bounds_check_end(rpos, end);

    while (isspace(rpos_c(rpos))) {
        if (forward_char(fpos, rpos, end)) {
            return 1;
        }
    }
    return 0;
}

void walk_fpos(file_position* fpos, const char* begin, size_t len) {
    const char* iterator;
    const char* loc;
    assert(fpos);
    assert(begin);
    for (iterator = begin;
         (loc =
              memchr(iterator, '\n', len - (iterator - begin)));
         ++iterator) {
        iterator = loc;
        ++fpos->y;
        fpos->x = 0;
    }
    fpos->x += len - (iterator - begin);
}

#if TEST_MODE
#include "test.h"

TEST(_forward_char_1) {
    file_position fpos = {"", 0, 0};
    raw_position rpos = VEC_INIT;
    vec_vec_tagged_str vvstr = VEC_INIT;
    vvstr_of(&vvstr, "HI");
}
END_TEST

TEST(_walk_fpos_1) {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi", 2);
    ASSERT(fpos.y == 0);
    ASSERT(fpos.x == 2);
}
END_TEST

TEST(_walk_fpos_2) {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi\nbye", 6);
    ASSERT(fpos.y == 1);
    ASSERT(fpos.x == 3);
}
END_TEST

TEST(_walk_fpos_3) {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hihihi\n\n\n\nbye\n", 14);
    ASSERT(fpos.y == 5);
    ASSERT(fpos.x == 0);
}
END_TEST

void test_position() {
    RUN(_forward_char_1);
    RUN(_walk_fpos_1);
    RUN(_walk_fpos_2);
    RUN(_walk_fpos_3);
}
#endif
