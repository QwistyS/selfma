#include <memory>
#include "selfma.h"
#include "unity.h"

#define TEST_HASH_ID "TEST_HASH1_ID"
#define TEST_BUFFER "TEST_BUFFER"

static std::unique_ptr<Selfma> selfma = nullptr;
static DefaultAPI args;

void setUp() {
    selfma = std::make_unique<Selfma>(TEST_HASH_ID, TEST_BUFFER);
    args = {"Project Name Project Name", "RAMEN", 0};

    selfma->project_add(args);  // Push project.

    args.project_id = 0;
    args.name = "Task Name";
    args.descritpion = "Make me ramnet";
    selfma->project_add_task(args);
}

void tearDown() {
    selfma.release();
    selfma = nullptr;
}

void test_project_add_args() {
    std::string test_name_false = "";
    std::string test_name_true = "TestName";
    std::string test_desc_false = "";
    std::string test_desc_true = "TestDesc";

    // false positive
    args.name = test_name_false;
    args.descritpion = test_desc_true;
    TEST_ASSERT(selfma->project_add(args) == false);

    args.name = test_name_true;
    args.descritpion = test_desc_false;
    TEST_ASSERT(selfma->project_add(args) == false);

    // positive
    args.name = test_name_true;
    args.descritpion = test_desc_true;
    TEST_ASSERT(selfma->project_add(args) == true);
}

void test_project_add_task() {
    DefaultAPI args1 = {"Project Name Project Name", "RAMEN", 0};
    selfma->project_add(args1);  // Push project.
    args.project_id = 0;
    args.name = "Day 2";
    args.descritpion = "Go to market";
    TEST_ASSERT(selfma->project_add_task(args) == true);
}

void test_project_remove() {
    // push few items
    DefaultAPI args{
        "Project Name Project Name",
        "RAMEN",
        0,
        0,
    };

    TEST_ASSERT(selfma->project_remove_task(args) == true);
    // delete item
    // ask for deleted item
    // check you dont have the item

    TEST_ASSERT(1);
}

void test_project_serialize() {

    selfma->project_serialize();
}
