#include "config.h"
#include "state.h"
#include "esp_log.h"

#define LOG_TAG "APP"

extern "C" {
    void app_main();
}

void app_main () {
    ESP_LOGI(LOG_TAG, "Polo OS %s", VERSION);

    motors_config_t motors_cfg = {
        .pin_m0 = GPIO_NUM_NC,
        .pin_m1 = GPIO_NUM_NC,
        .pin_m2 = GPIO_NUM_NC,
        .pin_enable = GPIO_NUM_14,
        .pin_sleep = GPIO_NUM_13,
        .pin_reset = GPIO_NUM_27,
        .pin_left_step = GPIO_NUM_26,
        .pin_right_step = GPIO_NUM_22,
        .pin_left_dir = GPIO_NUM_25,
        .pin_right_dir = GPIO_NUM_21,
        .pin_left_fault = GPIO_NUM_33,
        .pin_right_fault = GPIO_NUM_32,
        .microsteps = MotorSubstepsPerStep::MOTOR_SUBSTEPS_1,
    };

    CommandStream commands;
    CarControl ctrl(&motors_cfg);

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
    