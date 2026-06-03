cmake_minimum_required(VERSION 3.8)
project(drone_command_router)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(drone_core REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(drone_command_router
  src/drone_command_router.cpp
  /home/enrique/Documentos/TCC/drone_core/src/drone_core/src/drone_controller.cpp
)

target_include_directories(drone_command_router PUBLIC
  /home/enrique/Documentos/TCC/drone_core/src/drone_core/include
)

ament_target_dependencies(
  drone_command_router
  rclcpp
  std_msgs
  px4_msgs
  drone_core
  geometry_msgs
)

install(
  TARGETS drone_command_router
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()