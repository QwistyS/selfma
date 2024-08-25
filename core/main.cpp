#include <memory>
#include "selfma.h"
#include "qwistys_macros.h"

int main() {

    auto  selfma = std::make_unique<Selfma>("Container_id_max256bytes", "user_data_as_buffe_max_1024bytes");

    bool earh_is_speaning = true;
    // selfma_ctx_t* selfma = selfma_create(0, "Container_id_max256bytes", "user_data_as_buffe_max_1024bytes");
    // QWISTYS_ASSERT(selfma);

    // add_project(selfma, "Ramen", "Want to learn ramen");
    // add_task(selfma, 0, "get stuff", "byu stuff for ramen");

    // selfma_print(selfma);
    // selfma_serialize(selfma);
    // selfma_deserialize(selfma);

    // // while(earh_is_speaning) {

    // // }

    // selfma_destroy(selfma);
    return 0;
}

