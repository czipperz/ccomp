/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "read_file.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "alloc.h"
#include "diagnostics.h"
#include "str.h"
#include "terminal.h"

int read_file(const char* file_name,
              vec_vec_tagged_str *vvstr) {
    int ret = 0;
    FILE* file;
    file_position fpos;
    size_t str = 0;
    size_t vstr = 0;

    assert(file_name);
    assert(vvstr);

    fpos.file_name = file_name;
    fpos.y = 0;
    fpos.x = 0;

    /* Add a empty vector at vvstr */
    {
        vec_tagged_str temp = VEC_INIT;
        if (vec_push(vvstr, sizeof(vec_tagged_str), &temp)) {
            PRINT_MALLOC_ERROR();
            return 1;
        }
    }

    /* Allocate first tagged_str_list */
    vec_reserve(vvstr->ptr + vstr, sizeof(tagged_str), BUFFER_LIST_SIZE);

    /* Open the file. */
    file = fopen(file_name, "r");
    if (!file) {
        terminal_stderr_set_foreground(terminal_red);
        terminal_stderr_set_bold();
        fprintf(stderr, "Error: %s: Unable to open file.\n",
                file_name);
        terminal_stderr_reset();
        ret = 1;
        goto ret;
    }

    /* Now read the file */
    while (!feof(file) && !ferror(file)) {
        tagged_str data;

        /* If vstr is at max size, move to next. */
        if (str >= BUFFER_LIST_SIZE) {
            /* Reserve another list */
            vec_tagged_str temp = VEC_INIT;
            if (vec_push(vvstr, sizeof(temp), &temp)) {
                ret = 1;
                goto cleanup_file;
            }

            ++vstr;
            str = 0;

            /* Create the list */
            vec_reserve(vvstr->ptr + vstr, sizeof(tagged_str),
                        BUFFER_LIST_SIZE);
        }

        /* We read strings in BUFFER_SIZE increments */
        data.str.cap = sizeof(char) * BUFFER_SIZE;
        data.str.str = malloc(data.str.cap);
        if (!data.str.str) {
            ret = 1;
            goto cleanup_file;
        }

        data.str.len = fread(data.str.str, sizeof(char), BUFFER_SIZE, file);

        /* Update file_positions */
        data.fpos = fpos;
        walk_fpos(&fpos, data.str.str, data.str.len);

        vec_push(&vvstr->ptr[vstr], sizeof(tagged_str), &data);
        ++str;
    }

    /* Shrink buffers to size */
    /* *buffers = realloc(*buffers, *buffers_len * sizeof(**buffers)); */

cleanup_file:
    if (ferror(file)) {
        ret = 1;
        terminal_stderr_set_foreground(terminal_red);
        terminal_stderr_set_bold();
        fprintf(stderr, "Error: %s: File reading yielded an error.\n",
                file_name);
        terminal_stderr_reset();
    }
    fclose(file);

ret:
    return ret;
}
