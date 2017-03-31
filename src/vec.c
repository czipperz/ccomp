#include "vec.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "position.h"

/*! \brief Generic vector implementation.  We use type erasure of the
 *  input and then casting to get all different vectors to look the
 *  same. */
struct vec {
    char* ptr;
    size_t len;
    size_t cap;
};

/*! \brief Reallocate the memory */
static int _realloc(struct vec* self, size_t size, size_t new_cap) {
    char* temp;
    assert(self);
    temp = realloc(self->ptr, size * new_cap);
    if (!temp) {
        return 1;
    }
    self->ptr = temp;
    self->cap = new_cap;
    return 0;
}

/*! \brief Reserve used for auto reserves. */
static int _reserve(struct vec* self, size_t size, size_t new_cap) {
    assert(self);
    if (new_cap > self->cap) {
        if (new_cap <= self->cap * 2) {
            new_cap = self->cap * 2;
        }
        return _realloc(self, size, new_cap);
    }
    return 0;
}

int vec_reserve(void* s, size_t size, size_t new_cap) {
    struct vec* self = s;
    assert(self);
    if (new_cap > self->cap) {
        if (new_cap <= self->cap * 2) {
            new_cap = self->cap * 2;
        }
        return _realloc(self, size, new_cap);
    }
    return 0;
}

int vec_insert(void* s, size_t size, size_t index, void* elem) {
    struct vec* self = s;
    assert(self);
    assert(elem);
    assert(index <= self->len);
    if (_reserve(self, self->len + 1, size)) {
        return 1;
    }
    memmove(size * (index + 1) + self->ptr, size * index + self->ptr,
            size * (self->len - index));
    ++self->len;
    memcpy(size * index + self->ptr, elem, size);
    return 0;
}

int vec_push(void* s, size_t size, void* elem) {
    struct vec* self = s;
    assert(self);
    return vec_insert(self, size, self->len, elem);
}

int vec_shrink_to_size(void* s, size_t size) {
    struct vec* self = s;
    assert(self);
    return _realloc(self, size, self->len);
}
