#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include "selfma.h"
#include "unity.h"

const std::string db_path = STORAGE_PATH"/user_file.hash";
std::unique_ptr<Selfma> selfma = nullptr;
DefaultAPI proj;
DefaultAPI task;

void setUp() {
    selfma = std::make_unique<Selfma>(db_path, nullptr);
    proj = {"Default Project", "RAMEN", 0};
    task = {"Default Task", "Default Task", 0, 0};

    selfma->add_project(proj);
    selfma->add_task(task);
}

void tearDown() {
    auto none = selfma.release();
    selfma = nullptr;
}

void test_project_add_args() {
    std::string test_name_false = "";
    std::string test_name_true = "TestName";
    std::string test_desc_false = "";
    std::string test_desc_true = "TestDesc";

    // false positive
    proj.name = test_name_false;
    proj.description = test_desc_true;
    TEST_ASSERT(selfma->add_project(proj) == false);

    proj.name = test_name_true;
    proj.description = test_desc_false;
    TEST_ASSERT(selfma->add_project(proj) == false);

    // positive
    proj.name = test_name_true;
    proj.description = test_desc_true;
    TEST_ASSERT(selfma->add_project(proj) == true);
}

void test_project_add_task() {
    DefaultAPI proj = {"Project Name Project Name", "RAMEN", 0};
    selfma->add_project(proj);  // Push project.
    proj.project_id = 0;
    proj.name = "Day 2";
    proj.description = "Go to market";
    TEST_ASSERT(selfma->add_task(proj) == true);
}

void test_project_remove() {
    // push few items
    DefaultAPI proj{
        "Project Name Project Name",
        "RAMEN",
        0,
        0,
    };

    selfma->add_project(proj);

    TEST_ASSERT(selfma->remove_project(proj) == true);
    // True in case when project not exist, still means deleted.
    TEST_ASSERT(selfma->remove_project(proj) == true);
}

void test_project_serialize() {
    // Push 5 more porjects
    for (uint32_t i = 1; i < 5; i++) {
        DefaultAPI _proj = {"PROJECT_TEST", "PROJ_DESC_TEST", 0, 0, 0, 0};
        selfma->add_project(_proj);
    }
    
    for (uint32_t projects_size = 0; projects_size < 6; projects_size++) {
        for (uint32_t i=0; i<10; i++) {
            DefaultAPI _task = {"TEST_TASK", "TEST_DESC_TASK"+std::to_string(i), projects_size, 0, 100, 0};
            selfma->add_task(_task);
        }
    }
    TEST_ASSERT(selfma->serialize());
}

void test_project_deserialize() {
    TEST_ASSERT(selfma->deserialize());
}
