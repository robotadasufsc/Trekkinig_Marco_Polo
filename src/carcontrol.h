#include <cstdint>

#include <driver/gpio.h>
#include "motors.h"

enum class CarLongitudinalMovement { AHEAD, STOPPED, BEHIND };
enum class CarHorizontalAlignment { LEFT, CENTER, RIGHT };

class CarControl final {
    public:
        CarControl(const motors_config_t *config);

        bool reset();

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

        void update_signals();
    
    private:
        Motors motor_control;
        int32_t max_speed;
        int32_t speed_left, speed_right;
        int32_t target_speed_left, target_speed_right;
        CarLongitudinalMovement longitudinal_speed;
        CarHorizontalAlignment alignment;

        void update_targets();
};
