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

// to support any archtechure
#ifdef USE_GNU_PACKED_ATTRIBUTE
    #define PACKED_STRUCT __attribute__((packed))
#elif defined(USE_MSVC_PRAGMA_PACK)
    #define PACKED_STRUCT
    #pragma pack(push, 1)
#else
    #define PACKED_STRUCT
#endif

#define MAX_DESCRIPTION_LENGTH 1024
#define MAX_NAME_LENGTH (MAX_DESCRIPTION_LENGTH / 4)

static void copy_strchar(const std::string& src, char* buffer, size_t MAX_DEFINES) {
    auto view = std::string_view(src);
    auto length = QWISTYS_MIN(view.length(), MAX_DEFINES - 1);
    std::copy_n(view.begin(), length, buffer);
    buffer[length] = '\0';
}

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

struct PACKED_STRUCT Task {
    uint32_t id;
    char description[MAX_DESCRIPTION_LENGTH];
    Timer timer;
    time_t timestamp;

    explicit Task(TaskConf_t* config) : id(config->id), timer(config->duration_in_sec) {
        copy_strchar(config->description, description, MAX_DESCRIPTION_LENGTH);
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
};
#ifdef USE_MSVC_PRAGMA_PACK
    #pragma pack(pop)
#endif

#endif  // SELFMA_TASK_H
