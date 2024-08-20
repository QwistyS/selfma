#ifndef SELFMA_CONTAINER_H
#define SELFMA_CONTAINER_H

#include <cstdint>
#include <string_view>
#include "error_handler.h"
#include "project.h"
#include "qwistys_avltree.h"

class Container {
public:
    Container() : _element_counter(0), _root(nullptr) {};

    VoidResult add_project(std::string_view description);
    VoidResult print_projects();
    VoidResult add_task(uint32_t project_id, Task* task);
    Project* get_project(uint32_t id);

private:
    Project* get_project_by_id(avlt_node_t* node, uint32_t id);
    avlt_node_t* _root;
    uint32_t _element_counter;
};

#endif  // SELFMA_CONTAINER_H