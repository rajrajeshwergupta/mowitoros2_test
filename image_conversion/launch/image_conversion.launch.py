from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python import get_package_share_directory
import os

from pathlib import Path

def generate_launch_description():
    param_file = os.path.join(get_package_share_directory('image_conversion'), 'config', 'image_conversion.yaml')

    return LaunchDescription([
        Node(
            package='image_tools',
            executable='cam2image',
            name='cam2image',
            # parameters=[{'show_camera': False}],
            # remappings=[
            #     ('image', '/image_raw')  # Must match input_topic in the YAML
            # ]
        ),
        
        Node(
            package='image_conversion', 
            executable='image_conversion_node',
            name='image_conversion_node',
            parameters=[param_file]
        )
    ])
