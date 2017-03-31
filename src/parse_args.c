/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "parse_args.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "diagnostics.h"

int parse_args(int argc, const char** argv, arguments* args) {
    int ret = 0;
    int i;

    for (i = 0; i != argc; ++i) {
        REALLOC(args->files, sizeof(char*) * (1 + args->num_files), {
            ret = 1;
            PRINT_MALLOC_ERROR();
            goto fail;
        });
        args->files[args->num_files] = argv[i];
        ++args->num_files;
    }
    return ret;

fail:
    free(args->files);
    return ret;
}
