#include <memory>
#include "project.h"
#include "unity.h"
#include "container.h"

std::unique_ptr<Container> container;
ProjConf conf(0, "test", "test");

void setUp() {
    container = std::make_unique<Container>();
}
void tearDown() {
    container.reset();
}

void test_container_add_project() {
    TEST_ASSERT(container.get()->size() == 0);
    container.get()->add_project(conf);
    TEST_ASSERT(container.get()->size() == 1);
}
void test_container_remove_project() {}
void test_container_add_task() {}
void test_container_remove_task() {}
void test_container_get_project() {}
void test_container_project_vec() {}
void test_container_get_size() {}
