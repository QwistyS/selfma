#include "container.h"
#include <cstdint>
#include <string_view>
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_avltree.h"
#include "qwistys_macros.h"

/** Callback for avl tree Sort by Project type */
static int _comp(void* a, void* b) {
    Project* pa = (Project*) a;
    Project* pb = (Project*) b;
    return pa->get_self_id() > pb->get_self_id();
}

static void _print(void* a) {
    Project* pa = (Project*) a;
    pa->self_print();
    pa->print();
    fprintf(stderr, "-------------------------------------------\n");
}

static void _del(void* p) {
    Project* pp = (Project*) p;
    fprintf(stderr, "Freeing project id = %d\n", pp->get_self_id());
    QWISTYS_TODO_MSG("Clean each project tree before releasing");
}
/** End of callback's */

uint32_t Container::get_size() {
    return _element_counter;
}

std::vector<Project*> Container::project_vec() {
    uint32_t tree_count = get_size();
    std::vector<Project*> _tmp;
    uint32_t _increment = 0;
    while (tree_count) {
        Project* p = get_project_by_id(_root, _increment++);
        if (p) {
            _tmp.push_back(p);
            tree_count--;
        }
    }
    return _tmp;
}

void Container::_clean() {
    auto projects = project_vec();
    for (auto proj : projects) {
        proj->~Project();
        _root = avlt_delete(_root, proj, _comp, _del);
    }
}

VoidResult Container::add_project(std::string_view description, std::string_view name) {
    ProjectConf _config;
    QWISTYS_TODO_MSG("Wrong id calculation");
    _config.id = _element_counter++;
    _config.description = description;
    _config.name = name;
    time(&_config.created_at);

    Project* pproject = (Project*) qwistys_malloc(sizeof(Project), NULL);
    pproject->config = _config;

    _root = avlt_insert(_root, pproject, sizeof(Project), _comp);
    return Ok();
}

VoidResult Container::remove_project(uint32_t project_id) {
    Project* delete_candidate = get_project_by_id(_root, project_id);

    if (delete_candidate) {
        avlt_delete(_root, delete_candidate, _comp, _del);
        _element_counter--;
        QWISTYS_TODO_MSG("Clea r the Task tree in project before releasing it");
        return Ok();
    }
    return Ok();
}

VoidResult Container::remove_task(uint32_t project_id, uint32_t task_id) {
    Project* p = get_project_by_id(_root, project_id);
    if (p) {
        Task* t = p->get_task(task_id);
        if (t) {
            p->del_task(t);
        }
    }
    return Ok();
}

VoidResult Container::print_projects() {
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
    QWISTYS_ASSERT(project);

    project->push_task(task);
    return Ok();
}
