#ifndef SELFMA_PROJECT_H
#define SELFMA_PROJECT_H

#include "error_handler.h"
#define QWISTYS_AVLT_IMPLEMENTATION
#include "qwistys_avltree.h"

#include "task.h"

class Project {
    public:
        Project() : _error(_drp), _root(nullptr) { _init(); };
        ~Project() = default;
        VoidResult push_task(Task* t);
        VoidResult del_task(uint32_t id);
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
