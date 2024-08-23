#include <memory>
#include "container.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "task.h"

int main() {
    auto container = std::make_unique<Container>();

    {
        ProjConf ramen(0, "Ramen", "Learn to make cool Ramen");
        ProjConf slema(0, "Selfma", "Self managment app");

        TaskConf_t task_conf = {0, {0, 0, 0, 0, 0, 0}, "By products for Ramen"};
        Task t(&task_conf);

        container->add_project(ramen);
        container->add_task(0, &t);
    }

    container->print();

    container.reset();
    qwistys_print_memory_stats();
    return 0;
}
