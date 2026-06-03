#!/usr/bin/env python3

import cv2
import rclpy

from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
from ultralytics import YOLO
from geometry_msgs.msg import Twist


class YoloDetector(Node):
    def __init__(self):
        super().__init__('yolo_detector')

        self.input_topic = '/world/tcc_vision/model/x500_dual_cam_0/link/camera_front_link/sensor/camera_front/image'
        self.output_topic = '/drone/vision/yolo_image'

        self.bridge = CvBridge()

        self.model = YOLO("yolov8n.pt")

        self.frame_count = 0

        self.subscriber = self.create_subscription(
            Image,
            self.input_topic,
            self.image_callback,
            1
        )

        self.publisher = self.create_publisher(
            Image,
            self.output_topic,
            1
        )

        self.cmd_publisher = self.create_publisher(
            Twist,
            '/drone/vision/person_cmd_vel',
            10
        )

        self.get_logger().info(f'YOLO lendo câmera: {self.input_topic}')
        self.get_logger().info(f'YOLO publicando imagem em: {self.output_topic}')

    def image_callback(self, msg):
        frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

        self.frame_count += 1

        # Processa apenas 1 a cada 6 frames
        if self.frame_count % 6 != 0:
            return

        # Reduz resolução para YOLO ficar mais leve
        small_frame = cv2.resize(frame, (320, 240))

        results = self.model(small_frame, conf=0.45, classes=[0], verbose=False)

        annotated_frame = results[0].plot()

        # Volta para resolução original para visualizar melhor
        annotated_frame = cv2.resize(
            annotated_frame,
            (frame.shape[1], frame.shape[0])
        )

        # =========================
        # ALIGN PERSON
        # =========================

        height, width, _ = small_frame.shape

        image_center_x = width / 2
        image_center_y = height / 2

        cmd = Twist()

        person_found = False

        for box in results[0].boxes:

            cls_id = int(box.cls[0])

            class_name = self.model.names[cls_id]

            if class_name == "person":

                person_found = True

                x1, y1, x2, y2 = box.xyxy[0].tolist()

                center_x = (x1 + x2) / 2
                center_y = (y1 + y2) / 2

                error_x = center_x - image_center_x
                error_y = center_y - image_center_y

                kp_side = 0.016
                kp_forward = 2.0

                cmd.linear.y = max(min(kp_side * error_x, 1.0), -0.35)

                box_width = x2 - x1
                box_height = y2 - y1
                box_area = box_width * box_height
                image_area = width * height

                area_ratio = box_area / image_area

                target_area_ratio = 0.1

                error_area = target_area_ratio - area_ratio

                cmd.linear.x = max(min(kp_forward * error_area, 0.85), -0.25)

                if abs(error_x) < 8:
                    cmd.linear.y = 0.0

                if error_area > 0.04 and cmd.linear.x < 0.35:
                    cmd.linear.x = 0.35

                if abs(error_area) < 0.03:
                    cmd.linear.x = 0.0

                self.get_logger().info(
                    f'Pessoa | erro_x={error_x:.2f} area_ratio={area_ratio:.3f} '
                    f'error_area={error_area:.3f} vx={cmd.linear.x:.2f} vy={cmd.linear.y:.2f}'
                )

                break

        if not person_found:
            cmd = Twist()

        self.cmd_publisher.publish(cmd)

        processed_msg = self.bridge.cv2_to_imgmsg(
            annotated_frame,
            encoding='bgr8'
        )

        processed_msg.header = msg.header

        self.publisher.publish(processed_msg)


def main(args=None):
    rclpy.init(args=args)

    node = YoloDetector()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()