#include "unity.h"
#include "unity_internals.h"

extern void setUp();
extern void tearDown();

extern void test_project_add_args();
extern void test_project_remove();
extern void test_project_add_task();

int main(void) {
    UnityBegin("test/runner/test_selfma.c");

    RUN_TEST(test_project_add_args);
    RUN_TEST(test_project_remove);
    RUN_TEST(test_project_add_task);

    return (UnityEnd());
}
