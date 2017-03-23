/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "compile_file.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "parse_args.h"
#include "str.h"
#include "read_file.h"
#include "preprocess.h"

int compile_file(const arguments* args, const char* file_name) {
    int ret = 0;

    tagged_str_list* buffers;
    int buffers_len;

    if (read_file(file_name, &buffers, &buffers_len)) {
        ret = 1;
        goto cleanup_buffers;
    }

    if (preprocess(&buffers, buffers + buffers_len)) {
        ret = 1;
        goto cleanup_buffers;
    }

cleanup_buffers:
    {
        int i;
        for (i = 0; i != buffers_len; ++i) {
            destroy_tagged_str_list(buffers + i);
        }
        free(buffers);
    }

    return ret;
}
