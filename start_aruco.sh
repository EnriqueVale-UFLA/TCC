#!/bin/bash

gnome-terminal --title="ArUco Detector" -- bash -c "
source /opt/ros/jazzy/setup.bash
source /home/enrique/Documentos/TCC/deep_learning_command/install/setup.bash
ros2 run drone_vision aruco_detector
exec bash
"