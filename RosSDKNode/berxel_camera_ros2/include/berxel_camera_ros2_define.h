#ifndef _BERXEL_CAMERA_DEFINE_ROS2_H_
#define _BERXEL_CAMERA_DEFINE_ROS2_H_

#define BERXEL_ROS2_MAJOR_VERSION    2
#define BERXEL_ROS2_MINOR_VERSION    0
#define BERXEL_ROS2_PATCH_VERSION    1

#define STRINGIFY(arg) #arg
#define VAR_ARG_STRING(arg) STRINGIFY(arg)
#define BERXEL_ROS2_VERSION_STR (VAR_ARG_STRING(BERXEL_ROS2_MAJOR_VERSION. \
  BERXEL_ROS2_MINOR_VERSION.BERXEL_ROS2_PATCH_VERSION))

namespace berxel_ros2
{
    const bool CLOUDPOINT_ENABLE = true;
    // const bool DEPTH_ENABLE = true;
    // const bool COLOR_ENABLE = true;
    // const bool IR_ENABLE = false;
    const bool ALIGN_ENABLE = false;
    const bool ENABLE_DENOISE = false;
    const bool ENABLE_DEVICE_TIMESTAMP = false;
    const int DEPTH_WIDTH = 640;
    const int DEPTH_HEIGHT = 400;
    const int COLOR_WIDTH = 640;
    const int COLOR_HEIGHT = 400;
    const int IR_WIDTH = 640;
    const int IR_HEIGHT = 400;
    const int DEPTH_FPS = 30;
    const int STREAM_FLAG = 1;
    const int STREAM_TYPE = 2;
    
    const float CAMERA_LINK_X = 0;
    const float CAMERA_LINK_Y = 0.0152;
    const float CAMERA_LINK_Z = 0.008155;
    const float CAMERA_LINK_ROLL = 0;
    const float CAMERA_LINK_PITCH = 0;
    const float CAMERA_LINK_YAW = 0;

    const float CAMERA_RGB_X = 0.006;
    const float CAMERA_RGB_Y = 0;
    const float CAMERA_RGB_Z = 0;
    const float CAMERA_RGB_ROLL = 0;
    const float CAMERA_RGB_PITCH = 0;
    const float CAMERA_RGB_YAW = 0;

    const float CAMERA_DEPTH_X = -0.004;
    const float CAMERA_DEPTH_Y = 0;
    const float CAMERA_DEPTH_Z = 0;
    const float CAMERA_DEPTH_ROLL = 0;
    const float CAMERA_DEPTH_PITCH = 0;
    const float CAMERA_DEPTH_YAW = 0;

    const float CAMERA_RGB_OPTICAL_X = 0;
    const float CAMERA_RGB_OPTICAL_Y = 0;
    const float CAMERA_RGB_OPTICAL_Z = 0;
    const float CAMERA_RGB_OPTICAL_ROLL = -0.5;
    const float CAMERA_RGB_OPTICAL_PITCH = 0;
    const float CAMERA_RGB_OPTICAL_YAW = -0.5;

    const float CAMERA_DEPTH_OPTICAL_X = 0;
    const float CAMERA_DEPTH_OPTICAL_Y = 0;
    const float CAMERA_DEPTH_OPTICAL_Z = 0;
    const float CAMERA_DEPTH_OPTICAL_ROLL = -0.5;
    const float CAMERA_DEPTH_OPTICAL_PITCH = 0;
    const float CAMERA_DEPTH_OPTICAL_YAW = -0.5;

    const char DEFAULT_BASE_FRAME_ID[] = "base_link";
    const char DEFAULT_CAMERA_FRAME_ID[] = "camera_link";
    const char DEFAULT_DEPTH_FRAME_ID[] = "camera_depth_frame";
    const char DEFAULT_COLOR_FRAME_ID[] = "camera_color_frame";
    const char DEFAULT_IR_FRAME_ID[] = "camera_ir_frame";
    const char DEFAULT_DEPTH_OPTICAL_FRAME_ID[] = "camera_depth_optical_frame";
    const char DEFAULT_COLOR_OPTICAL_FRAME_ID[] = "camera_color_optical_frame";
    const char DEFAULT_IR_OPTICAL_FRAME_ID[] = "camera_ir_optical_frame";
    const char DEFAULT_DEVICE_NAME[] = "berxel_camera";
}

#endif