#include "position.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void _1() {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi", 2);
    assert(fpos.y == 0);
    assert(fpos.x == 2);
}

static void _2() {
    file_position fpos = {"", 0, 0};
    walk_fpos(&fpos, "hi\nbye", 6);
    assert(fpos.y == 1);
    assert(fpos.x == 3);
}

void test_walk_fpos() {
    _1();
    _2();
}
