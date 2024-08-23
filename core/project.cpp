#include <cstdint>
#include <cstdio>
#include <cstring>

#include "error_handler.h"
#include "project.h"
#include "qwistys_avltree.h"
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
    QWISTYS_DEBUG_MSG("Clearing Task id [%d]", t->id);
}

void _print(void* p) {
    Task* t = (Task*) p;
    t->print();
}
/** End of callback's */

VoidResult Project::print() {
    avlt_print(_root, _print);
    return Ok();
}

void Project::self_print() {
    fprintf(stderr, "======== PROJECT %d ========\n", config._id);
    fprintf(stderr, "-\t name - %s \n", config._name);
    fprintf(stderr, "-\t description - %s \n", config._description);
    fprintf(stderr, "===========================\n");
}

void Project::_init() {
    if (auto ret = _id.init(); ret.is_err()) {
        QWISTYS_HALT("Fail to init id system in Project");
    }
}

void Project::clean() {
    if (_root) {
        avlt_free_tree(_root, _delet);
        _root = nullptr;
    }
    _cunter = 0;
    _id.clean();
}

uint32_t Project::size() {
    return _cunter;
}

uint32_t Project::get_self_id() {
    return config._id;
}

VoidResult Project::remove(Task* t) {
    avlt_delete(_root, t, _compare, _delet);
    _id.release(t->id);
    _cunter--;
    return Ok();
}

Task* Project::get_task(uint32_t id) {
    return _get_task(_root, id);
}

std::vector<Task*> Project::to_vector() {
    auto tree_len = size();
    uint _increment = 0;
    std::vector<Task*> _tmp;

    while (tree_len) {
        Task* task = get_task(_increment++);
        if (task) {
            _tmp.push_back(task);
            tree_len--;
        }
        if (_increment == _id.max()) {
            break;
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

VoidResult Project::add(Task* t) {
    auto new_id = _id.next();
    if (new_id.is_err()) {
        Err(ErrorCode::ADD_TASK_FAIL, "Fail to create id for a task", Severity::LOW);
    }
    t->id = new_id.value();
    _root = avlt_insert(_root, t, sizeof(Task), _compare);
    _cunter++;
    QWISTYS_DEBUG_MSG("Task added Success id = %d desc = %s", t->id, t->description);
    return Ok();
}
