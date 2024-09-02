#ifndef SELFMA_PROJECT_H
#define SELFMA_PROJECT_H

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include "error_handler.h"
#include "ids_pool.h"
#include "qwistys_avltree.h"
#include "task.h"

static void task_on_tree(void* t) {
    Task* task = (Task*) t;
    if (task->update()) {
        task->print();
        task->timer.set(1);
    }
    // here should generate resoponce.
}

class ProjConf final {
public:
    explicit ProjConf(uint32_t id, const std::string& name, const std::string& description)
        : _id(id), _name(name), _description(description), _created_at(0) {
        time(&_created_at);
    }

    // Copy constructor
    ProjConf(const ProjConf& other)
        : _id(other._id), _name(other._name), _description(other._description), _created_at(other._created_at) {}

    // Copy assignment operator
    ProjConf& operator=(const ProjConf& other) {
        if (this != &other) {
            _id = other._id;
            _name = other._name;
            _description = other._description;
            _created_at = other._created_at;
        }
        return *this;
    }

    // Move constructor
    ProjConf(ProjConf&& other) noexcept
        : _id(other._id), _name(other._name), _description(other._description), _created_at(other._created_at) {}

    // Move assignment operator
    ProjConf& operator=(ProjConf&& other) noexcept {
        if (this != &other) {
            _id = other._id;
            _name = other._name;
            _description = other._description;
            _created_at = other._created_at;
        }
        return *this;
    }

    uint32_t _id;
    std::string _name;
    std::string _description;
    time_t _created_at;
};


struct PACKED_STRUCT ProjectConfigurations {
    uint32_t id;
    char name[MAX_NAME_LENGTH];
    char description[MAX_DESCRIPTION_LENGTH];
    time_t created_at;
};
#ifdef USE_MSVC_PRAGMA_PACK
    #pragma pack(pop)
#endif

class Project {
public:
    ProjectConfigurations config; // Serializable data

    Project(const ProjConf& conf) : _error(_drp), _root(nullptr), _cunter(0), _id(4096) {
        _init();
        config.id = conf._id;
        copy_strchar(conf._description, config.description, MAX_DESCRIPTION_LENGTH);
        copy_strchar(conf._name, config.name, MAX_NAME_LENGTH);
        config.created_at = conf._created_at;
        
    };
    ~Project() = default;

    // get size of current ammount of elements in total in the tree. __NOTE__ : not related to id's.
    uint32_t size();
    // MEANWHILE get vector of pointers to each element in the tree {sorted}.
    std::vector<Task*> to_vector();
    // Write task to tree
    VoidResult add(Task* t);
    // Remove task from tree
    VoidResult remove(Task* t);
    // Get pointer to Task after finding it by id, NULL in case not exist
    Task* get_task(uint32_t id);
    uint32_t get_self_id();
    void self_print();
    // Here should come the ruotin we want to call on thread.
    void worker() {
        // scan for stuff to notify about.
        avlt_in_order(_root, task_on_tree);
    }
    // For debug ussage.
    VoidResult print();
    void clean();

private:
    // if needed some stuff @ ctor time.
    void _init();
    // RAII Stuff
    // Get task by node id which is in this case Task structure
    Task* _get_task(avlt_node_t* node, uint32_t task_id);
    uint32_t _cunter;  // :D and yes its a mistake that would leave here forever
    ErrorHandler _error;
    // The main root node of Task's tree
    avlt_node_t* _root;
    DisasterRecoveryPlan _drp;
    IDs _id;

protected:
};

#endif  // SELFMA_PROJECT_H
