#ifndef SELFMA_TASK_H
#define SELFMA_TASK_H

#include <chrono>
#include <time.h>
#include <string.h>
#include "stdlib.h"
#include "qwistys_macros.h"

#define MAX_DESCRIPTION_LENGTH 1024
#define MAX_NAME_LENGTH (MAX_DESCRIPTION_LENGTH / 4)

class Timer {
private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::duration<double> duration;
    bool finished;

public:
    Timer(double seconds)
        : start_time(std::chrono::steady_clock::now()),
          duration(seconds),
          finished(false) {}

    bool is_finished() const {
        return finished;
    }

    void update() {
        if (!finished) {
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration<double>(now - start_time) >= duration) {
                finished = true;
            }
        }
    }

    double get_ramaning_time() const {
        if (finished) return 0.0;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - start_time);
        return QWISTYS_MAX(0.0, duration.count() - elapsed.count());
    }
};


typedef struct {
    uint32_t id;
    char* description;
    double duration_in_sec;
} TaskConf_t;

struct Task {
    uint32_t id;
    char description[MAX_DESCRIPTION_LENGTH];
    Timer timer;
    time_t timestamp;

    explicit Task(TaskConf_t* config) : timer(config->duration_in_sec) {
        id = config->id;
        strncpy(description, config->description, QWISTYS_MIN(strlen(config->description), MAX_NAME_LENGTH - 1));
        time(&timestamp);
    }

    void update() {
        timer.update();
        if (timer.is_finished()) {
            // Notify
        }
    }
    
    void print() {
        fprintf(stderr, "-------- Task %d --------\n", id);
        fprintf(stderr, "-\t desc [%s]\n", description);
        fprintf(stderr, "-\t duration [%f sec\n", timer.get_ramaning_time());
        // fprintf(stderr, "-\t timestamp [%d]\t-\n", timestamp);
        fprintf(stderr, "---------------------------\n");
    }
};

#endif  // SELFMA_TASK_H
