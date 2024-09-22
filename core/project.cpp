#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "error_handler.h"
#include "project.h"
#include "qwistys_macros.h"
#include "task.h"

static void task_on_tree(void* t, void* e) {
    Task* task = (Task*) t;
    if (task->update()) {
        task->timer.set(1);
        DefaultAPI event = {};
        auto callbacks = (std::array<event_callback, NotifyCode::NOTIFY_TOTAL>*) e;
        auto cb = (*callbacks)[NotifyCode::TASK_TIME_ELAPSED];
        if (cb) {
            cb(&event);
        }
    }
}

static int _compare(void* t, void* t2) {
    avlt_node_t* first = (avlt_node_t*) t;
    avlt_node_t* second = (avlt_node_t*) t2;

    Task* task = (Task*) &first->user_data;
    Task* task2 = (Task*) &second->user_data;

    if (task->id < task2->id)
        return -1;
    if (task->id > task2->id)
        return 1;
    return 0;
}

static void _delet(void* p) {
    if (!p)
        return;

    Task* t = (Task*) p;
    QWISTYS_DEBUG_MSG("Clearing Task id [%d]", t->id);
    // Nothing to do here, the ownership of the memorie is @ avl layer.
    // So unles you want to reset some fields in Task structure go nus
    // but after this scope will end this memorye will be free by avl_delete.
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
    fprintf(stderr, "======== PROJECT %d ========\n", config.id);
    fprintf(stderr, "-\t name - %s \n", config.name);
    fprintf(stderr, "-\t description - %s \n", config.description);
    fprintf(stderr, "===========================\n");
}

void Project::_init() {
    if (auto ret = _id.init(); ret.is_err()) {
        QWISTYS_HALT("Fail to init id system in Project");
    }
    QWISTYS_DEBUG_MSG("Success of creating IDs instance for Project id %d", config.id);
}

void Project::clean() {
    if (_root && _cunter) {
        QWISTYS_DEBUG_MSG("Cleaning project %d...", config.id);
        avlt_free_tree(_root, _delet);
        _cunter = 0;
    }
    _root = nullptr;
    _id.clean();
}

uint32_t Project::size() {
    return _cunter;
}

uint32_t Project::get_self_id() {
    return config.id;
}

VoidResult Project::remove(Task* t) {
    if (_cunter == 0) {
        return Ok();
    }
    avlt_delete(_root, t, _compare, _delet);
    _id.release(t->id);
    _cunter--;
    return Ok();
}

Task* Project::get_task(uint32_t id) {
    return _get_task(_root, id);
}

void Project::worker(void* e) {
    // scan for stuff to notify about.
    avlt_in_order(_root, task_on_tree, e);
}

std::vector<Task*> Project::to_vector() {
    auto tree_len = size();
    uint32_t _increment = 0;
    std::vector<Task*> _tmp;
    _tmp.reserve(tree_len);

    while (tree_len) {
        Task* task = get_task(_increment++);
        if (task) {
            _tmp.emplace_back(task);
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
