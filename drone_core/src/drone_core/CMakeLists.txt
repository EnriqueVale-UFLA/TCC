cmake_minimum_required(VERSION 3.8)
project(drone_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(px4_msgs REQUIRED)

add_library(drone_controller SHARED
  src/drone_controller.cpp
)

target_include_directories(drone_controller PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(drone_controller
  rclcpp
  px4_msgs
)

install(
  TARGETS drone_controller
  EXPORT export_drone_controller
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

ament_export_include_directories(include)
ament_export_libraries(drone_controller)
ament_export_targets(export_drone_controller HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp px4_msgs)

ament_package()