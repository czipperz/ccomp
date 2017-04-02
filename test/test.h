#ifndef HEADER_GUARD_TEST_H
#define HEADER_GUARD_TEST_H

#include <stdio.h>
#include <stddef.h>

#define TEST(name)                                                   \
    static int name() {                                              \
        int _ret = 0;                                                \
        const char* const _name = #name;
#define END_TEST                                                     \
        return _ret;                                                 \
    }

#define PRINT_TEST_FAILED()                                          \
    do {                                                             \
        fprintf(stderr, "Test failed: %s:%s\n", __FILE__, _name);    \
    } while (0)
#define PRINT_ASSERTION_FAILED(str)                                  \
    do {                                                             \
        fprintf(stderr, "Assertion failed on line %d: %s\n",         \
                __LINE__, str);                                      \
    } while (0)

#define LAZY_ASSERT(cond)                                            \
    do {                                                             \
        if (!(cond)) {                                               \
            if (!_ret) {                                             \
                PRINT_TEST_FAILED();                                 \
            }                                                        \
            PRINT_ASSERTION_FAILED(#cond);                           \
            _ret = 1;                                                \
        }                                                            \
    } while (0)
#define ASSERT(cond)                                                 \
    do {                                                             \
        if (!(cond)) {                                               \
            if (!_ret) {                                             \
                PRINT_TEST_FAILED();                                 \
            }                                                        \
            PRINT_ASSERTION_FAILED(#cond);                           \
            return 1;                                                \
        }                                                            \
    } while (0)

#define RUN(name)                                                    \
    int name() {                                                     \
        int _ret = 0;                                                \
        size_t i;                                                    \
        for (i = 0; i != sizeof(tests) / sizeof(*tests); ++i) {      \
            if (tests[i]()) {                                        \
                _ret = 1;                                            \
            }                                                        \
        }                                                            \
        return _ret;                                                 \
    }

#endif
