#include "carcontrol.h"

#include <cmath>

#include "esp_log_pollyfill.h"
#include <driver/ledc.h>

#define LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_A_LEDC_CHANNEL LEDC_CHANNEL_0
#define MOTOR_B_LEDC_CHANNEL LEDC_CHANNEL_1

#define LOG_TAG "CARCTL"

CarControl::CarControl() :
#if 0
        left_motor_a_pin(GPIO_NUM_15), left_motor_b_pin(GPIO_NUM_2), left_motor_speed_pin(GPIO_NUM_4),
        right_motor_a_pin(GPIO_NUM_17), right_motor_b_pin(GPIO_NUM_5), right_motor_speed_pin(GPIO_NUM_18),
        led_a_pin(GPIO_NUM_16), led_b_pin(GPIO_NUM_19),
        movement(CarLongitudinalMovement::STOPPED), alignment(CarHorizontalAlignment::CENTER), led_state(CarLedState::OFF)
#else
        left_motor_a_pin(GPIO_NUM_2), left_motor_b_pin(GPIO_NUM_4), left_motor_speed_pin(GPIO_NUM_16),
        right_motor_a_pin(GPIO_NUM_27), right_motor_b_pin(GPIO_NUM_26), right_motor_speed_pin(GPIO_NUM_25),
        led_a_pin(GPIO_NUM_33), led_b_pin(GPIO_NUM_32),
        left_motor_pwm(0), right_motor_pwm(0), left_motor_pwm_target(0), right_motor_pwm_target(0),
        longitudinal_speed(CarLongitudinalMovement::STOPPED), alignment(CarHorizontalAlignment::CENTER), led_state(CarLedState::OFF)
#endif
{
    gpio_set_direction(left_motor_a_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(left_motor_b_pin, GPIO_MODE_OUTPUT);
    // gpio_set_direction(left_motor_speed_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(right_motor_a_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(right_motor_b_pin, GPIO_MODE_OUTPUT);
    // gpio_set_direction(right_motor_speed_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(led_a_pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(led_b_pin, GPIO_MODE_OUTPUT);

    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t motor_a_config = {
        .gpio_num = left_motor_speed_pin,
        .speed_mode = LEDC_SPEED_MODE,
        .channel = MOTOR_A_LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 128,
    };
    ledc_channel_config_t motor_b_config = {
        .gpio_num = right_motor_speed_pin,
        .speed_mode = LEDC_SPEED_MODE,
        .channel = MOTOR_B_LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER_0,
        .duty = 128,
    };

    ledc_channel_config(&motor_a_config);
    ledc_channel_config(&motor_b_config);
    ledc_fade_func_install(0);
}

bool CarControl::reset() {
    //this->left_motor_pwm = 0;
    //this->right_motor_pwm = 0;
    this->left_motor_pwm_target = 0;
    this->right_motor_pwm_target = 0;
    this->longitudinal_speed = CarLongitudinalMovement::STOPPED;
    this->alignment = CarHorizontalAlignment::CENTER;
    this->led_state = CarLedState::OFF;

    this->update_pins();

    return true;
}

void CarControl::go_forward() {
    ESP_LOGI_(LOG_TAG, "ahead");

    this->longitudinal_speed = CarLongitudinalMovement::AHEAD;
}

void CarControl::go_backwards() {
    ESP_LOGI_(LOG_TAG, "back");

    this->longitudinal_speed = CarLongitudinalMovement::BEHIND;
}

void CarControl::stop() {
    ESP_LOGI_(LOG_TAG, "stop");

    this->longitudinal_speed = CarLongitudinalMovement::STOPPED;
}

void CarControl::point_left() {
    ESP_LOGI_(LOG_TAG, "left");

    this->alignment = CarHorizontalAlignment::LEFT;
}

void CarControl::point_ahead() {
    ESP_LOGI_(LOG_TAG, "middle");

    this->alignment = CarHorizontalAlignment::CENTER;
}

void CarControl::point_right() {
    ESP_LOGI_(LOG_TAG, "right");

    this->alignment = CarHorizontalAlignment::RIGHT;
}

void CarControl::update_pins() {
    int l, r;

    switch(this->longitudinal_speed) {
        case CarLongitudinalMovement::STOPPED:
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = -255;
                    r = 255;
                    break;
                case CarHorizontalAlignment::CENTER:
                    l = 0;
                    r = 0;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    l = 255;
                    r = -255;
                    break;
            }
            break;
        case CarLongitudinalMovement::AHEAD:
            l = 255;
            r = 255;
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = 128;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    r = 128;
                    break;
            }
            break;
        case CarLongitudinalMovement::BEHIND:
            l = -255;
            r = -255;
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = -128;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    r = -128;
                    break;
            }
    }

    this->left_motor_pwm_target = l;
    this->right_motor_pwm_target = r;

    if(this->left_motor_pwm < this->left_motor_pwm_target)
        this->left_motor_pwm++;
    else if(this->left_motor_pwm > this->left_motor_pwm_target)
        this->left_motor_pwm--;

    if(this->right_motor_pwm < this->right_motor_pwm_target)
        this->right_motor_pwm++;
    else if(this->right_motor_pwm > this->right_motor_pwm_target)
        this->right_motor_pwm--;

    gpio_set_level(this->left_motor_a_pin, this->left_motor_pwm > 0 ? 1 : 0);
    gpio_set_level(this->left_motor_b_pin, this->left_motor_pwm < 0 ? 1 : 0);
    //gpio_set_level(this->left_motor_speed_pin, this->left_motor_pwm != 0 ? 1 : 0);
    if(ledc_set_duty_and_update(LEDC_SPEED_MODE, MOTOR_A_LEDC_CHANNEL, std::abs(this->left_motor_pwm), 0) != ESP_OK) {
        ESP_LOGI_(LOG_TAG, "Bad");
        ESP_LOGI_(LOG_TAG, std::abs(this->left_motor_pwm));
    }
    gpio_set_level(this->right_motor_a_pin, right_motor_pwm < 0 ? 1 : 0);
    gpio_set_level(this->right_motor_b_pin, right_motor_pwm > 0 ? 1 : 0);
    //gpio_set_level(this->right_motor_speed_pin, right_motor_pwm != 0 ? 1 : 0);
    ledc_set_duty_and_update(LEDC_SPEED_MODE, MOTOR_B_LEDC_CHANNEL, std::abs(this->right_motor_pwm), 0);

    gpio_set_level(this->led_a_pin, this->led_state == CarLedState::ON ? 1 : 0);
}

void CarControl::lights_on() {
    this->led_state = CarLedState::ON;
}

void CarControl::lights_off() {
    this->led_state = CarLedState::OFF;
}
