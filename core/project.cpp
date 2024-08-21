#include <cstdint>
#include <cstdio>
#include <cstring>

#include "project.h"
#include "qwistys_macros.h"

/** Callbacks for avl tree for Project to task */
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
/** End of callback's */ 

VoidResult Project::print() {
    avlt_print(_root, _print);
    return Ok();
}

void Project::self_print() {
    fprintf(stderr, "TBD PROJECT CONF PTING\n");
}

void Project::_init() {
}

void Project::_clean() {

}

uint32_t Project::get_size() {
    return _cunter;
}

uint32_t Project::get_self_id() {
    return config.id;
}

VoidResult Project::del_task(Task* t) {
    avlt_delete(_root, t, _compare, _delet);
    _cunter--;
    return Ok();
}

Task* Project::get_task(uint32_t id) {
    return _get_task(_root, id);
}

std::vector<Task*> Project::task_vec() {
    QWISTYS_TODO_MSG("That's should not be a thinkg due the bug in id indexing");
    auto tree_len= get_size();
    uint increment = 0;
    std::vector<Task*> _tmp;
    
    while (tree_len) {
        Task* task = get_task(increment++);
        if (task) {
            _tmp.push_back(task);
            fprintf(stderr, "Task id[%d] descr[%s]\n", task->id, task->description.c_str());
            tree_len --;
        }
    }
    return _tmp;
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

VoidResult Project::push_task(Task* t) {
    t->id = _cunter++;
    _root = avlt_insert(_root, t, sizeof(Task), _compare);
    return Ok();
}
