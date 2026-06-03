# TCC - Sistema Multimodal de Controle de Drone em Simulação

Este repositório contém o projeto de TCC desenvolvido para simulação e controle de um drone utilizando **ROS 2**, **PX4**, **Gazebo** e diferentes modalidades de comando. O sistema foi estruturado de forma modular para permitir controle por voz, aplicativo/web, Telegram e visão computacional com ArUco e YOLO.

## Objetivo do projeto

O objetivo principal é desenvolver uma arquitetura de controle multimodal para drones em ambiente simulado, permitindo que diferentes interfaces enviem comandos para um sistema central de roteamento e controle.

O projeto tem como aplicações possíveis:

- apoio em operações de busca e resgate;
- acessibilidade para pessoas com mobilidade reduzida;
- controle remoto por aplicativo, web ou mensagens;
- alinhamento automático com marcadores visuais;
- detecção e seguimento de pessoas usando visão computacional;
- estudo de integração entre ROS 2, PX4, Gazebo e interfaces externas.

## Visão geral da arquitetura

A arquitetura foi dividida em módulos independentes:

```text
TCC/
├── drone_core/                # Biblioteca central de controle do drone
├── drone_command_router/      # Roteador de comandos e modos de operação
├── voice_command/             # Controle por voz e controle offboard relativo
├── mobile_command/            # API, app/web e integração por mensagens
├── deep_learning_command/     # Visão computacional com ArUco e YOLO
├── start_tcc.sh               # Script principal de inicialização
├── start_voice.sh             # Script para inicializar o controle por voz
├── start_aruco.sh             # Script para inicializar o detector ArUco
├── start_yolo.sh              # Script para inicializar o detector YOLO
└── README.md
```

## Módulos do sistema

### drone_core

Contém a biblioteca central de controle do drone. Esse módulo concentra funções de controle reutilizáveis, como:

- armar e desarmar;
- decolar;
- pousar;
- mover em direções relativas;
- publicar comandos compatíveis com PX4;
- manter controle em modo Offboard.

Esse módulo serve como base para outros pacotes, evitando duplicação de lógica de controle.

### drone_command_router

Responsável por receber comandos vindos de diferentes fontes e decidir qual ação deve ser executada. Ele funciona como uma camada intermediária entre as interfaces de entrada e o controle real do drone.

Fontes de comando consideradas:

- comandos por voz;
- comandos por aplicativo/web;
- comandos por Telegram;
- comandos de visão computacional;
- comandos automáticos como ArUco ou YOLO.

### voice_command

Contém o controle por voz e o nó de controle relativo em Offboard.

Esse módulo permite comandos como:

- armar;
- decolar;
- pousar;
- subir;
- descer;
- ir para frente;
- ir para trás;
- ir para esquerda;
- ir para direita;
- girar.

### mobile_command

Contém a API e as interfaces ligadas a comandos externos, como app, web e Telegram.

A API permite enviar comandos ao sistema por requisições HTTP, facilitando integração com interfaces móveis ou painéis de controle.

### deep_learning_command

Contém os módulos de visão computacional.

Principais componentes:

- visualização da câmera;
- detecção de marcador ArUco;
- detecção de pessoas com YOLO;
- publicação de comandos de alinhamento ou seguimento.

## Tecnologias utilizadas

- Ubuntu 24.04
- ROS 2 Jazzy
- PX4 Autopilot
- Gazebo / Gazebo Sim
- QGroundControl
- Micro XRCE-DDS Agent
- Python 3
- C++
- OpenCV
- YOLOv8
- FastAPI
- Uvicorn
- Vosk
- tmux
- colcon

## Pré-requisitos

Antes de executar o projeto, é necessário ter instalados:

- ROS 2 Jazzy;
- PX4 Autopilot;
- Gazebo compatível com PX4;
- QGroundControl;
- Micro XRCE-DDS Agent;
- Python 3;
- colcon;
- tmux;
- Git.

Instale pacotes básicos:

```bash
sudo apt update
sudo apt install python3-venv python3-pip tmux git colcon-common-extensions
```

## Organização esperada de diretórios

O projeto foi desenvolvido considerando a seguinte organização local:

```text
~/Documentos/TCC
~/PX4-Autopilot
~/ws_px4
```

