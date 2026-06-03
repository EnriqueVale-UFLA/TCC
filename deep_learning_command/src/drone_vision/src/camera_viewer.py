#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
import cv2


class CameraViewer(Node):
    def __init__(self):
        super().__init__('camera_viewer')

        self.topic = '/world/tcc_vision/model/x500_mono_cam_down_0/link/camera_link/sensor/camera/image'

        self.bridge = CvBridge()

        self.subscription = self.create_subscription(
            Image,
            self.topic,
            self.image_callback,
            10
        )

        self.get_logger().info(f'Camera Viewer iniciado. Escutando: {self.topic}')

    def image_callback(self, msg):
        frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding='bgr8')

        cv2.imshow('Camera do Drone', frame)
        cv2.waitKey(1)


def main(args=None):
    rclpy.init(args=args)

    node = CameraViewer()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    cv2.destroyAllWindows()
    rclpy.shutdown()


if __name__ == '__main__':
    main()