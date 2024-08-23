#ifndef SELFMA_CONTAINER_H
#define SELFMA_CONTAINER_H

#include <cstdint>
#include "error_handler.h"
#include "project.h"
#include "qwistys_avltree.h"
#include "ids_pool.h"

class Container {
public:
    Container() : _element_counter(0), _root(nullptr), _id(4096) { _init(); };
    ~Container() { _clean(); };

    VoidResult add_project(ProjectConf& config);
    VoidResult remove_project(uint32_t project_id);
    VoidResult print();
    VoidResult add_task(uint32_t project_id, Task* task);
    VoidResult remove_task(uint32_t project_id, uint32_t task_id);
    Project* get_project(uint32_t id);
    std::vector<Project*> to_vector();
    uint32_t size();
private:
    Project* get_project_by_id(avlt_node_t* node, uint32_t id);
    void _init();
    void _clean();
    avlt_node_t* _root;
    uint32_t _element_counter;
    IDs _id;

};

#endif  // SELFMA_CONTAINER_H