O repositório principal deve estar em:

```bash
~/Documentos/TCC
```

Caso o projeto esteja em outro caminho, será necessário alterar os caminhos nos scripts `.sh`.

## Clonando o repositório

```bash
cd ~/Documentos
git clone https://github.com/EnriqueVale-UFLA/TCC.git
cd TCC
```

## Instalação e teste do PX4

Clone o PX4 Autopilot:

```bash
cd ~
git clone https://github.com/PX4/PX4-Autopilot.git --recursive
cd PX4-Autopilot
```

Execute o script de instalação do PX4 conforme a documentação oficial do PX4 para Ubuntu.

Depois, teste a simulação básica:

```bash
cd ~/PX4-Autopilot
make px4_sitl gz_x500
```

Para este projeto, o modelo utilizado na demonstração principal é o `gz_x500_dual_cam` com o mundo `tcc_vision`:

```bash
cd ~/PX4-Autopilot
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:$HOME/PX4-Autopilot/Tools/simulation/gz/models
PX4_GZ_WORLD=tcc_vision make px4_sitl gz_x500_dual_cam
```

## QGroundControl

O QGroundControl é utilizado para acompanhar o estado do drone, conexão com PX4, arming, modo de voo e mensagens de segurança.

Coloque o AppImage na pasta home e torne executável:

```bash
cd ~
chmod +x QGroundControl-x86_64.AppImage
./QGroundControl-x86_64.AppImage
```

## Micro XRCE-DDS Agent

O Micro XRCE-DDS Agent faz a ponte de comunicação entre o PX4 e o ROS 2.

Execute:

```bash
MicroXRCEAgent udp4 -p 8888
```

Sem esse agente, os tópicos `/fmu/out/...` e `/fmu/in/...` podem não aparecer corretamente no ROS 2.

## Configuração do workspace PX4/ROS 2

O projeto considera a existência de um workspace em:

```bash
~/ws_px4
```

Antes de executar nós que se comunicam com PX4, carregue:

```bash
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
```

## Build dos pacotes ROS 2

Cada módulo é um workspace ROS 2 separado. Recomenda-se compilar na ordem abaixo.

### 1. drone_core

```bash
cd ~/Documentos/TCC/drone_core
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
colcon build
```

### 2. drone_command_router

```bash
cd ~/Documentos/TCC/drone_command_router
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
source ~/Documentos/TCC/drone_core/install/setup.bash
colcon build
```

### 3. voice_command

```bash
cd ~/Documentos/TCC/voice_command
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
source ~/Documentos/TCC/drone_core/install/setup.bash
colcon build
```

### 4. mobile_command

```bash
cd ~/Documentos/TCC/mobile_command
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
colcon build
```

### 5. deep_learning_command

```bash
cd ~/Documentos/TCC/deep_learning_command
source /opt/ros/jazzy/setup.bash
colcon build
```

## Ambientes virtuais Python

O projeto utiliza ambientes virtuais separados para evitar conflito entre dependências.

### Ambiente virtual para voz

```bash
python3 -m venv ~/venv_voice_drone
source ~/venv_voice_drone/bin/activate
pip install --upgrade pip
pip install vosk sounddevice pyyaml setuptools
```

### Ambiente virtual para API mobile/web

```bash
python3 -m venv ~/venv_mobile_command
source ~/venv_mobile_command/bin/activate
pip install --upgrade pip
pip install fastapi uvicorn python-dotenv requests
```

### Ambiente virtual para visão computacional

```bash
python3 -m venv ~/vision_env
source ~/vision_env/bin/activate
pip install --upgrade pip
pip install "numpy<2"
pip install opencv-python ultralytics pyyaml
```

Caso ocorram erros envolvendo OpenCV, YOLO ou NumPy, uma solução comum é manter o NumPy abaixo da versão 2:

```bash
pip install "numpy<2"
```

## Inicialização principal

O script `start_tcc.sh` inicializa os componentes principais do sistema:

- PX4 SITL com Gazebo;
- QGroundControl;
- Micro XRCE-DDS Agent;
- drone_command_router;
- API mobile/web;
- bridges das câmeras do Gazebo para ROS 2.

Execute:

```bash
cd ~/Documentos/TCC
chmod +x start_tcc.sh
./start_tcc.sh
```

