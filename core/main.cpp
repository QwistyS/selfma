#include "container.h"
#include "task.h"

int main() {
    QWISTYS_DEBUG_MSG("Hello App");
    Container conntainer;

    conntainer.add_project("Ramen");
    conntainer.add_project("Selfma app");
    Task t = {1, "By stuff for ramen sup"};
    Task t1 = {2, "Wash stuf for ramen sup"};
    Task t2 = {3, "Make ramen sup"};

    conntainer.add_task(0, &t);
    conntainer.add_task(0, &t1);
    conntainer.add_task(0, &t2);
    conntainer.print_projects(); 

    // conntainer.add_task(1, Task* task);
    return 0;
    
}
