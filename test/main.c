
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

void test_blockmap(void **state);
void test_map(void **state);
void test_wad(void **state);

int main(void) {
    int failed_test_count = 0;

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_blockmap),
        cmocka_unit_test(test_map),
        cmocka_unit_test(test_wad),
    };

    failed_test_count = cmocka_run_group_tests(tests, NULL, NULL);

    if (failed_test_count > 0) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vi: set et ts=4 sw=4: */
