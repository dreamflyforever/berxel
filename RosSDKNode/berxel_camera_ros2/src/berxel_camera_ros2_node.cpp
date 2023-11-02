#include "berxel_camera_ros2_driver.h"

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<berxel_ros2::BerxelCameraDriver>();
    node->init();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}