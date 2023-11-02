"""Launch berxel_camera_ros2 node and rviz."""

import os

from ament_index_python.packages import get_package_share_directory
import launch
from launch_ros.actions import Node


def generate_launch_description():
    paramer_dir = os.path.join(get_package_share_directory('berxel_camera_ros2'), 'config', 'test.yaml')
    rviz_config_dir = os.path.join(get_package_share_directory('berxel_camera_ros2'), 'rviz', 'berxel_default.rviz')

    stream_flag = launch.substitutions.LaunchConfiguration('stream_flag', default='1')
    stream_type = launch.substitutions.LaunchConfiguration('stream_type', default='2')
    depth_width = launch.substitutions.LaunchConfiguration('depth_width', default='640')
    depth_height = launch.substitutions.LaunchConfiguration('depth_height', default='400')
    color_width = launch.substitutions.LaunchConfiguration('color_width', default='640')
    color_height = launch.substitutions.LaunchConfiguration('color_height', default='400')
    ir_width = launch.substitutions.LaunchConfiguration('ir_width', default='640')
    ir_height = launch.substitutions.LaunchConfiguration('ir_height', default='400')
    depth_fps = launch.substitutions.LaunchConfiguration('depth_fps', default='30')
    enable_pointcloud = launch.substitutions.LaunchConfiguration('enable_pointcloud', default='true')
    enable_align = launch.substitutions.LaunchConfiguration('enable_align', default='true')
    enable_denoise = launch.substitutions.LaunchConfiguration('enable_denoise', default='false')
    enable_device_timestamp = launch.substitutions.LaunchConfiguration('enable_device_timestamp', default='true')

    return launch.LaunchDescription([
        Node(
            package='berxel_camera_ros2', 
            executable='berxel_camera_ros2',
            name='berxel_camera_ros2',
            parameters=[paramer_dir, 
            {'stream_flag':stream_flag}, 
            {'stream_type':stream_type},
            {'depth_width':depth_width},
            {'depth_height':depth_height},
            {'color_width':color_width},
            {'color_height':color_height},
            {'ir_width':ir_width},
            {'ir_height':ir_height},
            {'depth_fps':depth_fps},
            {'enable_pointcloud':enable_pointcloud},
            {'enable_align':enable_align},
            {'enable_denoise':enable_denoise},
            {'enable_device_timestamp':enable_device_timestamp},],
            output='screen'),

        # Rviz
        # Node(
        #     package='rviz2', node_executable='rviz2', output='screen',
        #     arguments=['--display-config', rviz_config_dir]),
    ])