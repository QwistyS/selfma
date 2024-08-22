#include <memory>
#include "project.h"
#include "unity.h"

std::unique_ptr<Project> project;

void setUp() {
    project = std::make_unique<Project>();
}

void tearDown() {
    project.reset();
}

void test_project_ctor() {
    return;
}

void test_project_dtor() {
}

void test_project_get_size() {
    // push nodes and colorate the size;
    // add n and false positive it with < 3 and > 3 
}
void test_project_task_vec() {
}
void test_project_del_task() {
}
void test_project_get_task() {
}
void test_project_get_self_id() {
}
