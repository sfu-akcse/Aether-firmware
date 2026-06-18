# Aether Devcontainer Subscriber Setup

This guide uses the existing Aether devcontainer as the shared ROS 2 environment and mounts this repo inside it at:

`/workspace/Aether-firmware`

## What has already been done

The Aether devcontainer config at `/Users/admin/Aether/.devcontainer/devcontainer.json` has been updated to include:

```json
"mounts": [
  "source=/Users/admin/Aether-firmware,target=/workspace/Aether-firmware,type=bind"
]
```

## What you need to do next

### 1. Rebuild the Aether devcontainer

In VS Code with the Aether repo open:

1. Press `Cmd + Shift + P`
2. Run `Dev Containers: Rebuild and Reopen in Container`

This is required so Docker remounts `Aether-firmware` into the container.

### 2. Verify this repo is mounted inside the container

Open a terminal inside the rebuilt Aether devcontainer and run:

```bash
ls /workspace
ls /workspace/Aether-firmware
```

You should see this repo and the `hand_xyz_subscriber_pkg` folder.

### 3. Check the ROS domain used by the publisher

Inside the same container, run:

```bash
echo $ROS_DOMAIN_ID
```

If this prints nothing, ROS 2 typically uses domain `0`.

### 4. Create a small workspace for this repo's subscriber package

Inside the container:

```bash
source /opt/ros/humble/setup.zsh
mkdir -p /workspace/Aether-firmware_ws/src
ln -sfn /workspace/Aether-firmware/hand_xyz_subscriber_pkg /workspace/Aether-firmware_ws/src/hand_xyz_subscriber_pkg
```

### 5. Build the subscriber package

Inside the container:

```bash
source /opt/ros/humble/setup.zsh
cd /workspace/Aether-firmware_ws
colcon build --packages-select hand_xyz_subscriber_pkg
```

### 6. Run the publisher

In one container terminal, run the publisher from the Aether workspace using whatever command already works in that repo.

If your publisher package lives in `/workspace/ros2_ws`, the usual pattern is:

```bash
source /opt/ros/humble/setup.zsh
source /workspace/ros2_ws/install/setup.zsh
ros2 run random_xyz_stream random_xyz_publisher
```

### 7. Run the subscriber from this repo

In a second container terminal:

```bash
source /opt/ros/humble/setup.zsh
source /workspace/Aether-firmware_ws/install/setup.zsh
ros2 run hand_xyz_subscriber_pkg hand_xyz_subscriber
```

### 8. Verify the topic before running the subscriber if needed

From the container:

```bash
source /opt/ros/humble/setup.zsh
source /workspace/ros2_ws/install/setup.zsh
ros2 topic list
ros2 topic info /hand_xyz
ros2 topic echo /hand_xyz
```

## What you do not need

- You do not need to install `colcon` on your Mac host for this flow.
- You do not need a second Docker setup for this repo if you are using the shared devcontainer approach.
- You do not need to run ROS 2 natively on macOS for this setup.

## If something fails

Check these first:

- the devcontainer was rebuilt after the mount change
- `/workspace/Aether-firmware` exists inside the container
- `colcon` works inside the container
- the publisher and subscriber are both using ROS 2 Humble
- `/hand_xyz` exists and is `geometry_msgs/msg/Point`
