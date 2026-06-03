import threading

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

from fastapi import FastAPI
from pydantic import BaseModel

import time
import cv2
import numpy as np

from fastapi.responses import StreamingResponse, HTMLResponse
from sensor_msgs.msg import Image


app = FastAPI(title="Drone Mobile Command API")


class CommandRequest(BaseModel):
    command: str
    value: float | None = None


class RosCommandPublisher(Node):
    def __init__(self):
        super().__init__("mobile_http_api_node")

        self.publisher = self.create_publisher(String, "/drone_command", 10)

        self.latest_frame_down = None
        self.latest_frame_front = None

        self.camera_down_sub = self.create_subscription(
            Image,
            "/drone/vision/aruco_image",
            self.camera_down_callback,
            10
        )

        self.camera_front_sub = self.create_subscription(
            Image,
            "/drone/vision/yolo_image",
            self.camera_front_callback,
            10
        )

    def publish_command(self, command: str):
        msg = String()
        msg.data = command
        self.publisher.publish(msg)
        self.get_logger().info(f"Publicado em /drone_command: {command}")

    def convert_image(self, msg: Image):
        try:
            height = msg.height
            width = msg.width
            encoding = msg.encoding

            img = np.frombuffer(msg.data, dtype=np.uint8)

            if encoding in ["rgb8", "bgr8"]:
                img = img.reshape((height, width, 3))

                if encoding == "rgb8":
                    img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)

            elif encoding in ["rgba8", "bgra8"]:
                img = img.reshape((height, width, 4))

                if encoding == "rgba8":
                    img = cv2.cvtColor(img, cv2.COLOR_RGBA2BGR)
                else:
                    img = cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)

            else:
                self.get_logger().warn(f"Encoding nao suportado: {encoding}")
                return None

            return img

        except Exception as e:
            self.get_logger().error(f"Erro ao converter imagem: {e}")
            return None


    def camera_down_callback(self, msg: Image):
        img = self.convert_image(msg)

        if img is not None:
            self.latest_frame_down = img


    def camera_front_callback(self, msg: Image):
        img = self.convert_image(msg)

        if img is not None:
            self.latest_frame_front = img

rclpy.init()
ros_node = RosCommandPublisher()


def spin_ros():
    rclpy.spin(ros_node)


ros_thread = threading.Thread(target=spin_ros, daemon=True)
ros_thread.start()


def send_command(command: str, value: float | None = None):
    full_command = command if value is None else f"{command} {value}"
    ros_node.publish_command(full_command)
    return {"status": "ok", "sent": full_command}

def generate_frames(camera_type="down"):

    while True:

        frame = (
            ros_node.latest_frame_down
            if camera_type == "down"
            else ros_node.latest_frame_front
        )

        if frame is None:
            time.sleep(0.03)
            continue

        _, buffer = cv2.imencode(".jpg", frame)

        frame_bytes = buffer.tobytes()

        yield (
            b"--frame\r\n"
            b"Content-Type: image/jpeg\r\n\r\n" +
            frame_bytes +
            b"\r\n"
        )

        time.sleep(0.03)


def make_video_response(camera_type="down"):
    return StreamingResponse(
        generate_frames(camera_type),
        media_type="multipart/x-mixed-replace; boundary=frame"
    )


@app.get("/")
def root():
    return {"message": "Drone Mobile Command API online"}

@app.get("/video_feed")
def video_feed():
    return video_feed_down()

@app.get("/video_feed_down")
def video_feed_down():
    return make_video_response("down")

@app.get("/video_feed_front")
def video_feed_front():
    return make_video_response("front")

