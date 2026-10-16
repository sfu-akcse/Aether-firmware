#ifndef ARM_CONTROLLER_H
#define ARM_CONTROLLER_H


#include "driver/uart.h"
#include "motor_control.h"


class ArmController
{
public:
    ArmController();

    /*
     * Initialize the UART servo bus, detect both servos,
     * and enable torque.
     */
    bool begin();


    /*
     * Move directly using calibrated ST3215 positions.
     *
     * Returns false if either target is outside
     * the configured safe range.
     */
    bool moveToPositions(
        int shoulderPosition,
        int elbowPosition
    );


    /*
     * Move using theoretical joint angles in degrees.
     *
     * Shoulder:
     *      0 deg = horizontal
     *      positive = upward / toward opposite side
     *
     * Elbow:
     *      0 deg = straight
     *      positive = outward bend
     *      negative = inward bend
     */
    bool moveToAngles(
        float shoulderDegrees,
        float elbowDegrees
    );


    /*
     * Move to the known safe vertical parked pose.
     */
    bool park();


    /*
     * Read current servo encoder positions.
     */
    int getShoulderPosition();
    int getElbowPosition();


    /*
     * Check whether raw servo positions are inside
     * our calibrated software safety limits.
     */
    bool positionsAreSafe(
        int shoulderPosition,
        int elbowPosition
    );


private:
    SMS_STS bus;


    // --------------------------------------------------
    // UART configuration
    // --------------------------------------------------

    static constexpr uart_port_t SERVO_UART =
        UART_NUM_1;

    static constexpr int SERVO_TX_PIN = 19;
    static constexpr int SERVO_RX_PIN = 18;


    // --------------------------------------------------
    // Servo IDs
    // --------------------------------------------------

    static constexpr int SHOULDER_ID = 1;
    static constexpr int ELBOW_ID = 2;


    // --------------------------------------------------
    // ST3215 conversion
    // --------------------------------------------------

    static constexpr float COUNTS_PER_DEGREE =
        4096.0f / 360.0f;


    // --------------------------------------------------
    // Physical calibration
    // --------------------------------------------------

    static constexpr int SHOULDER_CENTER = 2030;
    static constexpr int SHOULDER_DIRECTION = +1;

    static constexpr int SHOULDER_MIN = 2030;
    static constexpr int SHOULDER_MAX = 4000;


    static constexpr int ELBOW_CENTER = 2085;
    static constexpr int ELBOW_DIRECTION = -1;

    static constexpr int ELBOW_MIN = 950;
    static constexpr int ELBOW_MAX = 3150;


    // --------------------------------------------------
    // Park pose
    // --------------------------------------------------

    static constexpr int SHOULDER_PARK = 3034;
    static constexpr int ELBOW_PARK = 2085;


    // --------------------------------------------------
    // Motion settings
    // --------------------------------------------------

    static constexpr int SPEED = 120;
    static constexpr int ACCELERATION = 15;


    // --------------------------------------------------
    // Arrival checking
    // --------------------------------------------------

    static constexpr int POSITION_TOLERANCE = 10;
    static constexpr int POLL_INTERVAL_MS = 100;
    static constexpr int MOVEMENT_TIMEOUT_MS = 20000;


    /*
     * Wait until one servo reaches a target.
     */
    bool waitForPosition(
        int id,
        int target,
        const char *name
    );


    /*
     * Convert joint angle -> calibrated servo position.
     */
    int shoulderAngleToPosition(
        float degrees
    );

    int elbowAngleToPosition(
        float degrees
    );
};


#endif