# Subscriber Docker Setup

This folder gives this repo its own ROS 2 Humble Docker setup so it can subscribe to the publisher running in your other repo.

## What this does

- builds a dedicated subscriber container for this repo
- joins an external Docker network named `aether_ros_net`
- builds the standalone package at `/workspace/hand_xyz_subscriber_pkg`
- runs `hand_xyz_subscriber_pkg` inside the container

## Why this helps on macOS

Your publisher already runs inside Docker. On macOS, ROS 2 discovery is usually much easier when both publisher and subscriber run inside Linux containers on the same Docker network, instead of trying to bridge ROS 2 DDS between Docker and the Mac host shell.

## One-time network setup

Create the shared Docker network once:

```bash
docker network create aether_ros_net
```

If the command says the network already exists, that is fine.

## Make sure the publisher container is on the same network

If the publisher container is already running, connect it:

```bash
docker network connect aether_ros_net <publisher-container-name>
```

To find the container name:

```bash
docker ps --format '{{.Names}}'
```

If the publisher repo is started by Compose or a dev container, you only need that running container attached to `aether_ros_net`.

## Start this repo's subscriber container

From this repo:

```bash
cd /Users/admin/Aether-firmware/subscriber_docker_setup
docker compose up -d --build
```

## Build the subscriber package

```bash
docker compose exec aether-subscriber bash -lc "/workspace/subscriber_docker_setup/scripts/build_subscriber.sh"
```

## Run the subscriber

```bash
docker compose exec aether-subscriber bash -lc "/workspace/subscriber_docker_setup/scripts/run_subscriber.sh"
```

## Optional: set a different ROS domain

Both publisher and subscriber need the same `ROS_DOMAIN_ID`.

For this repo's container:

```bash
cd /Users/admin/Aether-firmware/subscriber_docker_setup
ROS_DOMAIN_ID=42 docker compose up -d --build
```

If the publisher uses a different domain, match that exact number here.

## Check whether the topic is visible

```bash
docker compose exec aether-subscriber bash -lc "source /opt/ros/humble/setup.bash && ros2 topic list"
docker compose exec aether-subscriber bash -lc "source /opt/ros/humble/setup.bash && ros2 topic info /hand_xyz"
docker compose exec aether-subscriber bash -lc "source /opt/ros/humble/setup.bash && ros2 topic echo /hand_xyz"
```

## If discovery still does not work

Check these first:

- both containers are attached to `aether_ros_net`
- both sides use ROS 2 Humble
- both sides use the same `ROS_DOMAIN_ID`
- the publisher is actually publishing `/hand_xyz`
- the publisher uses `geometry_msgs/msg/Point`

Useful inspection commands:

```bash
docker network inspect aether_ros_net
docker compose ps
docker compose logs
```