@app.get("/panel", response_class=HTMLResponse)
def panel():
    return """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Drone Control Panel</title>
        <style>
            body {
                margin: 0;
                font-family: Arial, sans-serif;
                background: #111827;
                color: white;
                padding: 20px;
            }

            h1 {
                text-align: center;
            }

            .container {
                display: grid;
                grid-template-columns: 1fr 1fr;
                gap: 24px;
                max-width: 1200px;
                margin: auto;
            }

            .card {
                background: #1f2937;
                padding: 20px;
                border-radius: 16px;
                box-shadow: 0 10px 30px rgba(0,0,0,0.35);
            }

            img {
                width: 100%;
                border-radius: 12px;
                background: #000;
            }

            input {
                width: 100%;
                box-sizing: border-box;
                padding: 14px;
                margin-bottom: 16px;
                border-radius: 10px;
                border: none;
                font-size: 16px;
            }

            .buttons {
                display: grid;
                grid-template-columns: repeat(3, 1fr);
                gap: 12px;
            }

            button {
                padding: 15px;
                border: none;
                border-radius: 12px;
                font-size: 15px;
                font-weight: bold;
                cursor: pointer;
                background: #2563eb;
                color: white;
            }

            button:hover {
                background: #1d4ed8;
            }

            .danger {
                background: #dc2626;
            }

            .danger:hover {
                background: #b91c1c;
            }

            .wide {
                grid-column: span 3;
            }

            #log {
                margin-top: 16px;
                padding: 12px;
                border-radius: 10px;
                background: #111827;
                min-height: 50px;
                font-family: monospace;
                font-size: 14px;
                white-space: pre-line;
            }

            @media (max-width: 800px) {
                .container {
                    grid-template-columns: 1fr;
                }
            }
        </style>
    </head>

    <body>
        <h1>Drone Control Panel</h1>

        <div class="container">
            <div class="card">
                <h2>Camera Down</h2>
                <img src="/video_feed_down">

                <h2>Camera Front</h2>
                <img src="/video_feed_front">
            </div>

            <div class="card">
                <h2>Comandos</h2>

                <label>Distância / Altitude (m)</label>
                <input id="valueInput" type="number" value="1" step="0.5" min="0.1">

                <div class="buttons">
                    <button onclick="send('/arm', 'ARM')">ARM</button>
                    <button onclick="send('/offboard', 'OFFBOARD')">OFFBOARD</button>
                    <button onclick="sendValue('/takeoff', 'TAKEOFF')">TAKEOFF</button>

                    <button onclick="sendMove('left')">LEFT</button>
                    <button onclick="sendMove('up')">UP</button>
                    <button onclick="sendMove('right')">RIGHT</button>

                    <button onclick="sendMove('back')">BACK</button>
                    <button onclick="sendMove('down')">DOWN</button>
                    <button onclick="sendMove('forward')">FORWARD</button>

                    <button class="wide" onclick="send('/aruco/on', 'ARUCO ON')">ARUCO ON</button>
                    <button class="wide" onclick="send('/aruco/off', 'ARUCO OFF')">ARUCO OFF</button>

                    <button class="wide" onclick="send('/person/on', 'PERSON ON')">PERSON ON</button>
                    <button class="wide" onclick="send('/person/off', 'PERSON OFF')">PERSON OFF</button>
                    
                    <button class="danger wide" onclick="send('/land', 'LAND')">LAND</button>
                    <button class="danger wide" onclick="send('/rtl', 'RTL')">RTL</button>
                </div>

                <h3>Log</h3>
                <div id="log">Aguardando comando...</div>
            </div>
        </div>

        <script>
            function getValue() {
                const value = document.getElementById("valueInput").value;
                return value || 1;
            }

            function addLog(message) {
                const log = document.getElementById("log");
                const time = new Date().toLocaleTimeString();
                log.textContent = `[${time}] ${message}\\n` + log.textContent;
            }

            async function send(endpoint, label) {
                try {
                    const response = await fetch(endpoint, { method: "POST" });

                    if (response.ok) {
                        addLog(`✅ Comando recebido com sucesso: ${label}`);
                    } else {
                        addLog(`❌ Erro ao enviar comando: ${label}`);
                    }
                } catch (error) {
                    addLog(`❌ Falha de conexão: ${label}`);
                }
            }

            async function sendValue(endpoint, label) {
                const value = getValue();
                await send(`${endpoint}?value=${value}`, `${label} ${value}m`);
            }

            async function sendMove(direction) {
                const value = getValue();
                await send(`/move/${direction}?value=${value}`, `${direction.toUpperCase()} ${value}m`);
            }
        </script>
    </body>
    </html>
    """

@app.post("/command")
def command(request: CommandRequest):
    return send_command(request.command, request.value)


@app.post("/arm")
def arm():
    return send_command("arm")


@app.post("/offboard")
def offboard():
    return send_command("offboard")


@app.post("/takeoff")
def takeoff(value: float = 2.0):
    return send_command("takeoff", value)


@app.post("/land")
def land():
    return send_command("land")


@app.post("/rtl")
def rtl():
    return send_command("rtl")


@app.post("/move/{direction}")
def move(direction: str, value: float = 1.0):
    allowed = ["right", "left", "forward", "back", "up", "down"]

    if direction not in allowed:
        return {
            "status": "error",
            "message": f"Direção inválida. Use: {allowed}"
        }

    return send_command(direction, value)

@app.post("/velocity")
def velocity(vx: float = 0.0, vy: float = 0.0, vz: float = 0.0, yaw: float = 0.0):
    return send_command(f"velocity {vx} {vy} {vz} {yaw}")


@app.post("/stop")
def stop():
    return send_command("stop")


@app.post("/aruco/on")
def aruco_on():
    return send_command("aruco on")


@app.post("/aruco/off")
def aruco_off():
    return send_command("aruco off")

@app.post("/person/on")
def person_on():
    return send_command("person on")


@app.post("/person/off")
def person_off():
    return send_command("person off")