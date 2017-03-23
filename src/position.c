#include "position.h"

#include <string.h>

#include "alloc.h"

void shrink_tagged_str(tagged_str* str) {
    REALLOC(str->str, str->len, {});
}

void destroy_tagged_str_list(tagged_str_list* list) {
    assert(list);

    /* Delete each element by walking the list backwards */
    while (list->len != 0) {
        --list->len;
        free(list->strs[list->len].str);
    }

    free(list->strs);
}

int forward_char(file_position *fpos, raw_position *rpos,
                 tagged_str_list* strings_end) {
    assert(fpos);
    assert(rpos);
    assert(strings_end);

    /* Handle end of file gracefully */
    if (rpos->list == strings_end) {
        return 0;
    }

    assert(rpos->index >= 0 && rpos->index < rpos->buffer->len);

    /* Goto next character */
    if (rpos->buffer->str[rpos->index] == '\n') {
        ++fpos->y;
        fpos->x = 0;
    } else {
        ++fpos->x;
    }

    ++rpos->index;
    if (rpos->index == rpos->buffer->len) {
        ++rpos->buffer;
        rpos->index = 0;
        if (rpos->buffer == rpos->list->strs + rpos->list->len) {
            ++rpos->list;
            rpos->buffer = rpos->list->strs;
            if (rpos->list == strings_end) {
                return 1;
            }
        }
        /* There was a break in the source code at this point.  Reset
         * the fpos so that error messages print correctly. */
        *fpos = rpos->buffer->fpos;
    }
    return 0;
}

int forward_through_blanks(file_position *fpos,
                           raw_position *rpos,
                           tagged_str_list* strings_end) {
    assert(fpos);
    assert(rpos);
    assert(strings_end);
    assert(rpos->index < rpos->buffer->len);

    while (rpos->buffer->str[rpos->index] == ' ' ||
           rpos->buffer->str[rpos->index] == '\t') {
        if (forward_char(fpos, rpos, strings_end)) {
            return 1;
        }

        assert(rpos->index < rpos->buffer->len);
    }
    return 0;
}

int forward_through_whitespace(file_position *fpos,
                               raw_position *rpos,
                               tagged_str_list* strings_end) {
    assert(fpos);
    assert(rpos);
    assert(strings_end);
    assert(rpos->index < rpos->buffer->len);

    while (isspace(rpos->buffer->str[rpos->index])) {
        if (forward_char(fpos, rpos, strings_end)) {
            return 1;
        }

        assert(rpos->index < rpos->buffer->len);
    }
    return 0;
}

void walk_fpos(file_position* fpos, const char* begin, int len) {
    const char* iterator;
    for (iterator = begin;
         (iterator =
              memchr(iterator, '\n', len - (iterator - begin)));
         ++iterator) {
        ++fpos->y;
        fpos->x = 0;
    }
    fpos->x += len - (iterator - begin);
}
