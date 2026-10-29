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