Caso o script utilize `tmux`, os principais comandos são:

```bash
Ctrl + b, depois n       # próxima janela
Ctrl + b, depois p       # janela anterior
Ctrl + b, depois d       # sair da sessão sem encerrar processos
tmux attach -t tcc       # voltar para a sessão
tmux kill-session -t tcc # encerrar todos os processos da sessão
```

## Inicialização dos módulos opcionais

### Controle por voz

```bash
cd ~/Documentos/TCC
chmod +x start_voice.sh
./start_voice.sh
```

Esse script inicializa:

- o nó de controle relativo em Offboard;
- o script Python de entrada de voz.

### Detector ArUco

```bash
cd ~/Documentos/TCC
chmod +x start_aruco.sh
./start_aruco.sh
```

Esse script inicializa o detector de marcador ArUco.

### Detector YOLO

```bash
cd ~/Documentos/TCC
chmod +x start_yolo.sh
./start_yolo.sh
```

Esse script inicializa o detector YOLO para detecção de pessoas.

## Bridges das câmeras

As câmeras utilizadas no mundo `tcc_vision` são:

```text
/world/tcc_vision/model/x500_dual_cam_0/link/camera_down_link/sensor/camera_down/image
/world/tcc_vision/model/x500_dual_cam_0/link/camera_front_link/sensor/camera_front/image
```

Comandos individuais para criar as bridges:

```bash
source /opt/ros/jazzy/setup.bash
ros2 run ros_gz_bridge parameter_bridge \
"/world/tcc_vision/model/x500_dual_cam_0/link/camera_down_link/sensor/camera_down/image@sensor_msgs/msg/Image@gz.msgs.Image"
```

```bash
source /opt/ros/jazzy/setup.bash
ros2 run ros_gz_bridge parameter_bridge \
"/world/tcc_vision/model/x500_dual_cam_0/link/camera_front_link/sensor/camera_front/image@sensor_msgs/msg/Image@gz.msgs.Image"
```

## Tópicos principais

Alguns tópicos importantes utilizados no projeto:

```text
/drone_command
/mobile_cmd
/drone/vision/cmd_vel
/drone/vision/person_cmd_vel
/fmu/out/vehicle_odometry
/fmu/out/vehicle_local_position
```

## Comandos úteis para teste

Listar tópicos ROS 2:

```bash
ros2 topic list
```

Verificar tópicos relacionados ao PX4:

```bash
ros2 topic list | grep fmu
```

Verificar odometria do drone:

```bash
ros2 topic echo /fmu/out/vehicle_odometry --qos-reliability best_effort
```

Verificar frequência de uma câmera:

```bash
ros2 topic hz /world/tcc_vision/model/x500_dual_cam_0/link/camera_front_link/sensor/camera_front/image
```

Publicar comando manual para o roteador:

```bash
ros2 topic pub --once /drone_command std_msgs/msg/String "{data: 'aruco on'}"
```

## Fluxo recomendado para demonstração

1. Iniciar o sistema principal:

```bash
./start_tcc.sh
```

2. Aguardar o Gazebo e o PX4 carregarem.

3. Conferir se o QGroundControl conectou ao veículo.

4. Conferir se o Micro XRCE-DDS Agent recebeu conexão.

5. Verificar se os tópicos ROS aparecem:

```bash
ros2 topic list
```

6. Testar comandos básicos.

7. Ativar o módulo que será demonstrado:

```bash
./start_voice.sh
```

ou:

```bash
./start_aruco.sh
```

ou:

```bash
./start_yolo.sh
```

## Pontos de atenção

### Ordem de inicialização

A ordem recomendada é:

1. PX4 + Gazebo;
2. QGroundControl;
3. Micro XRCE-DDS Agent;
4. drone_command_router;
5. API mobile/web;
6. bridges das câmeras;
7. módulos opcionais: voz, ArUco ou YOLO.

### Modo Offboard

O PX4 exige envio contínuo de setpoints para manter o modo Offboard. Caso o drone pare de receber comandos, pode ocorrer failsafe, saída do modo Offboard ou retorno automático.

### QGroundControl

O QGroundControl deve estar conectado corretamente ao PX4. Caso contrário, podem aparecer falhas de preflight, problemas de arming ou mensagens de failsafe.

