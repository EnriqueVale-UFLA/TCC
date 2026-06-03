#!/bin/bash

gnome-terminal \
--tab --title="Voice Controller" -- bash -c "
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
source ~/Documentos/TCC/drone_core/install/setup.bash
source ~/Documentos/TCC/voice_command/install/setup.bash
ros2 run voice_drone_control_cpp relative_offboard_control
exec bash
" \
--tab --title="Voice Input" -- bash -c "
cd ~/Documentos/TCC/voice_command/src/voice_drone_control_cpp/src
source /opt/ros/jazzy/setup.bash
source ~/Documentos/TCC/voice_command/install/setup.bash
source ~/venv_voice_drone/bin/activate
python3 voice_input.py
exec bash
"