from setuptools import find_packages, setup

package_name = 'drone_vision'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='enrique',
    maintainer_email='riqueav09@gnail.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'camera_viewer = src.camera_viewer:main',
            'aruco_detector = src.aruco_detector:main',
            'yolo_detector = src.yolo_detector:main',
            'vision_detector = src.vision_detector:main',
        ],
    },
)
