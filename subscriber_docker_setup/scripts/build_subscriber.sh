#!/usr/bin/env bash
set -euo pipefail

source /opt/ros/humble/setup.bash

mkdir -p /subscriber_ws/src
rm -rf /subscriber_ws/src/hand_xyz_subscriber_pkg
ln -s /workspace/hand_xyz_subscriber_pkg /subscriber_ws/src/hand_xyz_subscriber_pkg

cd /subscriber_ws
colcon build --packages-select hand_xyz_subscriber_pkg
