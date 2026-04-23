#include <cstddef>
#include <cstdint>

#include <driver/gpio.h>

enum class CarLongitudinalMovement { AHEAD, STOPPED, BEHIND };
enum class CarHorizontalAlignment { LEFT, CENTER, RIGHT };
enum class CarLedState { ON, OFF };

class CarControl {
    public:
        CarControl();

        bool setup();

        /**
         * Instructs the car to go ahead. It will keep going forward until `go_backwards`or `stop` is called.
         */
        void go_forward();

        /**
         * Instructs the car to go backwards. It will keep going backwards until `go_forward` or `stop` is called.
         */
        void go_backwards();

        /**
         * Turn off tyre traction.
         */
        void stop();

        /**
         * Make the front wheels turn left
         */
        void point_left();

        /**
         * Make the front wheels straight
         */
        void point_ahead();

        /**
         * Make the front wheels turn right
         */
        void point_right();

        void lights_on();

        void lights_off();

        void toggle_lights();
    
    private:
        gpio_num_t left_motor_a_pin, left_motor_b_pin, left_motor_speed_pin;
        gpio_num_t right_motor_a_pin, right_motor_b_pin, right_motor_speed_pin;
        gpio_num_t led_a_pin, led_b_pin;
        CarLongitudinalMovement movement;
        CarHorizontalAlignment alignment;
        CarLedState led_state;

        void update_pins();
};
