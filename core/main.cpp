#include <memory>
#include "selfma.h"
#include "qwistys_macros.h"

int main() {

    auto selfma = std::make_unique<Selfma>("File", "buffer");
    QWISTYS_DEBUG_MSG("Hello Selfma");
    DefaultAPI ramen = {
        .name = "Ramen",
        .descritpion = "Project about ramen",
    };

    DefaultAPI app = {
        .name = "Selfma application",
        .descritpion = "Software tracking for selfma project",
    };

    DefaultAPI task_app = {
        .name = "Test the app",
        .descritpion = "Make definition of how to make tests for an application",
        .project_id = 1,
    };

    
    selfma->project_add(ramen);
    selfma->project_add(app);
    selfma->project_add_task(task_app);

        
    return 0;
}
