/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include <string.h>
#include <assert.h>

#if !TEST_MODE
#include <stdio.h>
#include <stdlib.h>

#include "compile_file.h"
#include "parse_args.h"
#include "terminal.h"

int main(int argc, const char** argv) {
    arguments args = {0, 0};
    int ret = 0;
    size_t i;

    --argc;
    ++argv;

    /* Parse arguments */
    if (parse_args(argc, argv, &args)) {
        ret = 1;
        goto ret;
    }

    if (args.num_files == 0) {
        terminal_stderr_set_foreground(terminal_red);
        terminal_stderr_set_bold();
        fputs("fatal error:", stderr);
        terminal_stderr_reset();
        fputs(" no input files\n", stderr);
        return 1;
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

#else
#include "test.h"

int failures = 0;
int successes = 0;

#define run(test)                                                    \
    do {                                                             \
        void test();                                                 \
        test();                                                      \
    } while (0)

int main() {
    run(test_vec);
    run(test_str);
    run(test_position);
    /* run(test_delete_block_comments); */
    printf("%d of %d succeeded.\n", successes, failures + successes);
    return failures;
}
#endif
