#ifndef SELFMA_PROJECT_H
#define SELFMA_PROJECT_H

#include <cstdint>
#include "error_handler.h"
#include "qwistys_avltree.h"
#include "task.h"

class Project {
    public:
        uint32_t id;
        std::string description;
        Project() : _error(_drp), _root(nullptr) { _init(); };
        ~Project() = default;
        VoidResult push_task(Task* t);
        VoidResult del_task(Task* t);
        VoidResult print();
    private:
        VoidResult write_task(/*db connection, */ const Task* t);
        VoidResult read_task(/*db connection, */ Task& t);
        void _init();
            
    ErrorHandler _error;
    avlt_node_t* _root;
    DisasterRecoveryPlan _drp;
    protected:
};

#endif // SELFMA_PROJECT_H
