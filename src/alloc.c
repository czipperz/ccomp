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
