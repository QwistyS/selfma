#include <cstdlib>
#include "unity.h"
#include "unity_internals.h"
#include "qwistys_alloc.h"

extern void setUp();
extern void tearDown();

extern void test_project_ctor();
extern void test_project_get_size();
extern void test_project_task_vec();
extern void test_project_del_task();
extern void test_project_get_task();
extern void test_project_get_self_id();

int main(void)
{
  UnityBegin("test/runner/test_project_runne.c");
  RUN_TEST(test_project_ctor);
  RUN_TEST(test_project_get_size);
  RUN_TEST(test_project_task_vec);
  RUN_TEST(test_project_del_task);
  RUN_TEST(test_project_get_task);
  qwistys_print_memory_stats();
  return (UnityEnd());
}
