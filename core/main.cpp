#include <cstdint>
#include <queue>
#include <thread>
#include <chrono>
#include <memory>
#include "qwistys_macros.h"
#include "selfma.h"

enum SelfmaProto {
    SELFMA_OK = 0,
    ADD_PROJECT,
    ADD_TASK,
    REMOVE_PROJECT,
    REMOVE_TASK,
    UPDATA_PROJECT,
    UPDATA_TASK,
    GET_PROJECT,
    GET_TASK,
    SELFMA_TOTAL,
};


// Range in ms of time sleep
// in case when data isnt in use, we deep the thread to sleep
// However we still wanna to know earlier as possible if some one needs service
// So after each request, sleep timer will be reset, when no operations arn't ressent
// thread goes to sleep on MIN time, next cycle if request doesnt come sleep MORE!!!
// Till Max time will be reached at this point thread is in constunt sleep, do you still need it?
// maby just kill it and reinit when needed?
constexpr uint32_t DEFAULT_SLEEP_TIME = 5; // ms
constexpr uint32_t MAX_SLEEP_TIME = 500; // ms
static bool earth_is_speaning = true;
volatile static uint32_t time_to_sleep = DEFAULT_SLEEP_TIME;

struct SelfmaMsg {
  DefaultAPI args;
    SelfmaProto cmd;
};

static auto selfma = std::make_unique<Selfma>("File", "buffer");

void on_event(DefaultAPI* data) {
    QWISTYS_DEBUG_MSG("Notification from selfma id %zu type %d name %s description %s", 
                      data->project_id, data->notify, data->name.c_str(), data->descritpion.c_str());
}

int main() {

    // Somehow get the file
    // launch the app
    
    std::queue<SelfmaMsg> msgs;
    
    QWISTYS_DEBUG_MSG("Hello Selfma");

    selfma->register_callback(NotifyCode::EVENT_MAX_TIME_SLEEP, on_event);

    DefaultAPI proj = {
        .name = "Ramen",
        .descritpion = "Project about Rame",
    };

    DefaultAPI task = {
        .name = "Learn to make ramen",
        .descritpion = "Read a book or something of how to make Ramen",
        .project_id = 0,
        .task_id = 0,
        .duration = 1, // One sec for task to exist
        .notify = 1,
    };

    SelfmaMsg msg = {
        .args = proj,
        .cmd = SelfmaProto::ADD_PROJECT,
    };
    
    SelfmaMsg msg1 = {
        .args = task,
        .cmd = SelfmaProto::ADD_TASK,
    };

    msgs.push(msg);
    msgs.push(msg1);
    
    while (earth_is_speaning) {
        SelfmaMsg msg;
        selfma->update();

        // Description @definitions
        if (msgs.empty()) {
            QWISTYS_DEBUG_MSG("Sleeping duration %dms", time_to_sleep);
            std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep++));
            if (time_to_sleep == MAX_SLEEP_TIME) {
                DefaultAPI event = {
                    .name = "event",
                    .descritpion = "Max sleep time",
                    .project_id = 0xFFFFFFFF,
                    .task_id = 0,
                    .duration = 0,
                    .notify = NotifyCode::EVENT_MAX_TIME_SLEEP,
                };
                
                time_to_sleep = DEFAULT_SLEEP_TIME;
                selfma->notify(event);
            }
            continue;
        } else {
            msg = msgs.front();
            msgs.pop();
        }
        
        switch (msg.cmd) {
            case ADD_PROJECT:
                selfma->add_project(msg.args);
                break;
            case ADD_TASK:
                selfma->add_task(msg.args);
                break;
            case REMOVE_PROJECT:
                selfma->remove_project(msg.args);
                break;
            case REMOVE_TASK:
                selfma->remove_task(msg.args);
                break;
            case UPDATA_PROJECT:
                break;
            case UPDATA_TASK:
                break;
            case GET_PROJECT:
                break;
            case GET_TASK:
                break;
            default:
                break;
        }
        time_to_sleep = DEFAULT_SLEEP_TIME;
    }

    // Marge the client with file
    // Write Marge to file
    // closeup routin
    
    return 0;
}
