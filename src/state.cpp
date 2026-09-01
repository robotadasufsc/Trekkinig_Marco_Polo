#include "state.h"
#include <list>
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_timer.h"



#define LOG_TAG "STATE"

typedef struct {
    int64_t time;
    Command command;
} TimedCommand;

static std::list<TimedCommand> command_list;

State debug_loop(CommandStream &cmds, CarControl &ctrl) {
    for(;;) {
        if(cmds.has_commands()) {
            switch(cmds.next_command()) {
                case Command::NONE: ESP_LOGI(LOG_TAG, "NONE"); break;
                case Command::GO_FORWARD: ESP_LOGI(LOG_TAG, "GO_FORWARD"); break;
                case Command::GO_BACKWARDS: ESP_LOGI(LOG_TAG, "GO_BACKWARDS"); break;
                case Command::STOP: ESP_LOGI(LOG_TAG, "STOP"); break;
                case Command::POINT_LEFT: ESP_LOGI(LOG_TAG, "POINT_LEFT"); break;
                case Command::POINT_RIGHT: ESP_LOGI(LOG_TAG, "POINT_RIGHT"); break;
                case Command::POINT_AHEAD: ESP_LOGI(LOG_TAG, "POINT_AHEAD"); break;
                case Command::ABORT: ESP_LOGI(LOG_TAG, "ABORT"); break;
                case Command::BEGIN_RECORDING: ESP_LOGI(LOG_TAG, "BEGIN_RECORDING"); break;
                case Command::STOP_RECORDING: ESP_LOGI(LOG_TAG, "STOP_RECORDING"); break;
                case Command::PLAY_RECORDING: ESP_LOGI(LOG_TAG, "PLAY_RECORDING"); break;
                case Command::LIGHTS_ON: ESP_LOGI(LOG_TAG,"LIGHTS_ON" ); break;
                case Command::LIGHTS_OFF: ESP_LOGI(LOG_TAG,"LIGHTS_OFF" ); break;
                default: ESP_LOGI(LOG_TAG, "???");
            }
        }
    }
    vTaskDelay(1);
}

State idle_loop(CommandStream &cmds, CarControl &ctrl) {
    while(true) {
        while(cmds.has_commands()) {
            switch(cmds.next_command()) {
                case Command::GO_FORWARD:
                    ctrl.go_forward();
                    break;
                case Command::GO_BACKWARDS:
                    ctrl.go_backwards();
                    break;
                case Command::STOP:
                    ctrl.stop();
                    break;
                case Command::POINT_LEFT:
                    ctrl.point_left();
                    break;
                case Command::POINT_AHEAD:
                    ctrl.point_ahead();
                    break;
                case Command::POINT_RIGHT:
                    ctrl.point_right();
                    break;
                case Command::BEGIN_RECORDING:
                    return State::RECORDING;
                case Command::PLAY_RECORDING:
                    return State::REPLICATING;
                case Command::LIGHTS_ON:
                    ctrl.lights_on();
                    break;
                case Command::LIGHTS_OFF:
                    ctrl.lights_off();
                    break;
                default:
                    break;
            }
        }

        ctrl.update_pins();
        vTaskDelay(1);
    }
}

State recording_loop(CommandStream &cmds, CarControl &ctrl) {
    command_list.clear();

    ESP_LOGI(LOG_TAG, "Started recording commands");

    int64_t begin = esp_timer_get_time() / 10000;

    while(true) {
        while(cmds.has_commands()) {
            auto cmd = cmds.next_command();
            switch(cmd) {
                case Command::GO_FORWARD:
                    ctrl.go_forward();
                    break;
                case Command::GO_BACKWARDS:
                    ctrl.go_backwards();
                    break;
                case Command::STOP:
                    ctrl.stop();
                    break;
                case Command::POINT_LEFT:
                    ctrl.point_left();
                    break;
                case Command::POINT_AHEAD:
                    ctrl.point_ahead();
                    break;
                case Command::POINT_RIGHT:
                    ctrl.point_right();
                    break;
                case Command::STOP_RECORDING:
                    ESP_LOGI(LOG_TAG, "Finished recording commands");
                    return State::IDLE;
                case Command::LIGHTS_ON:
                    ctrl.lights_on();
                    break;
                case Command::LIGHTS_OFF:
                    ctrl.lights_off();
                    break;
                default:
                    break;
            }

            command_list.emplace_back(TimedCommand { esp_timer_get_time() / 10000ll - begin, cmd });
        }

        ctrl.update_pins();
        vTaskDelay(1);
    }
}

State replicating_loop(CommandStream &cmds, CarControl &ctrl) {
    ESP_LOGI(LOG_TAG, "Starting to replicate saved path");

    auto begin = esp_timer_get_time() / 10000;

    for(TimedCommand tc : command_list) {
        auto next_command_time = begin + tc.time;

        while(esp_timer_get_time() / 10000 < next_command_time) {
            while(cmds.has_commands()) {
                auto command = cmds.next_command();
                if(command == Command::ABORT) {
                    ctrl.reset();
                    return State::IDLE;
                }
            }
            ctrl.update_pins();
            vTaskDelay(1);
        }

        switch(tc.command) {
            case Command::GO_FORWARD:
                ctrl.go_forward();
                break;
            case Command::GO_BACKWARDS:
                ctrl.go_backwards();
                break;
            case Command::STOP:
                ctrl.stop();
                break;
            case Command::POINT_LEFT:
                ctrl.point_left();
                break;
            case Command::POINT_AHEAD:
                ctrl.point_ahead();
                break;
            case Command::POINT_RIGHT:
                ctrl.point_right();
                break;
            case Command::LIGHTS_ON:
                ctrl.lights_on();
                break;
            case Command::LIGHTS_OFF:
                ctrl.lights_off();
                break;
            default:
                break;
        }
    }

    ESP_LOGI(LOG_TAG, "Finished replicating");

    return State::IDLE;
}
