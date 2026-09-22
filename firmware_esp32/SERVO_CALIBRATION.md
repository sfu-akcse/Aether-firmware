# Aether ESP32 Servo Calibration

## Hardware

- ESP32 development board
- Waveshare Bus Servo Adapter (A)
- 2x Feetech ST3215 servos
- 12 V external servo supply

## UART Wiring

| ESP32 | Waveshare |
|------|-----------|
| D19 / GPIO19 | TX |
| D18 / GPIO18 | RX |
| GND | GND |

- Waveshare jumpers: A / A
- Servo baud rate: 1,000,000

## Servo IDs

- ID 1: Shoulder
- ID 2: Elbow

## Neutral Reference

Nominal software neutral:

- Shoulder: 2047
- Elbow: 2047

After physical arm assembly, approximate neutral readback:

- Shoulder: 2042
- Elbow: 2048

The arm is approximately straight and horizontal at this reference pose.

Small readback differences around 2047 are expected and are not currently
being treated as calibration offsets.

## Direction Test

Test sequence:

- 2047 -> 2077
- 2077 -> 2047
- 2047 -> 2017
- 2017 -> 2047

### Shoulder

Observed:

- Increasing servo count moves shoulder UP.
- Decreasing servo count moves shoulder DOWN.

Therefore:

SHOULDER_DIRECTION = +1

### Elbow

Observed:

- Increasing servo count moves forearm DOWN.
- Decreasing servo count moves forearm UP.

Therefore:

ELBOW_DIRECTION = -1

## Current Calibration Constants

SHOULDER_CENTER = 2047
SHOULDER_DIRECTION = +1

ELBOW_CENTER = 2047
ELBOW_DIRECTION = -1

## Mechanical Notes

The prototype currently starts approximately horizontal.

When sitting directly on the work surface, decreasing the shoulder position
too far below 2047 can cause the arm to collide with the surface.