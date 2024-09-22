#include "unity.h"
#include "unity_internals.h"
#include "qwistys_alloc.h"

extern void setUp();
extern void tearDown();

extern void test_project_add_args();
extern void test_project_remove();
extern void test_project_remove_task();
extern void test_project_add_task();
extern void test_project_serialize();
extern void test_project_deserialize();

int main(void) {
    UnityBegin("test/runner/test_selfma.c");

    RUN_TEST(test_project_add_args);
    RUN_TEST(test_project_add_task);
    RUN_TEST(test_project_remove_task);
    RUN_TEST(test_project_remove);
    RUN_TEST(test_project_serialize);
    RUN_TEST(test_project_deserialize);
    qwistys_print_memory_stats();
    return (UnityEnd());
}
