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
