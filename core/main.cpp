#include <memory>
#include "container.h"
#include "qwistys_alloc.h"

int main() {
    Container conntainer;
    auto container = std::make_unique<Container>();

    conntainer.add_project("Project about Ramen sop", "The Ramen");
        
    conntainer.add_project("Development tracking of selfma", "selfma");
    Task t = {1, "By stuff for ramen sup"};
    Task t1 = {2, "Wash stuf for ramen sup"};
    Task t2 = {3, "Make ramen sup"};

    conntainer.add_task(0, &t);
    conntainer.add_task(0, &t1);
    conntainer.add_task(0, &t2);

    Task p1 = {0, "Make test for container class"};
    Task p2 = {1, "Make test for project class"};
    Task p3 = {2, "Make test for task class"};
    conntainer.add_task(1, &p1);
    conntainer.add_task(1, &p2);
    conntainer.add_task(1, &p3);

    // conntainer.remove_task(1, 0);
    // conntainer.print_projects();
    // conntainer.remove_project(1);
    // conntainer.remove_task(0, 1);
    QWISTYS_DEBUG_MSG("============================");
    conntainer.print_projects();

    Project* it = conntainer.get_project(0);

    if (it) {
        auto task_iter = it->task_vec();
        for (auto task : task_iter) {
            fprintf(stderr, "Task id[%d] descr[%s]\n", task->id, task->description.c_str());
        }
    }
    
    qwistys_print_memory_stats();
    QWISTYS_TODO_MSG("_BUG_ : There is a bug with how nodes gets an id\nIts just a counter, however, it may produce duplicates and confutionw");
    // conntainer.add_task(1, Task* task);
    return 0;
}
