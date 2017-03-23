/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_DIAGNOSTICS_H
#define HEADER_GUARD_DIAGNOSTICS_H

#include "position.h"

void print_error(const file_position* position,
                 const char* format, ...);
void print_fatal(const file_position* position,
                 const char* format, ...);
void print_warning(const file_position* position,
                   const char* format, ...);

#endif
