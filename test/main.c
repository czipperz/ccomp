/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#define TEST(x)                                                      \
    do {                                                             \
        int test_##x();                                              \
        test_##x();                                                  \
    } while (0)

int main() {
    TEST(walk_fpos);
    TEST(vec);
    return 0;
}
