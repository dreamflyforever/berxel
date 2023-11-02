"""Launch berxel_camera_ros2 node and rviz."""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    paramer_dir = os.path.join(get_package_share_directory('berxel_camera_ros2'), 'config', 'berxel_ihawk_100.yaml')
    rviz_config_dir = os.path.join(get_package_share_directory('berxel_camera_ros2'), 'rviz', 'berxel_default.rviz')
    return LaunchDescription([
        Node(
            package='berxel_camera_ros2', 
            namespace='berxel_camera_1',
            executable='berxel_camera_ros2',
            name='berxel_camera_1',
            parameters=[paramer_dir, {'DeviceName':'berxel_camera_1'} ,{'serial_number':''}, {'device_bus' : 3}, {'device_port' : '4-1'}],
            output='screen'),
        
        Node(
            package='berxel_camera_ros2', 
            namespace='berxel_camera_2',
            executable='berxel_camera_ros2',
            name='berxel_camera_2',
            parameters=[paramer_dir, {'DeviceName':'berxel_camera_2'}, {'serial_number': ''}, {'device_bus' : 3}, {'device_port' : '4-2'}],
            output='screen'),

        # Rviz
        # Node(
        #     package='rviz2', node_executable='rviz2', output='screen',
        #     arguments=['--display-config', rviz_config_dir]),
    ])