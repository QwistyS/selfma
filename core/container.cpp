#include "container.h"
#include <cstring>
#include <string>
#include "error_handler.h"
#include "project.h"
#include "qwistys_avltree.h"
#include "qwistys_macros.h"
#include "iostream"
/** Callback for avl tree Sort by Project type */
static int _comp(void* a, void* b) {
    Project* pa = (Project*) a;
    Project* pb = (Project*) b;
    if (pa->get_self_id() < pb->get_self_id())
        return -1;
    if (pa->get_self_id() > pb->get_self_id())
        return 1;
    return 0;
}

static void _print(void* a) {
    Project* pa = (Project*) a;
    pa->self_print();
    pa->print();
}

static void _update(void* p, void* e) {
    Project* project = (Project*) p;
    project->worker(e);
}

static void _del(void* p) {
    if (!p) {
        return;
    }
    Project* obj = (Project*) p;
    QWISTYS_DEBUG_MSG("Clearing tree of project [%d]", obj->config.id);
    obj->clean();
}
/** End of callback's */

void Container::_init() {
    if (auto ret = _id.init(); ret.is_err()) {
        QWISTYS_HALT("Fail to init id system in Container");
    }
}

uint32_t Container::size() {
    return _element_counter;
}

std::vector<Project*> Container::to_vector() {
    uint32_t tree_count = size();
    std::vector<Project*> _tmp;
    uint32_t _increment = 0;
    while (tree_count) {
        Project* p = get_project_by_id(_root, _increment++);
        if (p) {
            _tmp.push_back(p);
            tree_count--;
        }
        if (_increment == _id.max()) {
            break;
        }
    }
    return _tmp;
}

void Container::_clean() {
    QWISTYS_DEBUG_MSG("Cleaning container ...");
    if (_root) {
        avlt_free_tree(_root, _del);
        _root = nullptr;
    }
    _element_counter = 0;
    _id.clean();
}

VoidResult Container::add_project(ProjConf& config) {
    // Set id to config before writing the object to tree
    auto new_id = _id.next();
    if (new_id.is_err()) {
        return Err(ErrorCode::ADD_PROJECT_FAIL, "Fail to add poject generate id fail");
    }
    config._id = new_id.value();

    // Create object with configurations safe and secure ;)
    Project p(config);
    // Copy data to tree
    _root = avlt_insert(_root, &p, sizeof(Project), _comp);
    _element_counter++;
    QWISTYS_DEBUG_MSG("Project {%s} added successful", p.config.name);
    return Ok();
}
VoidResult Container::remove_project(uint32_t project_id) {
    if (!_root || _element_counter == 0) {
        return Ok();
    }

    Project* delete_candidate = get_project_by_id(_root, project_id);

    if (delete_candidate) {
        _root = avlt_delete(_root, delete_candidate, _comp, _del);
        if (_root) {  // Check if the tree is not empty after deletion
            _id.release(project_id);
            _element_counter--;
        } else {
            _element_counter = 0;  // Tree is now empty
            _root = nullptr;
        }
        return Ok();
    }
    return Err(ErrorCode::PROJECT_NOT_FOUND, "Project not found for deletion");
}

VoidResult Container::remove_task(uint32_t project_id, uint32_t task_id) {
    Project* p = get_project_by_id(_root, project_id);
    if (p) {
        Task* t = p->get_task(task_id);
        if (t) {
            p->remove(t);
            return Ok();
        }
    }
    return Err(ErrorCode::REMOVE_TASK_FAIL, "Fail to remove task id " + std::to_string(task_id), Severity::LOW);
}

VoidResult Container::print() {
    avlt_print(_root, _print);
    return Ok();
}

Project* Container::get_project(uint32_t id) {
    return get_project_by_id(_root, id);
}

Project* Container::get_project_by_id(avlt_node_t* node, uint32_t id) {
    if (!node) {
        return NULL;
    }

    Project* proj = (Project*) node->user_data;
    if (proj->get_self_id() == id) {
        return proj;
    } else if (proj->get_self_id() < id) {
        return get_project_by_id(node->right, id);
    } else if (proj->get_self_id() > id) {
        return get_project_by_id(node->left, id);
    } else {
        return NULL;
    }
}

VoidResult Container::add_task(uint32_t project_id, Task* task) {
    Project* project = get_project(project_id);
    if (!project) {
        return Err(ErrorCode::OK, "project Id[" + std::to_string(project_id) + "] does not exis");
    }
    return project->add(task);
}

void Container::update(void* cbs) {
    avlt_in_order(_root, _update, cbs);
}

void Container::print_tree_hierarchy() {
    if (!_root) {
        std::cout << "Tree is empty" << std::endl;
        return;
    }
    print_node(_root, "", true);
}

void Container::print_node(avlt_node_t* node, const std::string& prefix, bool isLeft) {
    if (!node) return;

    std::cout << prefix;
    std::cout << (isLeft ? "├──" : "└──" );

    Project* proj = (Project*)node->user_data;
    std::cout << proj->get_self_id() << std::endl;

    // Prepare strings for children
    std::string childPrefix = prefix + (isLeft ? "│   " : "    ");

    print_node(node->right, childPrefix, true);
    print_node(node->left, childPrefix, false);
}
