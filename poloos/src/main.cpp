#include "config.h"
#include "esp_log_pollyfill.h"
#include "state.h"

#define LOG_TAG "APP"

void setup() {
    Serial.begin(115200);
}

void loop () {
    for(int i = 0; i < GPIO_NUM_MAX; i++) {
        gpio_set_direction((gpio_num_t) i, GPIO_MODE_OUTPUT);
    }
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(pdMS_TO_TICKS(250));
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    gpio_set_level(GPIO_NUM_2, 0);

    for(int i = 0; i < GPIO_NUM_MAX; i++) {
        gpio_set_direction((gpio_num_t) i, GPIO_MODE_OUTPUT);
    }

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
    