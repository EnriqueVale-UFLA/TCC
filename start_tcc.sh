#!/bin/bash

SESSION="tcc"

tmux kill-session -t $SESSION 2>/dev/null

tmux new-session -d -s $SESSION -n "PX4"

tmux send-keys -t $SESSION:0 "
source /opt/ros/jazzy/setup.bash
cd ~/PX4-Autopilot
export GZ_SIM_RESOURCE_PATH=\$GZ_SIM_RESOURCE_PATH:\$HOME/PX4-Autopilot/Tools/simulation/gz/models
PX4_GZ_WORLD=tcc_vision make px4_sitl gz_x500_dual_cam
" C-m

tmux new-window -t $SESSION -n "QGC"
tmux send-keys -t $SESSION:1 "
sleep 8
cd ~
./QGroundControl-x86_64.AppImage
" C-m

tmux new-window -t $SESSION -n "MicroAgent"
tmux send-keys -t $SESSION:2 "
sleep 10
MicroXRCEAgent udp4 -p 8888
" C-m

tmux new-window -t $SESSION -n "Router"
tmux send-keys -t $SESSION:3 "
sleep 15
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
source ~/Documentos/TCC/drone_core/install/setup.bash
source ~/Documentos/TCC/drone_command_router/install/setup.bash
ros2 run drone_command_router drone_command_router
" C-m

tmux new-window -t $SESSION -n "API"
tmux send-keys -t $SESSION:4 "
sleep 18
cd ~/Documentos/TCC/mobile_command/api_server
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
source ~/Documentos/TCC/mobile_command/install/setup.bash
source ~/venv_mobile_command/bin/activate
uvicorn main:app --host 0.0.0.0 --port 8000
" C-m

tmux new-window -t $SESSION -n "CamDown"
tmux send-keys -t $SESSION:5 "
sleep 25
source /opt/ros/jazzy/setup.bash
ros2 run ros_gz_bridge parameter_bridge '/world/tcc_vision/model/x500_dual_cam_0/link/camera_down_link/sensor/camera_down/image@sensor_msgs/msg/Image@gz.msgs.Image'
" C-m

tmux new-window -t $SESSION -n "CamFront"
tmux send-keys -t $SESSION:6 "
sleep 27
source /opt/ros/jazzy/setup.bash
ros2 run ros_gz_bridge parameter_bridge '/world/tcc_vision/model/x500_dual_cam_0/link/camera_front_link/sensor/camera_front/image@sensor_msgs/msg/Image@gz.msgs.Image'
" C-m

tmux attach-session -t $SESSION