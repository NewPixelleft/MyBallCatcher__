cmake_minimum_required(VERSION 3.8)
project(bouncing_ball_catcher)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
#find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)
#find_package(OpenCV REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    videoio
    highgui
)


add_executable(camera_node src/camera_node.cpp)
ament_target_dependencies(camera_node
  rclcpp
  sensor_msgs
  cv_bridge
)

add_executable(game_node src/game_node.cpp)
ament_target_dependencies(game_node
    rclcpp
    sensor_msgs
    cv_bridge
)

target_include_directories(camera_node PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(camera_node
    ${OpenCV_LIBS}
)

target_include_directories(game_node PRIVATE
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(game_node
    ${OpenCV_LIBS}
)


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

install(TARGETS
  camera_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS
    game_node
    DESTINATION lib/${PROJECT_NAME}
)

ament_package()
