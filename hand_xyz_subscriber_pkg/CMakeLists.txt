cmake_minimum_required(VERSION 3.8)
project(hand_xyz_subscriber_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)

add_executable(hand_xyz_subscriber src/hand_xyz_subscriber.cpp)
ament_target_dependencies(hand_xyz_subscriber
  geometry_msgs
  rclcpp
)

install(TARGETS hand_xyz_subscriber
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
