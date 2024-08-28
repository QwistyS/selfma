#include <cstdint>
#include <queue>
#include <thread>
#include <chrono>
#include <memory>
#include "qwistys_macros.h"
#include "selfma.h"

enum SelfmaProto {
    OK = 0,
    ADD_PROJECT,
    ADD_TASK,
    REMOVE_PROJECT,
    REMOVE_TASK,
    UPDATA_PROJECT,
    UPDATA_TASK,
    GET_PROJECT,
    GET_TASK,
    TOTAL,
};

constexpr uint32_t DEFAULT_SLEEP_TIME = 5;
constexpr uint32_t MAX_SLEEP_TIME = 0xFF;
static bool earth_is_speaning = true;
volatile static uint8_t time_to_sleep = DEFAULT_SLEEP_TIME;

struct SelfmaMsg {
  DefaultAPI args;
    SelfmaProto cmd;
};


int main() {

    // Somehow get the file
    // launch the app
    
    std::queue<SelfmaMsg> msgs;
    
    auto selfma = std::make_unique<Selfma>("File", "buffer");
    QWISTYS_DEBUG_MSG("Hello Selfma");

    while (earth_is_speaning) {
        SelfmaMsg msg;
        
        if (msgs.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(time_to_sleep++));
            if (time_to_sleep == MAX_SLEEP_TIME) {
                DefaultAPI event = {
                    .name = "event",
                    .descritpion = "Max sleep time",
                    .project_id = 0xFFFFFFFF,
                    .task_id = 0,
                    .duration = 0,
                    .notify = MAX_SLEEP_TIME,
                }
                
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
