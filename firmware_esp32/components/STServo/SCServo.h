/**
 * @file SCServo.h
 * @brief Master include file for Feetech Serial Servo SDK
 *
 * @details This is the main header file that includes all servo series interfaces.
 * Include this single file to access all Feetech servo series classes.
 *
 * **Supported Servo Series:**
 * - SMS_STS: SMS and STS series (3 operating modes: servo, wheel closed-loop, wheel open-loop)
 * - SCSCL: SCSCL series (position control and PWM mode)
 * - HLSCL: HLS series (servo, wheel, and force control modes)
 * - SCS0009: Servos used by the AmazingHand v1 servos
 *
 * **Usage:**
 * @code
 * #include "SCServo.h"
 *
 * SMS_STS servo;
 *
 * if (!servo.begin(1000000, UART_NUM_1, SERVO_TX_PIN, SERVO_RX_PIN)) {
 *     return;
 * }
 *
 * servo.InitMotor(1, 0, 1);
 * servo.WritePosEx(1, 2048, 1000, 50);
 * @endcode
 *
 * @note Implementation is built through the STServo ESP-IDF component
 * @see SMS_STS.h for protocol documentation
 */

#ifndef _SCSERVO_H
#define _SCSERVO_H

// #include "SCSCL.h"
#include "SMS_STS.h"
// #include "HLSCL.h"
// #include "SCS0009.h"
/* additional support for SCS0009 from Amazing Hand */
#endif