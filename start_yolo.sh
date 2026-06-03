#!/bin/bash

gnome-terminal --title="YOLO Detector" -- bash -c "
source ~/vision_env/bin/activate
source /opt/ros/jazzy/setup.bash
source /home/enrique/Documentos/TCC/deep_learning_command/install/setup.bash
python3 /home/enrique/Documentos/TCC/deep_learning_command/src/drone_vision/src/yolo_detector.py
exec bash
"