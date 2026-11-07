#include "motors.h"

#include <driver/ledc.h>
#include <esp_check.h>

#define LOG_TAG "MOTORS"

#define PIN_CMP(pin_one, pin_two) (config->pin_ ## pin_one == config->pin_ ## pin_two)
#define PIN_MASK(pin_name) (config->pin_ ## pin_name == GPIO_NUM_NC ? 0 : 1ull << config->pin_ ## pin_name)

Motors::Motors(const motors_config_t *config) {
    if(
        ((config->pin_m0 == GPIO_NUM_NC) != (config->pin_m1 == GPIO_NUM_NC) || (config->pin_m0 == GPIO_NUM_NC) != (config->pin_m2 == GPIO_NUM_NC)) ||
        (config->pin_m0 != GPIO_NUM_NC && (
            (PIN_CMP(m0, m1) || PIN_CMP(m0, m2) || PIN_CMP(m0, enable) || PIN_CMP(m0, sleep) || PIN_CMP(m0, reset) || PIN_CMP(m0, left_step) || PIN_CMP(m0, right_step) || PIN_CMP(m0, left_dir) || PIN_CMP(m0, right_dir) || PIN_CMP(m0, left_fault) || PIN_CMP(m0, right_fault)) ||
            (PIN_CMP(m1, m2) || PIN_CMP(m1, enable) || PIN_CMP(m1, sleep) || PIN_CMP(m1, reset) || PIN_CMP(m1, left_step) || PIN_CMP(m1, right_step) || PIN_CMP(m1, left_dir) || PIN_CMP(m1, right_dir) || PIN_CMP(m1, left_fault) || PIN_CMP(m1, right_fault)) ||
            (PIN_CMP(m2, enable) || PIN_CMP(m2, sleep) || PIN_CMP(m2, reset) || PIN_CMP(m2, left_step) || PIN_CMP(m2, right_step) || PIN_CMP(m2, left_dir) || PIN_CMP(m2, right_dir) || PIN_CMP(m2, left_fault) || PIN_CMP(m2, right_fault))
        )) ||
        (config->pin_enable != GPIO_NUM_NC && (PIN_CMP(enable, sleep) || PIN_CMP(enable, reset) || PIN_CMP(enable, left_step) || PIN_CMP(enable, right_step) || PIN_CMP(enable, left_dir) || PIN_CMP(enable, right_dir) || PIN_CMP(enable, left_fault) || PIN_CMP(enable, right_fault))) ||
        (config->pin_sleep != GPIO_NUM_NC && (PIN_CMP(sleep, reset) || PIN_CMP(sleep, left_step) || PIN_CMP(sleep, right_step) || PIN_CMP(sleep, left_dir) || PIN_CMP(sleep, right_dir) || PIN_CMP(sleep, left_fault) || PIN_CMP(sleep, right_fault))) ||
        (config->pin_reset != GPIO_NUM_NC && (PIN_CMP(reset, left_step) || PIN_CMP(reset, right_step) || PIN_CMP(reset, left_dir) || PIN_CMP(reset, right_dir) || PIN_CMP(reset, left_fault) || PIN_CMP(reset, right_fault))) ||
        config->pin_left_step == GPIO_NUM_NC ||
        (PIN_CMP(left_step, right_step) || PIN_CMP(left_step, left_dir) || PIN_CMP(left_step, right_dir) || PIN_CMP(left_step, left_fault) || PIN_CMP(left_step, right_fault)) ||
        config->pin_right_step == GPIO_NUM_NC ||
        (PIN_CMP(right_step, left_dir) || PIN_CMP(right_step, right_dir) || PIN_CMP(right_step, left_fault) || PIN_CMP(right_step, right_fault)) ||
        config->pin_left_dir == GPIO_NUM_NC ||
        (PIN_CMP(left_dir, right_dir) || PIN_CMP(left_dir, left_fault) || PIN_CMP(left_dir, right_fault)) ||
        config->pin_right_dir == GPIO_NUM_NC ||
        (PIN_CMP(right_dir, left_fault) || PIN_CMP(right_dir, right_fault)) ||
        config->pin_left_fault == GPIO_NUM_NC ||
        (PIN_CMP(left_fault, right_fault))
    ) {
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    }

    uint32_t substeps_code = 0xFFFFFFFF;
    if(config->pin_m0 != GPIO_NUM_NC) {
        switch(config->microsteps) {
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_1: substeps_code = 0; break;
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_2: substeps_code = 1; break;
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_4: substeps_code = 2; break;
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_8: substeps_code = 3; break;
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_16: substeps_code = 4; break;
            case MotorSubstepsPerStep::MOTOR_SUBSTEPS_32: substeps_code = 5; break;
            default:
                ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
        }
    }

    gpio_config_t gpio_out_cfg = {
        .pin_bit_mask =
            PIN_MASK(m0) | PIN_MASK(m1) | PIN_MASK(m2) | PIN_MASK(enable) | PIN_MASK(sleep) | PIN_MASK(reset) |
            PIN_MASK(left_step) | PIN_MASK(right_step) | PIN_MASK(left_dir) | PIN_MASK(right_dir),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_out_cfg));

    gpio_config_t gpio_in_cfg = {
        .pin_bit_mask = PIN_MASK(left_fault) | PIN_MASK(right_fault),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&gpio_in_cfg));

    gpio_set_level(config->pin_enable, 1);
    gpio_set_level(config->pin_sleep, 0);
    gpio_set_level(config->pin_reset, 0);
    gpio_set_level(config->pin_left_step, 0);
    gpio_set_level(config->pin_right_step, 0);
    gpio_set_level(config->pin_left_dir, 0);
    gpio_set_level(config->pin_right_dir, 0);

    if(substeps_code != 0xFFFFFFFF) {
        gpio_set_level(config->pin_m0, (substeps_code >> 0) & 1);
        gpio_set_level(config->pin_m1, (substeps_code >> 1) & 1);
        gpio_set_level(config->pin_m2, (substeps_code >> 2) & 1);
    }

    this->config = *config;
    this->speed_mode = LEDC_HIGH_SPEED_MODE;
    this->left_timer = LEDC_TIMER_0;
    this->right_timer = LEDC_TIMER_1;
    this->left_channel = LEDC_CHANNEL_0;
    this->right_channel = LEDC_CHANNEL_1;
}

