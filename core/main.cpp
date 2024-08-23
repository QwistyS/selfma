#include <memory>
#include "container.h"
#include "project.h"
#include "qwistys_alloc.h"

int main() {
    auto container = std::make_unique<Container>();

    ProjectConf conf{0, "Ramen", "Learn to make ramen sup"};
    ProjectConf conf2{0, "Selfma", "Application for task menagment"};

    Task task = {
        .description = "By product for ramen",
    };
    task.duration.set(0, 0, 0, 1, 5, 30);

    Task task1 = {
        .description = "Read a book about ramens",
    };
    task1.duration.set(0, 0, 1, 3, 0, 0);
    
    Task task2 = {
        .description = "Practice to improove your ramen",
    };
    task2.duration.set(0, 0, 2, 0, 0, 0);
    
    Task task3 = {
        .description = "Make ramen to people you love",
    };
    task3.duration.set(1, 0, 0, 0, 0, 0);

    container->add_project(conf);
    container->add_project(conf2);
    container->add_task(0, &task);
    container->add_task(0, &task1);
    container->add_task(0, &task2);
    container->add_task(0, &task3);
    container->print();

    container.reset();
    qwistys_print_memory_stats();
    return 0;
}
