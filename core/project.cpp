#include <cstdint>
#include "inc/error_handler.h"
#include "inc/task.h"
#include "inc/project.h"

static int _compare(void* t, void* t2) {
    avlt_node_t* first = (avlt_node_t*)t;
    avlt_node_t* second = (avlt_node_t*)t2;

    Task* task = (Task*)&first->user_data;
    Task* task2= (Task*)&second->user_data;
    
    return task->id > task2->id;
}

void Project::_init() {
    _root = avlt_create_node(sizeof(Task));
}

VoidResult Project::del_task(uint32_t id) {
    QWISTYS_TODO_MSG("del_task implemntation");
}

VoidResult Project::write_task(const Task* t) {
    QWISTYS_TODO_MSG("write task implemntation");
    return Ok();
}

VoidResult Project::push_task(Task* t) {
    if(auto ret = write_task(t); ret.is_err() ) {
        avlt_insert(_root, t, sizeof(Task), _compare);
        QWISTYS_TODO_MSG("Define the errors and stuff");
    }
    QWISTYS_DEBUG_MSG("on push");
    return Ok();
}
