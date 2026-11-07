#include <string.h>

#include "commandstream.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define DEVICE_NAME "Marco Polo"
#define LOG_TAG "CMDS"

static CommandStream *singleton;

CommandStream::CommandStream() { singleton = this; }

static void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    singleton->bt_callback(event, param);
}

bool CommandStream::setup() {
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 3. Inicializa Bluetooth Clássico
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable((esp_bt_mode_t) bt_cfg.mode));
    esp_bluedroid_init();
    esp_bluedroid_enable();

    // 4. Configura Perfil SPP
    esp_spp_register_callback(callback);
    esp_spp_cfg_t bt_spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = false,
        .tx_buffer_size = 0,  // Not used for ESP_EPP_MODE_CB
    };
    esp_spp_enhanced_init(&bt_spp_cfg);
    return true;
}

Command CommandStream::next_command() {
    if(this->len == 0) return Command::NONE;

    uint8_t command = this->read_buffer();

    switch(command) {
        case 'F':
            return Command::GO_FORWARD;
        case 'B':
            return Command::GO_BACKWARDS;
        case 'f':
        case 'b':
            return Command::STOP;
        case 'L':
            return Command::POINT_LEFT;
        case 'R':
            return Command::POINT_RIGHT;
        case 'l':
        case 'r':
            return Command::POINT_AHEAD;
        case 'X':
            return Command::ABORT;
        case 'S':
            return Command::STOP_RECORDING;
        case 'C':
            return Command::BEGIN_RECORDING;
        case 'T':
            return Command::PLAY_RECORDING;
        case 'P':
            return Command::LIGHTS_ON;
        case 'p':
            return Command::LIGHTS_OFF;
    }

    return Command::NONE;
}

bool CommandStream::has_commands(){
    return this-> len != 0;
}

void CommandStream::bt_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    switch(event) {
        case ESP_SPP_INIT_EVT:
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
            break;
        case ESP_SPP_START_EVT:
            esp_bt_gap_set_device_name(DEVICE_NAME);
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            ESP_LOGI(LOG_TAG, "Bluetooth pronto! Nome: %s", DEVICE_NAME);
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            client_handle = param->srv_open.handle; // Salva o handle para enviar dados depois
            ESP_LOGI(LOG_TAG, "Cliente Bluetooth Conectado!");
            break;
        case ESP_SPP_DATA_IND_EVT:
            {
                size_t len = param->data_ind.len;
                memcpy(&buffer[this->len], param->data_ind.data, len);
                this->len += len;
            }
            break;
        case ESP_SPP_CLOSE_EVT:
            client_handle = 0;
            ESP_LOGI(LOG_TAG, "Cliente Bluetooth Desconectado.");
            break;
        default:
            break;
    }
}

uint8_t CommandStream::read_buffer() {
    ESP_LOGI(LOG_TAG, "Buffer read:");
    ESP_LOG_BUFFER_HEX(LOG_TAG, buffer, len);
    if(len == 0) return 0;
    uint8_t bits = buffer[0];
    for(int i = 1; i < len; i++)
        buffer[i - 1] = buffer[i];

    len--;
    return bits;
}