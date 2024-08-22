#ifndef SELFMA_PROJECT_H
#define SELFMA_PROJECT_H

#include <vector>
#include "error_handler.h"
#include "qwistys_avltree.h"
#include "task.h"
#include <ctime>
#include "ids_pool.h"

struct ProjectConf {
    uint32_t id;
    std::string name;
    std::string description;
    time_t created_at;
};

class Project {
public:
    ProjectConf config;
    
    Project(const ProjectConf& conf) : config(conf), _error(_drp), _root(nullptr), _cunter(0) , _id(4096) { _init(); };
    Project() : config({0}), _error(_drp), _root(nullptr), _cunter(0) , _id(4096) { _init(); };
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
