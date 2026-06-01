# Aether-firmware

Welcome to the firmware repository for the Aether Robotic Arm. This codebase manages the ROS 2 environment, bridging high-level computer vision telemetry with low-level hardware actuation over TCP/IP sockets.

## ⚠️ System Prerequisites

**CRITICAL:** ROS 2 distributions are strictly tied to explicit Ubuntu versions. 
* You **MUST** use **Ubuntu 24.04 (Noble)** for this project. 
* Do not attempt to use Debian or older Ubuntu releases like 22.04.
* If you are on Windows, ensure your WSL instance is specifically running Ubuntu 24.04 (`wsl --install -d Ubuntu-24.04`).

---

## Phase 1: Install ROS 2 Kilted Kaiju

Run the following commands in your Ubuntu 24.04 terminal to securely add the official ROS 2 repositories and install the pre-compiled binary desktop environment.

**1. Add the ROS 2 Keys & Repository:**
```bash
sudo apt update
sudo apt install curl -y
sudo curl -sSL [https://raw.githubusercontent.com/ros/rosdistro/master/ros.key](https://raw.githubusercontent.com/ros/rosdistro/master/ros.key) -o /usr/share/keyrings/ros-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] [http://packages.ros.org/ros2/ubuntu](http://packages.ros.org/ros2/ubuntu) $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
```

**2. Install the ROS 2 Binaries:**
```bash
sudo apt update
sudo apt install ros-kilted-desktop -y
```

**3. Configure Your Environment:**
To avoid manually sourcing ROS 2 every time you open a terminal, add it to your bash startup script:
```bash
echo "source /opt/ros/kilted/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

**4. Initialize Dependencies (rosdep):**
```bash
sudo apt install python3-rosdep -y
sudo rosdep init
rosdep update
```

## Phase 2: Building the Workspace

Once ROS 2 is installed, clone this repository and build the custom packages.
```bash
# Clone the repository
git clone [https://github.com/YourOrg/Aether-firmware.git](https://github.com/YourOrg/Aether-firmware.git)
cd Aether-firmware

# Install missing C++ dependencies
rosdep install --from-paths src --ignore-src -y

# Build the workspace
colcon build
```

## Phase 3: Testing the Sample Action Code
To verify your installation, we will run the sample_action_cpp components. This requires opening two separate terminal tabs to simulate a decentralized network.

Terminal 1 (Action Server):
Open a new terminal tab, navigate to the workspace, and start the server.
```bash
cd ~/Aether-firmware
source install/setup.bash
ros2 run sample_action_cpp sample_action_server
```

Terminal 2 (Action Client):
Open a second terminal tab, navigate to the workspace, and start the client. You should immediately see the nodes communicating.
```bash
cd ~/Aether-firmware
source install/setup.bash
ros2 run sample_action_cpp sample_action_client
```
