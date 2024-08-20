#include "container.h"
#include <cstddef>
#include <cstdio>
#include "error_handler.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_avltree.h"
#include "qwistys_macros.h"
#include "task.h"

static int _comp(void* a, void* b) {
    Project* pa = (Project*) a;
    Project* pb = (Project*) b;
    return pa->id > pb->id;
}

static void _print(void* a) {
    Project* pa = (Project*) a;
    fprintf(stderr, "Project [ id = %d description = %s ] \t Task:\n", pa->id, pa->description.c_str());
    pa->print();
    fprintf(stderr, "-------------------------------------------\n");
}

static void _del(void* p) {
    Project* pp = (Project*) p;
    fprintf(stderr, "Freeing project id = %d\n", pp->id);
    // TODO: add clean too Project.
}

void Container::_clean() {
    QWISTYS_TODO_MSG("Enable contaned DTOR !!!");
    // avlt_free_tree(_root, _del);
}

VoidResult Container::add_project(std::string_view description) {
    Project* pproject = (Project*) qwistys_malloc(sizeof(Project), NULL);
    pproject->id = _element_counter++;
    pproject->description = description;

    _root = avlt_insert(_root, pproject, sizeof(Project), _comp);
    return Ok();
}

VoidResult Container::remove_project(uint32_t project_id) {
    Project* delete_candidate = get_project_by_id(_root, project_id);

    if (delete_candidate) {
        avlt_delete(_root, delete_candidate, _comp, _del);
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
    if (proj->id == id) {
        return proj;
    } else if (proj->id < id) {
        return get_project_by_id(node->right, id);
    } else if (proj->id > id) {
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