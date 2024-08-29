#ifndef SELFMA_TASK_H
#define SELFMA_TASK_H

#include <chrono>
#include <cstdint>
#include <string>
#include <cstring>
#include <algorithm>
#include <string_view>
#include <time.h>
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

    void set(uint32_t time_in_sec) {
        duration = std::chrono::duration<double>(time_in_sec);
        finished = false;
        start_time = std::chrono::steady_clock::now();
    }
    
    double get_remaining_time() const {
        if (finished) return 0.0;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - start_time);
        return QWISTYS_MAX(0.0, duration.count() - elapsed.count());
    }
};


typedef struct {
    uint32_t id;
    std::string description;
    double duration_in_sec;
} TaskConf_t;

struct Task {
    uint32_t id;
    char description[MAX_DESCRIPTION_LENGTH];
    Timer timer;
    time_t timestamp;

    explicit Task(TaskConf_t* config) : id(config->id), timer(config->duration_in_sec) {
        copy_description(config->description);
        time(&timestamp);
    }

    bool update() {
        timer.update();
        return timer.is_finished();
    }
    
    void print() {
        fprintf(stderr, "-------- Task %d --------\n", id);
        fprintf(stderr, "-\t desc [%s]\n", description);
        fprintf(stderr, "-\t duration [%f sec]\n", timer.get_remaining_time());
        // fprintf(stderr, "-\t timestamp [%d]\t-\n", timestamp);
        fprintf(stderr, "---------------------------\n");
    }
private:
    void copy_description(const std::string& src) {
        auto view = std::string_view(src);
        auto length = QWISTYS_MIN(view.length(), MAX_DESCRIPTION_LENGTH - 1);
        std::copy_n(view.begin(), length, description);
        description[length] = '\0';
    }
};
#endif  // SELFMA_TASK_H
