#include <memory>

#include "container.h"
#include "qwistys_alloc.h"
#include "qwistys_macros.h"

int main() {
    QWISTYS_DEBUG_MSG("Hello Selfma");

    auto container = std::make_unique<Container>();
    

    container.reset();
    qwistys_print_memory_stats();

    return 0;
}
