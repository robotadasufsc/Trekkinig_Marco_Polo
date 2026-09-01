#include "config.h"
#include "state.h"
#include "esp_log.h"

#define LOG_TAG "APP"

extern "C" {
    void app_main();
}

void app_main () {
    ESP_LOGI(LOG_TAG, "Polo OS %s", VERSION);

    static CommandStream commands;
    static CarControl ctrl;

    commands.setup();
    
    auto state = State::IDLE;
    
    for(;;) {
        switch(state) {
            case State::IDLE:
                state = idle_loop(commands, ctrl);
                break;
            case State::RECORDING:
                state = recording_loop(commands, ctrl);
                break;
            case State::REPLICATING:
                state = replicating_loop(commands, ctrl);
                break;
        }
    }
}
    