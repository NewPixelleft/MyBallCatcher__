# MyBallCatcher__
Ball catcher game via ROS2 and OpenCV. Basically, you are expected to open your webcam, and pick a specified coloured object (default is blue), which could be edited in colourConfig.txt. Then, you are expected to hold the coloured object inside a circle for an amount of seconds (default is 2), which you would be rewarded by 1 point(s).

## How to Run
To run this project, you would need a C++ compiler (G++/Clang++), ROS 2 Humble, and OpenCV installed in your system. If you have fulfilled all of the 3 basic requirement, skip this step
```bash
sudo apt update
sudo apt install -y \
    git \
    build-essential \
    python3-colcon-common-extensions \
    python3-rosdep
```
Then, clone the repository 
```bash
git clone https://github.com/NewPixelleft/MyBallCatcher__.git
cd MyBallCatcher__
```
Once you have cloned the repo, run this command get these required dependencies
```bash
source /opt/ros/humble/setup.bash

rosdep install \
    --from-paths bouncing_ball_catcher \
    --ignore-src \
    -r -y
```
### Building the Project
First and foremost, build the project using `colcon`
```bash
colcon build --symlink-install
```
Then, install the project 
```bash
source install/setup.bash
```
### Running the Project
In running this project, you would need 2 different terminals. **Remember** to make sure that both terminals are running in the same project directory
#### Terminal 1
```bash
source ~/opt/ros/humble/setup.bash
source install/setup.bash

ros2 run bouncing_ball_catcher camera_node
```
#### Terminal 2
```bash
source ~/opt/ros/humble/setup.bash
source install/setup.bash

ros2 run bouncing_ball_catcher game_node
```

## Core Architechture
### Node
The overall project runs on 2 different nodes: `camera_node` and `game_node`. The camera node takes the image data from your webcam, and in the future, we might be able to insert files in it. The game mode takes the overall labour of running the game logic. The game node takes the topic `/camera/image_raw` from the camera node with the type of `_msgs/msg/Image`.
### Topics
### Topic

| Topic | Type  | Purpose |
|---|---|---|
|/camera/image_raw | sensor_msgs/msg/Image | Sending the required imagery to `game_node`|
|/game/score | std_msgs/msg/Int32 | Giving player score info as a 32 bit integer |
/game/debug_image | sensor_msgs/msg/Image | Imagery data for debugging purposes|

### .txt Parameters
The project has 2 `.txt` parameters: `colourConfig.txt` and `gameConfig.txt`. In `colourConfig.txt`, you can freely add, change, and pick HSV Qualifiers. For example, if you want to add and use yellow, you can type something like  
```txt
pick col_BLUE

col_YELLOW
HSV_MIN 40 80 140
HSV_MAX 75 255 255
```
here, `pick` is an operator to determine that we are picking a specific colour. `HSV_MIN` and `HSV_MAX` determine the floor and ceil of the HSV range. In OpenCV configuration, Hue accepts 0-180 integer values, while saturation and value accept 0-255.
`gameConfig.txt` determines how the game would run. 
```txt
# Main game parameters
# ========================================
MAINCFG WIDTH 640
MAINCFG HEIGHT 480

# Game variables
# ========================================
var BALLCOUNT 3
var BALLSPEED 150

# Ball radius:
# Specify an integer for a manual radius.
# Use CALCULATED to calculate it automatically.

var BALLRADIUS CALCULATED

# Examples:
# var BALLRADIUS 100
# var BALLRADIUS CALCULATED

# ========================================
# Game rules
# 0 = FALSE
# 1 = TRUE
# ========================================
rul BALLCOLLISION 1
rul PROGRESSIVE_SPEED 1
```
On both scripts, the `#` char denotes a line of comment
