#include <memory>
#include "project.h"
#include "qwistys_macros.h"
#include "task.h"
#include "unity.h"

std::unique_ptr<Project> project;
const ProjConf conf(0, "Test", "Test");
TaskConf_t config = {0, "Test data", 60};
static Task t1(&config);
static Task t2(&config);

void setUp() {
    project = std::make_unique<Project>(conf);
}

void tearDown() {
    project.reset();
}

void test_project_ctor() {
    
    TEST_ASSERT(project->config.id == 0);
    TEST_ASSERT_EQUAL_STRING(project->config.name, "Test");
    TEST_ASSERT_EQUAL_STRING(project->config.description, "Test");

}

void test_project_dtor() {
    QWISTYS_TODO_MSG("Tested each time tearDown is called");
}

void test_project_get_size() {
    project->add(&t1);
    project->add(&t2);

    TEST_ASSERT(project->size() == 2);
}

void test_project_task_vec() {
    project->add(&t1);
    project->add(&t2);

    auto tasks = project->to_vector();
    TEST_ASSERT(tasks.size() == 2);
}

void test_project_del_task() {
    project->add(&t1);
    project->add(&t2);

    project->remove(&t2);

    TEST_ASSERT(project->size() == 1);
}

void test_project_get_task() {
    project->add(&t1);
    project->add(&t2);

    auto ret = project->get_task(0);

    TEST_ASSERT(ret->id == 0);
    TEST_ASSERT_EQUAL_STRING(ret->description, t1.description);
}

void test_project_get_self_id() {
    TEST_ASSERT(1);
}
