#include "config.h"
#include "esp_log_pollyfill.h"
#include "state.h"

#define LOG_TAG "APP"

void setup() {
    Serial.begin(115200);
}

void loop () {
    char buffer[32];
    sprintf(buffer, "Polo OS %s", VERSION);
    ESP_LOGI_(LOG_TAG, buffer);

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
    