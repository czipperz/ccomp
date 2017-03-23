/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "compile_file.h"
#include "parse_args.h"

int main(int argc, const char** argv) {
    arguments args;
    int ret = 0;
    int i;

    --argc;
    ++argv;

    /* Parse arguments */
    memset(&args, 0, sizeof(arguments));
    if (parse_args(argc, argv, &args)) {
        ret = 1;
        goto ret;
    }

    /* Compile each file */
    for (i = 0; i != args.num_files; ++i) {
        if (compile_file(&args, args.files[i])) {
            ret = 1;
            goto cleanup_parse_args;
        }
    }

cleanup_parse_args:
    free(args.files);

ret:
    return ret;
}
