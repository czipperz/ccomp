#include "str.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static int _realloc(str* s, size_t new_cap) {
    char* temp = realloc(s->str, new_cap + 1);
    if (temp) {
        s->str = temp;
        s->cap = new_cap;
        return 0;
    }
    return 1;
}

int str_reserve(str* s, size_t new_cap) {
    assert(s);
    if (new_cap > s->cap) {
        if (new_cap < s->cap * 2) {
            new_cap = s->cap * 2;
        }
        return _realloc(s, new_cap);
    }
    return 0;
}

int str_push(str* s, char elem) {
    assert(s);
    if (str_reserve(s, s->len + 1)) {
        return 1;
    }
    s->str[s->len] = elem;
    ++s->len;
    s->str[s->len] = 0;
    return 0;
}

int str_shrink_to_size(str* s) {
    assert(s);
    return _realloc(s, s->len);
}

void str_set_len(str* s, size_t new_len) {
    assert(s);
    assert(new_len < s->cap);
    assert(s->str);
    s->len = new_len;
    s->str[new_len] = 0;
}
