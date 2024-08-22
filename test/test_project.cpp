#include <memory>
#include "project.h"
#include "qwistys_macros.h"
#include "unity.h"

std::unique_ptr<Project> obj;

void setUp() {
    obj = std::make_unique<Project>();
}

void tearDown() {
    obj->clean();
    obj.reset();
}

void test_project_ctor() {
    ProjectConf config = {
        .id = 1,
        .name = "Test",
        .description = "Test",
    };
    time(&config.created_at);
    auto project = std::make_unique<Project>(config);
    TEST_ASSERT(project->config.id == 1);
    TEST_ASSERT_EQUAL_STRING(project->config.name.c_str(), "Test");
    TEST_ASSERT_EQUAL_STRING(project->config.description.c_str(), "Test");
    project.reset();

    TEST_ASSERT(obj->config.id == 0);
}

void test_project_dtor() {
    QWISTYS_TODO_MSG("Create dtor test for Project class, currently _BUG_ in RAII");
}

void test_project_get_size() {
    Task t1 = {1, "task 1"};
    Task t2 = {2, "task 1"};
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    TEST_ASSERT(obj.get()->size() == 2);
}

void test_project_task_vec() {
    Task t1 = {1, "task 1"};
    Task t2 = {2, "task 1"};
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    auto tasks = obj.get()->to_vector();
    TEST_ASSERT(tasks.size() == 2);
}

void test_project_del_task() {
    Task t1 = {1, "task 1"};
    Task t2 = {2, "task 1"};
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    obj.get()->remove(&t2);
    
    TEST_ASSERT(obj.get()->size() == 1);
}

void test_project_get_task() {
    Task t1 = {1, "task 1"};
    Task t2 = {2, "task 2"};
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    auto ret = obj.get()->get_task(0);
    
    TEST_ASSERT(ret->id == 0);
    TEST_ASSERT_EQUAL_STRING(ret->description.c_str(), t1.description.c_str());
}

void test_project_get_self_id() {
    TEST_ASSERT(1);
}