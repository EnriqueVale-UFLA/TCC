import json
import queue

import rclpy
from rclpy.node import Node
from std_msgs.msg import String

import sounddevice as sd
from vosk import Model, KaldiRecognizer


AUDIO_QUEUE = queue.Queue()

MODEL_PATH = "/home/enrique/models/vosk-model-small-pt-0.3"

# Se quiser escolher o microfone manualmente, coloque o índice aqui.
# Exemplo: DEVICE_INDEX = 2
DEVICE_INDEX = None


def audio_callback(indata, frames, time, status):
    if status:
        print(status)
    AUDIO_QUEUE.put(bytes(indata))


class VoiceInputNode(Node):
    def __init__(self):
        super().__init__("voice_input_node")
        self.publisher = self.create_publisher(String, "/voice_cmd", 10)

    def publish_command(self, text):
        msg = String()
        msg.data = text
        self.publisher.publish(msg)
        self.get_logger().info(f"Publicado em /voice_cmd: {text}")


def main():
    rclpy.init()
    node = VoiceInputNode()

    print("Carregando modelo Vosk...")
    model = Model(MODEL_PATH)
    recognizer = KaldiRecognizer(model, 16000)

    print("Microfones disponíveis:")
    print(sd.query_devices())

    print("\n🎤 Fale um comando...")

    with sd.RawInputStream(
        samplerate=16000,
        blocksize=8000,
        dtype="int16",
        channels=1,
        callback=audio_callback,
        device=DEVICE_INDEX,
    ):
        while rclpy.ok():
            data = AUDIO_QUEUE.get()

            if recognizer.AcceptWaveform(data):
                result = json.loads(recognizer.Result())
                text = result.get("text", "").lower().strip()

                if text:
                    print(f"Você disse: {text}")
                    node.publish_command(text)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()