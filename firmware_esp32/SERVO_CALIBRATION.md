# Aether ESP32 Servo Calibration

## Hardware

- ESP32 development board
- Waveshare Bus Servo Adapter (A)
- 2x Feetech ST3215 servos
- 12 V external barrel charger

## UART Wiring

| ESP32 | Waveshare |
|---|---|
| D19 / GPIO19 | TX |
| D18 / GPIO18 | RX |
| GND | GND |

- Waveshare jumpers: A / A
- Servo baud rate: 1,000,000

## Servo IDs

- ID 1: Shoulder
- ID 2: Elbow

## Initial Neutral Reference

Initial software neutral:

- Shoulder: 2047
- Elbow: 2047

After initial physical arm assembly, approximate readback:

- Shoulder: 2042
- Elbow: 2048

The arm was about straight and horizontal at this initial reference pose.Small readback differences from commanded positions were observed during my testing.

### Shoulder

Observed:

- Increasing servo count moves the shoulder upward.
- Decreasing servo count moves the shoulder downward.

Therefore:

```text
SHOULDER_DIRECTION = +1
```

### Elbow

Observed:

- Increasing servo count bends the forearm inward toward the upper-arm link.
- Decreasing servo count bends the forearm outward away from the upper-arm link.

Therefore:

```text
ELBOW_DIRECTION = -1
```

## Final Physical Calibration

### Shoulder

- Horizontal / 0-degree command center: approximately 2030
- Horizontal readback: approximately 2025
- Vertical / 90-degree command: approximately 3034
- Maximum observed servo position: approximately 4094
- At approximately 4094, the arm is close to the table on the opposite side.

Recommended software values for range:

```text
SHOULDER_CENTER = 2030
SHOULDER_DIRECTION = +1
SHOULDER_MIN = 2050
SHOULDER_MAX = 4000
```

`SHOULDER_MIN` and `SHOULDER_MAX` include a small safety margin from the tested physical limits.

### Elbow

- Straight / 0-degree command center: approximately 2085
- Straight readback: approximately 2084
- Outward tested boundary: command 900, readback approximately 909
- Inward tested boundary: command 3200, readback approximately 3197

Recommended software values for range:

```text
ELBOW_CENTER = 2085
ELBOW_DIRECTION = -1
ELBOW_MIN = 950
ELBOW_MAX = 3150
```

`ELBOW_MIN` and `ELBOW_MAX` include a small safety margin from the tested physical limits.

## Current Calibration Constants

```text
SHOULDER_CENTER = 2030
SHOULDER_DIRECTION = +1
SHOULDER_MIN = 2050
SHOULDER_MAX = 4000

ELBOW_CENTER = 2085
ELBOW_DIRECTION = -1
ELBOW_MIN = 950
ELBOW_MAX = 3150
```

## Mechanical Notes

- The prototype was initially assembled approximately straight and horizontal.
- The shoulder can rotate from approximately horizontal, through vertical, and close to horizontal on the opposite side.
- Near the maximum shoulder position, the arm approaches the work surface.
- The elbow can bend in both directions around its straight reference position.
- Do not use the absolute tested physical boundaries as normal software limits.
- Keep safety margins to avoid collisions, cable strain, and mechanical binding.