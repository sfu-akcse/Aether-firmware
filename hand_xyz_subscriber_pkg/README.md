# hand_xyz_subscriber_pkg

This is a standalone ROS 2 C++ package you can place in any ROS 2 workspace as the subscriber side for `/hand_xyz`.

## What it does

- Subscribes to `/hand_xyz`
- Expects `geometry_msgs/msg/Point`
- Logs incoming values as JSON like `{"x":1.0,"y":2.0,"z":3.0}`

## Where to put it

Copy or move `hand_xyz_subscriber_pkg` into the `src/` folder of the ROS 2 workspace where you want the subscriber to live.

Example:

```bash
cd /path/to/your_ros2_workspace/src
cp -R /Users/admin/Aether-firmware/hand_xyz_subscriber_pkg .
```

## Build and run

You need a working ROS 2 install and `colcon`.

```bash
cd /path/to/your_ros2_workspace
colcon build --packages-select hand_xyz_subscriber_pkg
source install/setup.bash
ros2 run hand_xyz_subscriber_pkg hand_xyz_subscriber
```

If you want a different topic name:

```bash
ros2 run hand_xyz_subscriber_pkg hand_xyz_subscriber --ros-args -p topic_name:=/hand_xyz
```

## Verify the publisher is visible

In the same environment where you sourced ROS 2:

```bash
ros2 topic list
ros2 topic info /hand_xyz
ros2 topic echo /hand_xyz
```

If the publisher is running in another workspace, both terminals need compatible ROS 2 environments and the same `ROS_DOMAIN_ID` if that variable is set.

## macOS notes

Docker is not required for this subscriber. Docker only matters if your ROS setup depends on containers. For a normal ROS 2 publisher/subscriber connection, what matters is:

- ROS 2 is installed
- `colcon` is installed
- both sides use compatible ROS 2 distros
- both sides are on the same network / discovery domain

If `colcon` is missing, install it before building this package.
