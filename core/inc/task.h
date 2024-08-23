#ifndef SELFMA_TASK_H
#define SELFMA_TASK_H

#include <time.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "stdlib.h"
#include "qwistys_macros.h"

#define MAX_DESCRIPTION_LENGTH 1024

struct Duration {
    uint8_t year;
    uint8_t mounth;
    uint8_t week;
    uint8_t day;
    uint8_t hour;
    uint8_t min;

    void set(uint8_t year = 0, uint8_t mounth = 0, uint8_t week = 0, uint8_t day = 0, uint8_t hour = 0,
             uint8_t min = 0) {
        _set_year(year);
        _set_mounth(mounth);
        _set_week(week);
        _set_days(day);
        _set_hours(hour);
        _set_min(min);
    }

private:
    void _set_year(uint8_t y) { year = y; }
    void _set_mounth(uint8_t m) { mounth = m; }
    void _set_week(uint8_t w) { week = w; }
    void _set_days(uint8_t d) { day = d; }
    void _set_hours(uint8_t h) { hour = h; }
    void _set_min(uint8_t m) { min = m; }
};

typedef struct {
    uint32_t id;
    Duration duration;
    char* description;
} TaskConf_t;

struct Task {
    uint32_t id;
    char description[MAX_DESCRIPTION_LENGTH];
    Duration duration;
    time_t timestamp;

    explicit Task(TaskConf_t* config) {
        id = config->id;
        duration = config->duration;
        
        // Copy description
        int min = QWISTYS_MIN(strlen(config->description), MAX_DESCRIPTION_LENGTH - 1);
        memcpy(description, config->description, min);
        description[min] = '\0';     
        time(&timestamp);
    }
    
    void print() {
        fprintf(stderr, "-------- Task %d --------\n", id);
        fprintf(stderr, "-\t desc [%s]\n", description);
        fprintf(stderr, "-\t duration [%dy-%dm-%dw-%dd-%dh-%dm]\n", duration.year, duration.mounth, duration.week,
                duration.day, duration.hour, duration.min);
        // fprintf(stderr, "-\t timestamp [%d]\t-\n", timestamp);
        fprintf(stderr, "---------------------------\n");
    }
};

#endif  // SELFMA_TASK_H
