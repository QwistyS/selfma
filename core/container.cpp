#include "container.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include "error_handler.h"
#include "project.h"
#include "qwistys_alloc.h"
#include "qwistys_avltree.h"
#include "qwistys_macros.h"

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

VoidResult Container::add_project(std::string_view description) {
    Project* pproject = (Project*) qwistys_malloc(sizeof(Project), NULL);
    pproject->id = _element_counter++;
    pproject->description = description;

    _root = avlt_insert(_root, pproject, sizeof(Project), _comp);
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
    if (proj->id > id) {
        return get_project_by_id(node->left, id);
    } else if (proj->id < id) {
        return get_project_by_id(node->right, id);
    } else if (proj->id == id) {
        return proj;
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