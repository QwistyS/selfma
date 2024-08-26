#ifndef SELFMA_PROJECT_H
#define SELFMA_PROJECT_H

#include <cstdint>
#include <cstring>
#include <ctime>
#include "error_handler.h"
#include "ids_pool.h"
#include "qwistys_avltree.h"
#include "task.h"


class ProjConf final {
public:
    explicit ProjConf(uint32_t id, const char* name, const char* description) 
        : _id(id), _created_at(0) {
        setName(name);
        setDescription(description);
        time(&_created_at);
    }

    // Copy constructor
    ProjConf(const ProjConf& other)
        : _id(other._id), _created_at(other._created_at) {
        memcpy(_name, other._name, MAX_NAME_LENGTH);
        memcpy(_description, other._description, MAX_DESCRIPTION_LENGTH);
    }

    // Copy assignment operator
    ProjConf& operator=(const ProjConf& other) {
        if (this != &other) {
            _id = other._id;
            memcpy(_name, other._name, MAX_NAME_LENGTH);
            memcpy(_description, other._description, MAX_DESCRIPTION_LENGTH);
            _created_at = other._created_at;
        }
        return *this;
    }

    // Move constructor
    ProjConf(ProjConf&& other) noexcept
        : _id(other._id), _created_at(other._created_at) {
        memcpy(_name, other._name, MAX_NAME_LENGTH);
        memcpy(_description, other._description, MAX_DESCRIPTION_LENGTH);
    }

    // Move assignment operator
    ProjConf& operator=(ProjConf&& other) noexcept {
        if (this != &other) {
            _id = other._id;
            memcpy(_name, other._name, MAX_NAME_LENGTH);
            memcpy(_description, other._description, MAX_DESCRIPTION_LENGTH);
            _created_at = other._created_at;
        }
        return *this;
    }

    void setName(const char* name) {
        strncpy(_name, name, MAX_NAME_LENGTH - 1);
        _name[MAX_NAME_LENGTH - 1] = '\0';
    }

    void setDescription(const char* description) {
        strncpy(_description, description, MAX_DESCRIPTION_LENGTH - 1);
        _description[MAX_DESCRIPTION_LENGTH - 1] = '\0';
    }

    uint32_t _id;
    char _name[MAX_NAME_LENGTH];
    char _description[MAX_DESCRIPTION_LENGTH];
    time_t _created_at;
};


class Project {
public:
    ProjConf config;

    Project(const ProjConf& conf) : config(0, "", ""), _error(_drp), _root(nullptr), _cunter(0), _id(4096) {
        _init();
        config = std::move(conf);
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
