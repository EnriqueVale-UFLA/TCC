#!/usr/bin/env python3

import cv2
import rclpy

from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
from geometry_msgs.msg import Twist
from ultralytics import YOLO


class VisionDetector(Node):
    def __init__(self):
        super().__init__('vision_detector')

        self.input_topic = '/world/tcc_vision/model/x500_mono_cam_down_0/link/camera_link/sensor/camera/image'
        self.output_topic = '/drone/vision/processed_image'

        self.bridge = CvBridge()

        self.publisher = self.create_publisher(
            Image,
            self.output_topic,
            10
        )

        self.cmd_publisher = self.create_publisher(
            Twist,
            '/drone/vision/cmd_vel',
            10
        )

        self.subscriber = self.create_subscription(
            Image,
            self.input_topic,
            self.image_callback,
            10
        )

        self.aruco_dict = cv2.aruco.getPredefinedDictionary(
            cv2.aruco.DICT_4X4_50
        )
        self.aruco_params = cv2.aruco.DetectorParameters_create()

        self.model = YOLO("yolov8n.pt")

        self.get_logger().info(f'Lendo câmera: {self.input_topic}')
        self.get_logger().info(f'Publicando imagem processada em: {self.output_topic}')

    def image_callback(self, msg):
        frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

        # =========================
        # YOLO
        # =========================
        results = self.model(frame, conf=0.45, verbose=False)
        frame = results[0].plot()

        # =========================
        # ArUco
        # =========================
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        corners, ids, rejected = cv2.aruco.detectMarkers(
            gray,
            self.aruco_dict,
            parameters=self.aruco_params
        )

        if ids is not None:
            cv2.aruco.drawDetectedMarkers(frame, corners, ids)

            for marker_id, marker_corners in zip(ids.flatten(), corners):
                pts = marker_corners[0]

                center_x = int(pts[:, 0].mean())
                center_y = int(pts[:, 1].mean())

                cv2.circle(frame, (center_x, center_y), 6, (0, 0, 255), -1)

                cv2.putText(
                    frame,
                    f'ArUco ID: {marker_id}',
                    (center_x + 10, center_y - 10),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (0, 255, 0),
                    2
                )

            height, width, _ = frame.shape

            image_center_x = width / 2
            image_center_y = height / 2

            marker_corners = corners[0]
            pts = marker_corners[0]

            center_x = int(pts[:, 0].mean())
            center_y = int(pts[:, 1].mean())

            error_x = center_x - image_center_x
            error_y = center_y - image_center_y

            cmd = Twist()

            kp_x = 0.001
            kp_y = 0.001

            cmd.linear.y = kp_x * error_x
            cmd.linear.x = kp_y * error_y

            self.cmd_publisher.publish(cmd)

        else:
            self.cmd_publisher.publish(Twist())

        processed_msg = self.bridge.cv2_to_imgmsg(frame, encoding='bgr8')
        processed_msg.header = msg.header

        self.publisher.publish(processed_msg)


def main(args=None):
    rclpy.init(args=args)

    node = VisionDetector()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()