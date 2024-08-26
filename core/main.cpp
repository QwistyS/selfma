#include <memory>
#include "qwistys_alloc.h"
#include "qwistys_macros.h"
#include "selfma.h"


int main() {
    char buffer[1024] = {0};
    auto selfma = std::make_unique<Selfma>("file_path", buffer);

    DefaultAPI args = {
        .name = "Ramen",
        .descritpion = "Project about Ramen sup",
        .project_id = 0,
    };
    
    if (!selfma->project_add(args)) {
        QWISTYS_ERROR_MSG("Fail to add project %s", args.name.c_str());
    }

    DefaultAPI args_task = {
        .name = "Task 1",
        .descritpion = "Get stuff for Ramen sup.",
        .project_id = 0,
        .duration = 60 * 1,
    };

    if (!selfma->project_add_task(args_task)) {
        QWISTYS_ERROR_MSG("Fail to add task %s to project %d", args.name.c_str(), args.project_id);
    }

    if (!selfma->project_serialize()) {
        QWISTYS_ERROR_MSG("Fail to serialise");
    }
    
    selfma.reset();
    qwistys_print_memory_stats();
    return 0;
}
