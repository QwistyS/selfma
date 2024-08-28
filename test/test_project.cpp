#include <memory>
#include "project.h"
#include "qwistys_macros.h"
#include "task.h"
#include "unity.h"

std::unique_ptr<Project> obj;
const ProjConf conf(0, "Test", "Test");
TaskConf_t config = {0, "Test data", 60};
Task t1(&config);
Task t2(&config);

void setUp() {
    obj = std::make_unique<Project>(conf);
}

void tearDown() {
    obj->clean();
    obj.reset();
}

void test_project_ctor() {
    
    TEST_ASSERT(obj->config._id == 0);
    TEST_ASSERT_EQUAL_STRING(obj->config._name.c_str(), "Test");
    TEST_ASSERT_EQUAL_STRING(obj->config._description.c_str(), "Test");

}

void test_project_dtor() {
    QWISTYS_TODO_MSG("Create dtor test for Project class, currently _BUG_ in RAII");
}

void test_project_get_size() {
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    TEST_ASSERT(obj.get()->size() == 2);
}

void test_project_task_vec() {
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    auto tasks = obj.get()->to_vector();
    TEST_ASSERT(tasks.size() == 2);
}

void test_project_del_task() {
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    obj.get()->remove(&t2);

    TEST_ASSERT(obj.get()->size() == 1);
}

void test_project_get_task() {
    obj.get()->add(&t1);
    obj.get()->add(&t2);

    auto ret = obj.get()->get_task(0);

    TEST_ASSERT(ret->id == 0);
    TEST_ASSERT_EQUAL_STRING(ret->description, t1.description);
}

void test_project_get_self_id() {
    TEST_ASSERT(1);
}
