cmake_minimum_required(VERSION 3.8)
project(mobile_command_bridge)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(drone_core REQUIRED)

add_executable(mobile_command_bridge
  src/mobile_command_bridge.cpp
  /home/enrique/Documentos/TCC/drone_core/src/drone_core/src/drone_controller.cpp
)

target_include_directories(mobile_command_bridge PUBLIC
  /home/enrique/Documentos/TCC/drone_core/src/drone_core/include
)

ament_target_dependencies(
  mobile_command_bridge
  rclcpp
  std_msgs
  px4_msgs
  drone_core
)

install(
  TARGETS mobile_command_bridge
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()