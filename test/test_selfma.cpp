#include <memory>
#include "selfma.h"
#include "unity.h"

#define TEST_HASH_ID "TEST_HASH1_ID"
#define TEST_BUFFER "TEST_BUFFER"

std::unique_ptr<Selfma> selfma = nullptr;
DefaultAPI proj;
DefaultAPI task;

void setUp() {
    selfma = std::make_unique<Selfma>(TEST_HASH_ID, TEST_BUFFER);
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
    proj.descritpion = test_desc_true;
    TEST_ASSERT(selfma->add_project(proj) == false);

    proj.name = test_name_true;
    proj.descritpion = test_desc_false;
    TEST_ASSERT(selfma->add_project(proj) == false);

    // positive
    proj.name = test_name_true;
    proj.descritpion = test_desc_true;
    TEST_ASSERT(selfma->add_project(proj) == true);
}

void test_project_add_task() {
    DefaultAPI proj = {"Project Name Project Name", "RAMEN", 0};
    selfma->add_project(proj);  // Push project.
    proj.project_id = 0;
    proj.name = "Day 2";
    proj.descritpion = "Go to market";
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

    TEST_ASSERT(selfma->remove_project(proj) == true);
}

void test_project_serialize() {
    TEST_ASSERT(selfma->serialise());
}
