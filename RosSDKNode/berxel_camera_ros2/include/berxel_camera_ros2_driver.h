#ifndef _BERXEL_CAMERA_ROS2_DRIVER_H_
#define _BERXEL_CAMERA_ROS2_DRIVER_H_

#include <iostream>
#include <pthread.h>
#include <map>
#include <eigen3/Eigen/Geometry>
#include <builtin_interfaces/msg/time.hpp>
#include <console_bridge/console.h>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.hpp>

#include <rcl/time.h>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

#include <sensor_msgs/image_encodings.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/static_transform_broadcaster.h>

// #include <pcl_conversions/pcl_conversions.h>

#include <BerxelHawkContext.h>
#include <BerxelHawkDefines.h>
#include <BerxelHawkDevice.h>
#include <BerxelHawkFrame.h>

#include "berxel_camera_ros2_define.h"

using namespace berxel;

namespace berxel_ros2
{

class BerxelCameraDriver : public rclcpp::Node
{
public:
    BerxelCameraDriver();
    ~BerxelCameraDriver();

    int32_t init();

private:
    static void __attribute__((__stdcall__)) onDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, BerxelHawkDeviceStatus deviceState, void* pUserData);
	static void __attribute__((__stdcall__)) onNewFrameCallback(BerxelHawkStreamType streamType, BerxelHawkFrame* pFrame, void* pUserData);
    int processDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, BerxelHawkDeviceStatus deviceState);
    void berxelFrameCallback(BerxelHawkStreamType streamType, BerxelHawkFrame* pFrame);

	void getParameters();
	int32_t startCamera();
	void createPublishers();

	void destroyDevice();

	void berxelColorFrameCallback(BerxelHawkFrame* pFrame);
	void berxelDepthFrameCallback(BerxelHawkFrame* pFrame);
	void berxelIrFrameCallback(BerxelHawkFrame* pFrame);
	void berxelPublishTF();

	inline void convertDepthToCv16UC(uint16_t* pDepth, uint16_t* pCv16UC1,  int width, int height, BerxelHawkPixelType pixelType);

	void initCameraIntrinsic();
	void updateCameraInfo(berxel::BerxelHawkStreamType type);
	rclcpp::Time timesamp2rostime(uint64_t timesamp);

private:
	BerxelHawkDeviceIntrinsicParams		m_DeviceIntrinsicParams;
	BerxelHawkCameraIntrinsic 			m_rgbIntrinsicParams;
	BerxelHawkCameraIntrinsic 			m_irIntrinsicParams;
	BerxelHawkCameraIntrinsic 			m_depthIntrinsicParams;
	BerxelHawkCameraIntrinsic 			m_rotaParams;

	BerxelHawkContext*   				m_pContext = NULL;
	BerxelHawkDevice*    				m_pHawkDevice = NULL;
	BerxelHawkDeviceInfo 				m_CurrentDeviceInfo;

	pthread_mutex_t 							m_mutex_cb;
	rclcpp::Logger 								m_berxelLogger = rclcpp::get_logger("Berxel_Camera_Node");

	bool 										m_bEnableDepth = false;
	bool 										m_bEnableColor = false;
	bool 										m_bEnableIr	= false;
	bool										m_bEnablePointCloud = false;
	bool										m_bOrderedCloudPoint = false;
	bool										m_bEnableRegistration = false;
	bool										m_bEnableDenoise = false;
	bool										m_bEnableDeviceTimestamp = false;
	bool										m_bEnableTemperatureCompensation = false;
	bool										m_bEnableSetDepthConfidence = false;
	bool										m_bEnableDepthAE = true;
	bool										m_bEnableEdgeOptimization = false;
	bool										m_bEnableHightFpsMode = false;
	bool										m_bEnableAEGainRange = false;

	int											m_nDepthWidth = 640;
	int											m_nDepthHeight = 400;
	int											m_nColorWidth = 640;
	int											m_nColorHeight = 400;
	int											m_nIrWidth = 640;
	int											m_nIrHeight = 400;
	int											m_nDepthFps = 30;
	int											m_nStreamFlag = 0;
	int											m_nStreadmType = 0;
	int											m_nDeviceBus = 0;
	int											m_nDepthConfidence = 3;
	int											m_nDepthElectricCurrent = 15;
	int											m_nDepthExposureTime = 33;
	int											m_nDepthGain = 1;
	int											m_nDepthAEGainRangeMin = 1;
	int											m_nDepthAEGainRangeMax = 4;

    float 										m_camera_link_x;
	float 										m_camera_link_y;
	float 										m_camera_link_z;
	float 										m_camera_link_roll;
	float 										m_camera_link_pitch;
	float 										m_camera_link_yaw;
    float 										m_camera_rgb_frame_x;
	float 										m_camera_rgb_frame_y;
	float 										m_camera_rgb_frame_z;
	float 										m_camera_rgb_frame_roll;
	float 										m_camera_rgb_frame_pitch;
	float 										m_camera_rgb_frame_yaw;
    float 										m_camera_depth_frame_x;
	float 										m_camera_depth_frame_y;
	float 										m_camera_depth_frame_z;
	float 										m_camera_depth_frame_roll;
	float 										m_camera_depth_frame_pitch;
	float 										m_camera_depth_frame_yaw;
    float 										m_camera_rgb_optical_frame_x;
	float 										m_camera_rgb_optical_frame_y;
	float 										m_camera_rgb_optical_frame_z;
	float 										m_camera_rgb_optical_frame_roll;
	float 										m_camera_rgb_optical_frame_pitch;
	float 										m_camera_rgb_optical_frame_yaw;
    float 										m_camera_depth_optical_frame_x;
	float 										m_camera_depth_optical_frame_y;
	float 										m_camera_depth_optical_frame_z;
	float 										m_camera_depth_optical_frame_roll;
	float 										m_camera_depth_optical_frame_pitch;
	float 										m_camera_depth_optical_frame_yaw;

	std::string 								m_base_frame_id;
	std::string 								m_camera_frame_id;
	std::string 								m_depth_frame_id;
	std::string 								m_color_frame_id;
	std::string 								m_ir_frame_id;
	std::string 								m_depth_optical_frame_id;
	std::string 								m_color_optical_frame_id;
	std::string 								m_ir_optical_frame_id;
	std::string									m_str_serial_num;
	std::string 								m_str_device_name;
	std::string									m_str_device_port;
	rclcpp::Clock 								m_rosClock;

	berxel::BerxelHawkPoint3D* 					m_pPointClouds;

	image_transport::Publisher 					m_imgDepthPublish;
	image_transport::Publisher					m_imgColorPublish;
	image_transport::Publisher 					m_imgIrPublish;

	rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr 		m_pDepthInfoPublish;
	rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr 		m_pColorInfoPublish;
	rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr 		m_pIrInfoPublish;
	rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr 	m_pCloudPointPublish;

	std::shared_ptr<tf2_ros::StaticTransformBroadcaster> 			m_static_tf_broadcaster;

	//std::map<BerxelHawkStreamType, sensor_msgs::msg::CameraInfo::SharedPtr> m_cameraInfo;
	std::map<BerxelHawkStreamType, sensor_msgs::msg::CameraInfo> m_cameraInfo;
	std::map<BerxelHawkStreamType, BerxelHawkCameraIntrinsic> m_cameraIntrinsic;

	// image_transport::CameraPublisher 			m_imgDepthPublish;
	// image_transport::CameraPublisher 			m_imgColorPublish;
	// image_transport::CameraPublisher 			m_imgIrPublish;
};

}

#endif