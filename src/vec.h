#ifndef HEADER_GUARD_VEC_H
#define HEADER_GUARD_VEC_H

#include <stddef.h>

struct tagged_str;

/*! \brief Non binding request to increase the capacity.
 *
 * Returns 1 on memory allocation error. */
int vec_reserve(void* self, size_t sizeof_elem, size_t new_cap);

/*! \brief Reserve an extra element, then insert `str` at `index`. */
int vec_insert(void* self, size_t sizeof_elem, size_t index, void* elem);

/*! \brief Reserve an extra element, then insert `str` at the end. */
int vec_push(void* self, size_t sizeof_elem, void* elem);

/*! \brief Non binding request to shrink to size. */
int vec_shrink_to_size(void* self, size_t sizeof_elem);

#define VEC_INIT {0, 0, 0}

#endif
