/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_PARSE_ARGS_H
#define HEADER_GUARD_PARSE_ARGS_H

typedef struct arguments {
    /* NOT null */ const char** files;
    int num_files;
} arguments;

int parse_args(int argc, const char** argv,
               /* Assumes memset to 0 already: */ arguments *args);

#endif
