#include <memory>
#include "project.h"
#include "qwistys_macros.h"
#include "unity.h"
#include "container.h"

std::unique_ptr<Container> container;

void setUp() {
    container = std::make_unique<Container>();
}
void tearDown() {
    container.reset();
}

void test_container_add_project() {
    ProjectConf config = {
        .id = 0,
        .name = "Test",
        .description = "Test"
    };

    TEST_ASSERT(container.get()->get_size() == 0);
    container.get()->add_project(config);
    TEST_ASSERT(container.get()->get_size() == 1);
}
void test_container_remove_project() {}
void test_container_add_task() {}
void test_container_remove_task() {}
void test_container_get_project() {}
void test_container_project_vec() {}
void test_container_get_size() {}
