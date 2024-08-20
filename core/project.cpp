#include "project.h"
#include <cstdio>
#include <cstring>
#include "error_handler.h"
#include "qwistys_avltree.h"

static int _compare(void* t, void* t2) {
    avlt_node_t* first = (avlt_node_t*) t;
    avlt_node_t* second = (avlt_node_t*) t2;

    Task* task = (Task*) &first->user_data;
    Task* task2 = (Task*) &second->user_data;

    return task->id > task2->id;
}

static void _delet(void* p) {
    avlt_node_t* data = (avlt_node_t*) p;
    Task* t = (Task*) &data->user_data;
    fprintf(stderr, "Deleting task id = %d\n", t->id);
}

void _print(void* p) {
    Task* t = (Task*) p;
    fprintf(stderr, "Task id = %d description = %s \n", t->id, t->description.c_str());
}

VoidResult Project::print() {
    avlt_print(_root, _print);
    return Ok();
}

void Project::_init() {
}

VoidResult Project::del_task(Task* t) {
    avlt_delete(_root, t, _compare, _delet);
    _cunter--;
    return Ok();
}

Task* Project::get_task(uint32_t id) {
    return _get_task(_root, id);
}

Task* Project::_get_task(avlt_node_t* node, uint32_t task_id) {
    if (!node) {
        return NULL;
    }

    Task* task = (Task*) node->user_data;
    if (task->id == task_id) {
        return task;
    } else if (task->id < task_id) {
        return _get_task(node->right, task_id);
    } else if (task->id > task_id) {
        return _get_task(node->left, task_id);
    } else {
        return NULL;
    }
}

VoidResult Project::write_task(const Task* t) {
    QWISTYS_TODO_MSG("write task implemntatoon");
    return Ok();
}

VoidResult Project::push_task(Task* t) {
    _root = avlt_insert(_root, t, sizeof(Task), _compare);
    _cunter++;
    return Ok();
}
