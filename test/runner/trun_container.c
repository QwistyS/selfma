#include "unity.h"
#include "unity_internals.h"
#include "qwistys_alloc.h"

extern void setUp();
extern void tearDown();

extern void test_container_remove_project();
extern void test_container_add_task();
extern void test_container_remove_task();
extern void test_container_get_project();
extern void test_container_project_vec();
extern void test_container_get_size();

int main(void)
{
  UnityBegin("test/runner/test_container.cpp");
  RUN_TEST(test_container_remove_project);
  RUN_TEST(test_container_add_task);
  RUN_TEST(test_container_remove_task);
  RUN_TEST(test_container_get_project);
  RUN_TEST(test_container_project_vec);
  RUN_TEST(test_container_get_size);
  qwistys_print_memory_stats();
  return (UnityEnd());
}
