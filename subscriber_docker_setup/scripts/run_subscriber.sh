#!/usr/bin/env bash
set -euo pipefail

source /opt/ros/humble/setup.bash
source /subscriber_ws/install/setup.bash

ros2 run hand_xyz_subscriber_pkg hand_xyz_subscriber "$@"
