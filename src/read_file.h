/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_READ_FILE_H
#define HEADER_GUARD_READ_FILE_H

#include "position.h"

int read_file(const char* file_name, tagged_str_list* *buffers,
              int *buffers_len);

#endif
