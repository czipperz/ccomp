#include "test.h"
#include "position.h"

TEST(_1) {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi", 2);
    ASSERT(fpos.y == 0);
    ASSERT(fpos.x == 2);
}
END_TEST

TEST(_2) {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi\nbye", 6);
    ASSERT(fpos.y == 1);
    ASSERT(fpos.x == 3);
}
END_TEST

static int (*tests[])() = {_1, _2};
RUN(test_walk_fpos)