### Ambientes virtuais

Sempre ative o ambiente virtual correto antes de rodar scripts Python manualmente.

Exemplos:

```bash
source ~/venv_voice_drone/bin/activate
source ~/venv_mobile_command/bin/activate
source ~/vision_env/bin/activate
```

### YOLO e desempenho

O YOLO pode consumir bastante CPU/GPU dependendo do modelo utilizado. Para demonstração, recomenda-se usar um modelo leve, como `yolov8n.pt`.

### Arquivos sensíveis

Não subir para o GitHub:

- arquivos `.env`;
- tokens de bots;
- credenciais;
- arquivos de log;
- builds gerados pelo colcon;
- ambientes virtuais;
- modelos muito grandes.

## Sugestão de `.gitignore`

```gitignore
# ROS 2 / colcon
build/
install/
log/

# Python
__pycache__/
*.pyc
.venv/
venv/
venv_voice_drone/
venv_mobile_command/
vision_env/

# Environment variables / secrets
.env
*.env

# ROS bags / logs
*.db3
*.bag
*.bag.active
*.mcap
*.ulg
*.log

# Compiled files
*.so
*.o
*.a

# YOLO / ML models
*.pt
*.onnx
*.engine

# VS Code
.vscode/

# PX4 ROS workspace cloned inside project, if present
voice_command/src/px4_ros_com/

# OS
.DS_Store
```

## Resolução de problemas

### O drone não arma

Verifique:

- QGroundControl está aberto e conectado?
- PX4 terminou de inicializar?
- Micro XRCE-DDS Agent está rodando?
- Existem mensagens de preflight fail no QGroundControl?

### O ROS 2 não mostra tópicos do PX4

Verifique se o agente está ativo:

```bash
MicroXRCEAgent udp4 -p 8888
```

Depois carregue o ambiente e confira:

```bash
source /opt/ros/jazzy/setup.bash
source ~/ws_px4/install/local_setup.bash
ros2 topic list | grep fmu
```

### A câmera não aparece no ROS 2

Verifique se os bridges foram iniciados:

```bash
ros2 topic list | grep camera
```

Também confirme se o mundo carregado é `tcc_vision` e se o modelo é `x500_dual_cam`.

### O comando de voz não reconhece corretamente

Verifique:

- microfone selecionado;
- ambiente virtual ativado;
- dependências instaladas;
- ruído no ambiente;
- palavras usadas nos comandos.

Algumas palavras podem ser reconhecidas incorretamente pelo modelo de voz. Nesse caso, pode ser necessário adaptar os termos dos comandos.

### YOLO está lento ou travando

Recomendações:

- usar `yolov8n.pt`;
- reduzir resolução da imagem;
- evitar rodar ArUco e YOLO juntos se o computador estiver sobrecarregado;
- garantir que o ambiente virtual da visão esteja correto.

## Arquivos que merecem atenção antes da apresentação

Arquivos centrais que concentram a lógica principal:

```text
drone_core/src/drone_core/src/drone_controller.cpp
drone_core/src/drone_core/include/drone_core/drone_controller.hpp
drone_command_router/src/drone_command_router/src/drone_command_router.cpp
voice_command/src/voice_drone_control_cpp/src/relative_offboard_control.cpp
voice_command/src/voice_drone_control_cpp/src/voice_input.py
deep_learning_command/src/drone_vision/src/aruco_detector.py
deep_learning_command/src/drone_vision/src/yolo_detector.py
```

Recomenda-se revisar esses arquivos para remover prints de teste, comentários antigos e trechos não utilizados.

## Possíveis melhorias futuras

- criação de launch files ROS 2 para substituir parte dos scripts `.sh`;
- painel web mais completo com status do modo ativo;
- seleção dinâmica entre modos de controle;
- melhoria da robustez do reconhecimento de voz;
- otimização da visão computacional;
- integração mais forte entre câmera, interface web e comandos autônomos;
- execução em hardware real com Pixhawk.

## Conclusão

Este projeto demonstra uma arquitetura modular de controle de drones em simulação, integrando ROS 2, PX4, Gazebo, controle por voz, aplicação web/mobile e visão computacional. A separação em módulos facilita manutenção, testes e expansão futura do sistema.
