#include "carcontrol.h"
#include <driver/ledc.h>
#include "esp_log.h"

#define LEDC_SPEED_MODE LEDC_HIGH_SPEED_MODE
#define MOTOR_A_LEDC_CHANNEL LEDC_CHANNEL_0
#define MOTOR_B_LEDC_CHANNEL LEDC_CHANNEL_1

#define LOG_TAG "CARCTL"

#define ACCEL 10

CarControl::CarControl(const motors_config_t *config) :
    motor_control(config), max_speed(200), speed_left(0), speed_right(0), target_speed_left(0), target_speed_right(0),
    longitudinal_speed(CarLongitudinalMovement::AHEAD), alignment(CarHorizontalAlignment::CENTER)
{
    ESP_ERROR_CHECK(motor_control.init());
}

bool CarControl::reset() {
    this->longitudinal_speed = CarLongitudinalMovement::STOPPED;
    this->alignment = CarHorizontalAlignment::CENTER;

    this->update_signals();

    return true;
}

void CarControl::go_forward() {
    ESP_LOGI(LOG_TAG, "ahead");

    this->longitudinal_speed = CarLongitudinalMovement::AHEAD;
    this->update_targets();
}

void CarControl::go_backwards() {
    ESP_LOGI(LOG_TAG, "back");

    this->longitudinal_speed = CarLongitudinalMovement::BEHIND;
    this->update_targets();
}

void CarControl::stop() {
    ESP_LOGI(LOG_TAG, "stop");

    this->longitudinal_speed = CarLongitudinalMovement::STOPPED;
    this->update_targets();
}

void CarControl::point_left() {
    ESP_LOGI(LOG_TAG, "left");

    this->alignment = CarHorizontalAlignment::LEFT;
    this->update_targets();
}

void CarControl::point_ahead() {
    ESP_LOGI(LOG_TAG, "middle");

    this->alignment = CarHorizontalAlignment::CENTER;
    this->update_targets();
}

void CarControl::point_right() {
    ESP_LOGI(LOG_TAG, "right");

    this->alignment = CarHorizontalAlignment::RIGHT;
    this->update_targets();
}

void CarControl::update_signals() {
    int tl = this->target_speed_left, tr = this->target_speed_right;

    if(tl == speed_left && tr == speed_right) return;

    if(this->speed_left > tl) {
        speed_left -= ACCEL;
        if(speed_left < tl) speed_left = tl;
    }

    if(this->speed_right > tr) {
        speed_right -= ACCEL;
        if(speed_right < tr) speed_right = tr;
    }

    motor_control.set_speed(speed_left, speed_right);
}


void CarControl::update_targets() {
    int l = 0, r = 0;

    switch(this->longitudinal_speed) {
        case CarLongitudinalMovement::STOPPED:
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = -max_speed / 4;
                    r = max_speed / 4;
                    break;
                case CarHorizontalAlignment::CENTER:
                    l = 0;
                    r = 0;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    l = max_speed;
                    r = -max_speed;
                    break;
            }
            break;
        case CarLongitudinalMovement::AHEAD:
            l = max_speed;
            r = max_speed;
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = max_speed / 2;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    r = max_speed / 2;
                    break;
                    default:
                    break;
            }
            break;
        case CarLongitudinalMovement::BEHIND:
            l = -max_speed;
            r = -max_speed;
            switch(this->alignment) {
                case CarHorizontalAlignment::LEFT:
                    l = -max_speed / 2;
                    break;
                case CarHorizontalAlignment::RIGHT:
                    r = -max_speed / 2;
                    break;
                    default:
                    break;
            }
    }

    this->target_speed_left = l;
    this->target_speed_right = r;
}
