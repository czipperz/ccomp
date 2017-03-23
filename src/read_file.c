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
#include "str.h"
#include "terminal.h"

int read_file(const char* file_name,
              tagged_str_list* *buffers, int *buffers_len) {
    int ret = 0;
    FILE* file;
    int buffers_max = 16;
    tagged_str_list* list;
    file_position fpos;

    fpos.file_name = file_name;
    fpos.y = 0;
    fpos.x = 0;

    assert(file_name);
    assert(buffers);
    assert(buffers_len);

    /* Allocate dynamic array of tagged_str_list */
    *buffers_len = 0;
    CALLOC(*buffers, buffers_max, sizeof(**buffers), {
        *buffers = 0;
        return 1;
    });

    /* Allocate first tagged_str_list */
    list = *buffers;
    CALLOC(list->strs, BUFFER_LIST_SIZE, sizeof(*list->strs), { return 1; });
    list->len = 0;

    *buffers_len = 1;

    /* Open the file. */
    file = fopen(file_name, "r");
    if (!file) {
        fprintf(stderr, "Error: %s: Unable to open file.\n",
                file_name);
        ret = 1;
        goto ret;
    }

    /* Now read the file */
    while (!feof(file) && !ferror(file)) {
        tagged_str data;

        if (list->len >= BUFFER_LIST_SIZE) {
            /* Reserve another list */
            if (vector_reserve_another((void**) buffers,
                                       buffers_len, &buffers_max,
                                       sizeof(**buffers))) {
                ret = 1;
                goto cleanup_file;
            }

            list = *buffers + *buffers_len - 1;

            /* Create the list */
            MALLOC(list->strs, BUFFER_LIST_SIZE * sizeof(*list->strs),
                   {
                       memset(list, 0, sizeof(*list));
                       ret = 1;
                       goto cleanup_file;
                   });
            list->len = 0;
            list->cap = BUFFER_LIST_SIZE;
        }

        /* We read strings in BUFFER_SIZE increments */
        MALLOC(data.str, sizeof(char) * BUFFER_SIZE, {
            ret = 1;
            goto cleanup_file;
        });

        data.len = fread(data.str, sizeof(char), BUFFER_SIZE, file);

        /* Update file_positions */
        data.fpos = fpos;
        walk_fpos(&fpos, data.str, data.len);

        list->strs[list->len] = data;
        ++list->len;
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