#undef PIN_CMP
#undef PIN_MASK

esp_err_t motors_init_signal(ledc_mode_t speed_mode, ledc_timer_t timer, ledc_channel_t channel, gpio_num_t pin) {
    ledc_timer_config_t ledc_cfg = {
        .speed_mode = speed_mode,
        .duty_resolution = LEDC_TIMER_7_BIT,
        .timer_num = timer,
        .freq_hz = 400,
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&ledc_cfg), LOG_TAG, "Failed to start LEDC timer");

    ledc_channel_config_t channel_cfg = {
        .gpio_num = pin,
        .speed_mode = speed_mode,
        .channel = channel,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer,
        .duty = 1 << 6,
        .hpoint = 0,
        .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags = {
            .output_invert = false,
        },
    };
    esp_err_t result = ESP_ERROR_CHECK_WITHOUT_ABORT(ledc_channel_config(&channel_cfg));
    if(result != ESP_OK) {
        ledc_timer_config_t ledc_dcfg = {
            .speed_mode = speed_mode,
            .timer_num = timer,
            .deconfigure = true,
        };

        ledc_timer_config(&ledc_dcfg);

        return result;
    }

    return ESP_OK;
}

esp_err_t motors_stop_signal(ledc_mode_t speed_mode, ledc_timer_t timer, ledc_channel_t channel) {
    ledc_stop(speed_mode, channel, 0);

    ledc_timer_config_t ledc_dcfg = {
        .speed_mode = speed_mode,
        .timer_num = timer,
        .deconfigure = true,
    };
    ledc_timer_config(&ledc_dcfg);

    return ESP_OK;
}

Motors::~Motors() {
    motors_stop_signal(this->speed_mode, this->left_timer, this->left_channel);
    motors_stop_signal(this->speed_mode, this->right_timer, this->right_channel);
}

esp_err_t Motors::init() {
    ESP_RETURN_ON_ERROR(
        motors_init_signal(this->speed_mode, this->left_timer, this->left_channel, this->config.pin_left_step),
        LOG_TAG, "Error initialising signal for the left motor"
    );

    esp_err_t result = motors_init_signal(
        this->speed_mode, this->right_timer, this->right_channel, this->config.pin_right_step
    );
    if(result != ESP_OK) {
        ESP_LOGE(LOG_TAG, "Error initialising signal for the right motor");

        motors_stop_signal(this->speed_mode, this->left_timer, this->left_channel);
    }

    gpio_set_level(this->config.pin_enable, 0);
    gpio_set_level(this->config.pin_sleep, 1);
    gpio_set_level(this->config.pin_reset, 1);

    return ESP_OK;
}

esp_err_t Motors::set_speed(int32_t left_pps, int32_t right_pps) {
    if(left_pps != 0) {
        ledc_timer_resume(this->speed_mode, this->left_timer);
        if(left_pps < 0) {
            left_pps = -left_pps;
            gpio_set_level(this->config.pin_left_dir, 1);
        } else {
            gpio_set_level(this->config.pin_left_dir, 0);
        }
        ledc_set_freq(this->speed_mode, this->left_timer, left_pps);
    } else {
        ledc_timer_pause(this->speed_mode, this->left_timer);
    }

    if(right_pps != 0) {
        ledc_timer_resume(this->speed_mode, this->right_timer);
        if(right_pps < 0) {
            right_pps = -right_pps;
            gpio_set_level(this->config.pin_right_dir, 1);
        } else {
            gpio_set_level(this->config.pin_right_dir, 0);
        }
        ledc_set_freq(this->speed_mode, this->right_timer, right_pps);
    } else {
        ledc_timer_pause(this->speed_mode, this->right_timer);
    }

    return ESP_OK;
}
