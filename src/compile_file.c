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
#include "read_file.h"
#include "preprocess.h"
#include "position.h"
#include "vec.h"

int compile_file(const arguments* args, const char* file_name) {
    int ret = 0;

    vec_vec_tagged_str buffers = VEC_INIT;

    if (read_file(file_name, &buffers)) {
        ret = 1;
        goto cleanup_buffers;
    }

    /* if (preprocess(&buffers)) { */
    /*     ret = 1; */
    /*     goto cleanup_buffers; */
    /* } */

cleanup_buffers:
    destroy_vec_vec_tagged_str(&buffers);

    return ret;
}
