cmake_minimum_required(VERSION 3.8)
project(voice_drone_control_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

add_executable(relative_offboard_control
  src/relative_offboard_control.cpp
)

ament_target_dependencies(
  relative_offboard_control
  rclcpp
  std_msgs
)

install(
  TARGETS relative_offboard_control
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()