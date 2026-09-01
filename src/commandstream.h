#pragma once

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

enum class Command {
    NONE = 0,

    GO_FORWARD,
    GO_BACKWARDS,
    STOP,

    POINT_LEFT,
    POINT_RIGHT,
    POINT_AHEAD,

    ABORT,
    BEGIN_RECORDING,
    STOP_RECORDING,
    PLAY_RECORDING,

    LIGHTS_ON,
    LIGHTS_OFF
};

class CommandStream final {
    public:
        CommandStream();

        bool setup();

        Command next_command();
        bool has_commands();
        
        void bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    private:
        uint32_t client_handle;
        uint8_t buffer[128];
        size_t len;
        uint8_t read_buffer();
    };
