#include <driver/gpio.h>
#include <driver/ledc.h>

enum class MotorSubstepsPerStep {
    MOTOR_SUBSTEPS_1 = 0,
    MOTOR_SUBSTEPS_2,
    MOTOR_SUBSTEPS_4,
    MOTOR_SUBSTEPS_8,
    MOTOR_SUBSTEPS_16,
    MOTOR_SUBSTEPS_32,
};

typedef struct {
    gpio_num_t pin_m0;
    gpio_num_t pin_m1;
    gpio_num_t pin_m2;
    gpio_num_t pin_enable;
    gpio_num_t pin_sleep;
    gpio_num_t pin_reset;
    gpio_num_t pin_left_step;
    gpio_num_t pin_right_step;
    gpio_num_t pin_left_dir;
    gpio_num_t pin_right_dir;
    gpio_num_t pin_left_fault;
    gpio_num_t pin_right_fault;
    MotorSubstepsPerStep microsteps;
} motors_config_t;

class Motors final {
    private:
        motors_config_t config;
        ledc_mode_t speed_mode;
        ledc_timer_t left_timer;
        ledc_timer_t right_timer;
        ledc_channel_t left_channel;
        ledc_channel_t right_channel;

    public:
        /**
         * @brief Motors configuration
         * 
         * Configure motors control with the pins and microstepping setting.
         * 
         * @param[in] config Pointer to the configuration struct
         * @param[out] handle_ptr Motors handle
         * 
         * @return
         *     - ESP_OK: Motors configured successfully
         *     - ESP_ERR_INVALID_ARG: Motors configuration failed because of invalid argument.
         */
        Motors(const motors_config_t *config);

        /**
         * @brief Destroy motors control
         * 
         * @param handle Motors control to destroy
         * 
         * @return
         *     - ESP_OK: Motors control destroyed successfully
         */
        ~Motors();

        /**
         * @brief Initialise motors control
         * 
         * The motors control starts driving the motors, the motors will lock (speed = 0).
         * 
         * @return
         *     - ESP_OK: motors initialised successfully
         *     - ESP_ERR_INVALID_ARG: Parameter error
         *     - ESP_FAIL: Out of supported frequency range
         */
        esp_err_t init();

        /**
         * @brief Set the speed of both wheels
         * 
         * @param pps_left Left motor speed in pulses per second
         * @param pps_right Right motor speed in pulses per second
         * 
         * @return
         *     - ESP_OK:
         */
        esp_err_t set_speed(int32_t pps_left, int32_t pps_right);
};
