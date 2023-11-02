#include "berxel_camera_ros2_driver.h"
#include <csignal>

namespace berxel_ros2
{


inline void signalHandler(int signum)
{
	std::cout << strsignal(signum) << " Signal is received! Terminate Berxel Camera Node...\n";
	// destroyDevice();
	rclcpp::shutdown();
	exit(signum);
}

    
BerxelCameraDriver::BerxelCameraDriver() : 
	Node("berxel_camera_driver", rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true)),
	m_rosClock(RCL_ROS_TIME)

{
	RCLCPP_INFO(m_berxelLogger, "Berxel ROS2 v%s", BERXEL_ROS2_VERSION_STR);
	
	signal(SIGINT, signalHandler);

	m_pContext = berxel::BerxelHawkContext::getBerxelContext();
	if (m_pContext != NULL)
		m_pContext->setDeviceStateCallback(onDeviceStatusChange,this);
    pthread_mutex_init(&m_mutex_cb, NULL);
}

BerxelCameraDriver::~BerxelCameraDriver()
{
	destroyDevice();
    pthread_mutex_destroy(&m_mutex_cb);
	if (m_pPointClouds != NULL)
	{
		free(m_pPointClouds);
		m_pPointClouds = NULL;
	}
}

int32_t BerxelCameraDriver::init()
{
	if (m_pContext == NULL)
		return -1;
	RCLCPP_INFO(m_berxelLogger, "Berxel Camera Ros2 Node Init");
	getParameters();

	int32_t ret = 0;
	ret = startCamera();
	if (ret != 0)
	{
		RCLCPP_ERROR(m_berxelLogger, "start camera error");
		return ret;
	}
	// m_cameraInfo[BERXEL_HAWK_COLOR_STREAM] = std::make_shared<sensor_msgs::msg::CameraInfo>();
	// m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM] = std::make_shared<sensor_msgs::msg::CameraInfo>();
	// m_cameraInfo[BERXEL_HAWK_IR_STREAM] = std::make_shared<sensor_msgs::msg::CameraInfo>();

	return 0;
}

void BerxelCameraDriver::getParameters()
{
	// this->get_parameter_or("enable_depth", m_bEnableDepth, DEPTH_ENABLE);
	// this->get_parameter_or("enable_color", m_bEnableColor, COLOR_ENABLE);
	// this->get_parameter_or("enable_ir", m_bEnableIr, IR_ENABLE);

    this->get_parameter_or("enable_pointcloud", m_bEnablePointCloud, CLOUDPOINT_ENABLE);
	this->get_parameter_or("enable_align", m_bEnableRegistration, ALIGN_ENABLE);
	this->get_parameter_or("enable_denoise", m_bEnableDenoise, ENABLE_DENOISE);
	this->get_parameter_or("enable_device_timestamp", m_bEnableDeviceTimestamp, ENABLE_DEVICE_TIMESTAMP);
	this->get_parameter_or("enable_temperature_compensation", m_bEnableTemperatureCompensation, false);
	this->get_parameter_or("enable_ordered_pointcloud", m_bOrderedCloudPoint, true);
	this->get_parameter_or("enable_set_depth_Confidence", m_bEnableSetDepthConfidence, true);
	this->get_parameter_or("enable_depth_ae", m_bEnableDepthAE, true);
	this->get_parameter_or("enable_edge_optimization", m_bEnableEdgeOptimization, true);
	this->get_parameter_or("enable_hight_fps_mode", m_bEnableHightFpsMode, true);
	this->get_parameter_or("enable_adjust_ae_gain_range", m_bEnableAEGainRange, true);

	this->get_parameter_or("DeviceName", m_str_device_name, std::string(DEFAULT_DEVICE_NAME));
	this->get_parameter_or("serial_number", m_str_serial_num, std::string(""));
	this->get_parameter_or("device_port", m_str_device_port, std::string(""));
	this->get_parameter_or("device_bus", m_nDeviceBus, 0);

	this->get_parameter_or("depth_width", m_nDepthWidth, DEPTH_WIDTH);
	this->get_parameter_or("depth_height", m_nDepthHeight, DEPTH_HEIGHT);
	this->get_parameter_or("color_width", m_nColorWidth, COLOR_WIDTH);
	this->get_parameter_or("color_height", m_nColorHeight, COLOR_HEIGHT);
	this->get_parameter_or("ir_width", m_nIrWidth, IR_WIDTH);
	this->get_parameter_or("ir_height", m_nIrHeight, IR_HEIGHT);
	this->get_parameter_or("depth_fps", m_nDepthFps, DEPTH_FPS);
	this->get_parameter_or("stream_flag", m_nStreamFlag, STREAM_FLAG);
	this->get_parameter_or("stream_type", m_nStreadmType, STREAM_TYPE);

	this->get_parameter_or("depth_confidence", m_nDepthConfidence, 3);
	this->get_parameter_or("depth_current", m_nDepthElectricCurrent, 15);
	this->get_parameter_or("depth_exposure_time", m_nDepthExposureTime, 33);
	this->get_parameter_or("depth_gain", m_nDepthGain, 1);
	this->get_parameter_or("depth_ae_gain_range_min", m_nDepthAEGainRangeMin, 1);
	this->get_parameter_or("depth_ae_gain_range_max", m_nDepthAEGainRangeMax, 4);

	this->get_parameter_or("camera_link_x", m_camera_link_x, CAMERA_LINK_X);
	this->get_parameter_or("camera_link_y", m_camera_link_y, CAMERA_LINK_Y);
	this->get_parameter_or("camera_link_z", m_camera_link_z, CAMERA_LINK_Z);
	this->get_parameter_or("camera_link_roll", m_camera_link_roll, CAMERA_LINK_ROLL);
	this->get_parameter_or("camera_link_pitch", m_camera_link_pitch, CAMERA_LINK_PITCH);
	this->get_parameter_or("camera_link_yaw", m_camera_link_yaw, CAMERA_LINK_YAW);

	this->get_parameter_or("camera_rgb_frame_x", m_camera_rgb_frame_x, CAMERA_RGB_X);
	this->get_parameter_or("camera_rgb_frame_y", m_camera_rgb_frame_y, CAMERA_RGB_Y);
	this->get_parameter_or("camera_rgb_frame_z", m_camera_rgb_frame_z, CAMERA_RGB_Z);
	this->get_parameter_or("camera_rgb_frame_roll", m_camera_rgb_frame_roll, CAMERA_RGB_ROLL);
	this->get_parameter_or("camera_rgb_frame_pitch", m_camera_rgb_frame_pitch, CAMERA_RGB_PITCH);
	this->get_parameter_or("camera_rgb_frame_yaw", m_camera_rgb_frame_yaw, CAMERA_RGB_YAW);

	this->get_parameter_or("camera_depth_frame_x", m_camera_depth_frame_x, CAMERA_DEPTH_X);
	this->get_parameter_or("camera_depth_frame_y", m_camera_depth_frame_y, CAMERA_DEPTH_Y);
	this->get_parameter_or("camera_depth_frame_z", m_camera_depth_frame_z, CAMERA_DEPTH_Z);
	this->get_parameter_or("camera_depth_frame_roll", m_camera_depth_frame_roll, CAMERA_DEPTH_ROLL);
	this->get_parameter_or("camera_depth_frame_pitch", m_camera_depth_frame_pitch, CAMERA_DEPTH_PITCH);
	this->get_parameter_or("camera_depth_frame_yaw", m_camera_depth_frame_yaw, CAMERA_DEPTH_YAW);

	this->get_parameter_or("camera_rgb_optical_frame_x", m_camera_rgb_optical_frame_x, CAMERA_RGB_OPTICAL_X);
	this->get_parameter_or("camera_rgb_optical_frame_y", m_camera_rgb_optical_frame_y, CAMERA_RGB_OPTICAL_Y);
	this->get_parameter_or("camera_rgb_optical_frame_z", m_camera_rgb_optical_frame_z, CAMERA_RGB_OPTICAL_Z);
	this->get_parameter_or("camera_rgb_optical_frame_roll", m_camera_rgb_optical_frame_roll, CAMERA_RGB_OPTICAL_ROLL);
	this->get_parameter_or("camera_rgb_optical_frame_pitch", m_camera_rgb_optical_frame_pitch, CAMERA_RGB_OPTICAL_PITCH);
	this->get_parameter_or("camera_rgb_optical_frame_yaw", m_camera_rgb_optical_frame_yaw, CAMERA_RGB_OPTICAL_YAW);

	this->get_parameter_or("camera_depth_optical_frame_x", m_camera_depth_optical_frame_x, CAMERA_DEPTH_OPTICAL_X);
	this->get_parameter_or("camera_depth_optical_frame_y", m_camera_depth_optical_frame_y, CAMERA_DEPTH_OPTICAL_Y);
	this->get_parameter_or("camera_depth_optical_frame_z", m_camera_depth_optical_frame_z, CAMERA_DEPTH_OPTICAL_Z);
	this->get_parameter_or("camera_depth_optical_frame_roll", m_camera_depth_optical_frame_roll, CAMERA_DEPTH_OPTICAL_ROLL);
	this->get_parameter_or("camera_depth_optical_frame_pitch", m_camera_depth_optical_frame_pitch, CAMERA_DEPTH_OPTICAL_PITCH);
	this->get_parameter_or("camera_depth_optical_frame_yaw", m_camera_depth_optical_frame_yaw, CAMERA_DEPTH_OPTICAL_YAW);

	m_base_frame_id = m_str_device_name + "/berxel_base_link";
	m_camera_frame_id = m_str_device_name + "/berxel_camera_link";
	m_color_frame_id = m_str_device_name + "/berxel_camera_color_frame";
	m_depth_frame_id = m_str_device_name + "/berxel_camera_depth_frame";
	m_ir_frame_id = m_str_device_name + "/berxel_camera_ir_frame";
	m_color_optical_frame_id = m_str_device_name + "/berxel_camera_color_optical_frame";
	m_depth_optical_frame_id = m_str_device_name + "/berxel_camera_depth_optical_frame";
	m_ir_optical_frame_id = m_str_device_name + "/berxel_camera_ir_optical_frame";


	m_pPointClouds = (berxel::BerxelHawkPoint3D*)malloc(m_nDepthWidth * m_nDepthHeight * sizeof(berxel::BerxelHawkPoint3D));
	// this->get_parameter_or("base_frame_id", m_base_frame_id, std::string(DEFAULT_BASE_FRAME_ID));
	// this->get_parameter_or("camera_frame_id", m_camera_frame_id, std::string(DEFAULT_CAMERA_FRAME_ID));
	// this->get_parameter_or("depth_frame_id", m_depth_frame_id, std::string(DEFAULT_DEPTH_FRAME_ID));
	// this->get_parameter_or("ir_frame_id", m_ir_frame_id, std::string(DEFAULT_IR_FRAME_ID));
	// this->get_parameter_or("color_frame_id", m_color_frame_id, std::string(DEFAULT_COLOR_FRAME_ID));
	// this->get_parameter_or("depth_optical_frame_id", m_depth_optical_frame_id, std::string(DEFAULT_DEPTH_OPTICAL_FRAME_ID));
	// this->get_parameter_or("ir_optical_frame_id", m_ir_optical_frame_id, std::string(DEFAULT_IR_OPTICAL_FRAME_ID));
	// this->get_parameter_or("color_optical_frame_id", m_color_optical_frame_id, std::string(DEFAULT_COLOR_OPTICAL_FRAME_ID));
}

int32_t BerxelCameraDriver::startCamera()
{
	RCLCPP_INFO(m_berxelLogger, "Start Berxel Device");
	berxel::BerxelHawkDeviceInfo* pDeviceInfo = NULL;
	uint32_t deviceCount = 0;
	m_pContext->getDeviceList(&pDeviceInfo, &deviceCount);
	if((deviceCount <= 0) || (NULL == pDeviceInfo))
	{
		RCLCPP_ERROR(m_berxelLogger, "Get No Connected BerxelDevice");
		return -1;
	}
	memset(&m_CurrentDeviceInfo, 0x00, sizeof(m_CurrentDeviceInfo));
	if (m_str_device_port.empty()) {
		if (m_str_serial_num.empty()) {
			m_CurrentDeviceInfo = pDeviceInfo[0];
		}
		else {
			for (int i = 0; i < deviceCount; i++) {
				if (strcmp(pDeviceInfo[i].serialNumber, m_str_serial_num.c_str()) == 0) {
					m_CurrentDeviceInfo = pDeviceInfo[i];
					break;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < deviceCount; i++) 
		{
			if (strcmp(pDeviceInfo[i].devicePort, m_str_device_port.c_str()) == 0 && (m_nDeviceBus == pDeviceInfo[i].devBus)) 
			{
				m_CurrentDeviceInfo = pDeviceInfo[i];
				break;
			}
		}
	}

	if (m_CurrentDeviceInfo.vendorId == 0x00)
	{
		RCLCPP_ERROR(m_berxelLogger, "Get Current Device Info Error !!!");
		return -1;
	}

	RCLCPP_INFO(m_berxelLogger, "Will Open Device : %s", m_CurrentDeviceInfo.serialNumber);
	m_pHawkDevice = m_pContext->openDevice(m_CurrentDeviceInfo);
	if(NULL == m_pHawkDevice)
	{       
		RCLCPP_ERROR(m_berxelLogger, "Open BerxelDevice Failed");
		return -1;
	}

	memset((uint8_t*)&m_DeviceIntrinsicParams, 0x00, sizeof(berxel::BerxelHawkDeviceIntrinsicParams));
	m_pHawkDevice->getDeviceIntriscParams(&m_DeviceIntrinsicParams);
	memcpy((uint8_t*)&m_rgbIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.colorIntrinsicParams, sizeof(berxel::BerxelHawkCameraIntrinsic));
	memcpy((uint8_t*)&m_irIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.irIntrinsicParams, sizeof(berxel::BerxelHawkCameraIntrinsic));
	memcpy((uint8_t*)&m_depthIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.liteIrIntrinsicParams, sizeof(berxel::BerxelHawkCameraIntrinsic));
	memcpy((uint8_t*)&m_rotaParams, (uint8_t*)&m_DeviceIntrinsicParams.rotateIntrinsicParams, sizeof(berxel::BerxelHawkCameraIntrinsic));



	m_pHawkDevice->setSystemClock();
	m_pHawkDevice->setDenoiseStatus(m_bEnableDenoise);
	m_pHawkDevice->setRegistrationEnable(m_bEnableRegistration);

	m_pHawkDevice->setTemperatureCompensationEnable(m_bEnableTemperatureCompensation);
	//set depth confidence
	if (m_bEnableSetDepthConfidence) {
		int ret = m_pHawkDevice->setDepthConfidence(m_nDepthConfidence);
		RCLCPP_INFO(m_berxelLogger, "Set Depth Confidence : %d, ret : %d", m_nDepthConfidence, ret);
	}

	//set depth current
	m_pHawkDevice->setDepthElectricCurrent(m_nDepthElectricCurrent * 100);

	//set depth auto exposure status
	uint32_t depthAeStatus = 0;
	m_pHawkDevice->getDepthAEStatus(&depthAeStatus);
	bool enable_ae = depthAeStatus ? true : false;
	if (enable_ae != m_bEnableDepthAE)
	{
		RCLCPP_INFO(m_berxelLogger, "Set AE Status : %d", m_bEnableDepthAE);
		m_pHawkDevice->setDepthAEStatus(m_bEnableDepthAE);
	}

	if (m_bEnableDepthAE == false)
	{
		m_pHawkDevice->setDepthExposure(m_nDepthExposureTime);
		m_pHawkDevice->setDepthGain(m_nDepthGain);
	}

	//set edge optimization status
	m_pHawkDevice->setEdgeOptimizationStatus(m_bEnableEdgeOptimization);

	//set hight fps mode
	m_pHawkDevice->enableHightFpsMode(m_bEnableHightFpsMode);

	if (m_bEnableAEGainRange && m_bEnableDepthAE)
	{
		m_pHawkDevice->setDepthAEGainRange(m_nDepthAEGainRangeMin, m_nDepthAEGainRangeMax);
	}
	

	initCameraIntrinsic();
	updateCameraInfo(BERXEL_HAWK_COLOR_STREAM);
	updateCameraInfo(BERXEL_HAWK_DEPTH_STREAM);
	updateCameraInfo(BERXEL_HAWK_IR_STREAM);

	createPublishers();

	RCLCPP_INFO(m_berxelLogger, "m_nStreamFlag : %d, m_nStreadmType : %d", m_nStreamFlag, m_nStreadmType);
	m_pHawkDevice->setStreamFlagMode((berxel::BerxelHawkStreamFlagMode)m_nStreamFlag);

	if (m_nStreadmType & BERXEL_HAWK_COLOR_STREAM)
	{
		berxel::BerxelHawkStreamFrameMode colorFrameMode;
		m_pHawkDevice->getCurrentFrameMode(berxel::BERXEL_HAWK_COLOR_STREAM , &colorFrameMode);
		colorFrameMode.resolutionX = m_nColorWidth;
		colorFrameMode.resolutionY = m_nColorHeight;
		m_pHawkDevice->setFrameMode(berxel::BERXEL_HAWK_COLOR_STREAM, &colorFrameMode);
	}

	if (m_nStreadmType & BERXEL_HAWK_DEPTH_STREAM)
	{
		berxel::BerxelHawkStreamFrameMode depthFrameMode;
		m_pHawkDevice->getCurrentFrameMode(berxel::BERXEL_HAWK_DEPTH_STREAM , &depthFrameMode);
		depthFrameMode.resolutionX = m_nDepthWidth;
		depthFrameMode.resolutionY = m_nDepthHeight;
		depthFrameMode.framerate = m_nDepthFps;
		m_pHawkDevice->setFrameMode(berxel::BERXEL_HAWK_DEPTH_STREAM, &depthFrameMode);
	}

    int ret = m_pHawkDevice->startStreams(m_nStreadmType, onNewFrameCallback, this);
	if(ret != 0)
	{
		RCLCPP_ERROR(m_berxelLogger, "Open Berxel Stream Failed");
		return -1;
	}

	return 0;
}

void BerxelCameraDriver::createPublishers()
{
	// rmw_qos_profile_t custom_qos_profile = rmw_qos_profile_sensor_data;
	//m_imgDepthPublish = image_transport::create_camera_publisher(this, "camera/depth/depth_raw");
	
	if (m_nStreadmType & berxel::BERXEL_HAWK_DEPTH_STREAM) {
		m_imgDepthPublish = image_transport::create_publisher(this, "depth/depth_raw");
		m_pDepthInfoPublish = this->create_publisher<sensor_msgs::msg::CameraInfo>("depth/camera_info", 1);
		if (m_bEnablePointCloud)
			m_pCloudPointPublish = this->create_publisher<sensor_msgs::msg::PointCloud2>("berxel_cloudpoint", 1);
	}

	if (m_nStreadmType & berxel::BERXEL_HAWK_COLOR_STREAM) {
		//m_imgColorPublish = image_transport::create_camera_publisher(this, "camera/color/color_raw", custom_qos_profile);
		m_imgColorPublish = image_transport::create_publisher(this, "color/color_raw");
		m_pColorInfoPublish = this->create_publisher<sensor_msgs::msg::CameraInfo>("color/camera_info", 1);
	}


	if (m_nStreadmType & berxel::BERXEL_HAWK_IR_STREAM || m_nStreadmType & berxel::BERXEL_HAWK_LIGHT_IR_STREAM) {
		//m_imgIrPublish = image_transport::create_camera_publisher(this, "camera/ir/ir_raw", custom_qos_profile);
		m_imgIrPublish = image_transport::create_publisher(this, "ir/color_raw");
		m_pIrInfoPublish = this->create_publisher<sensor_msgs::msg::CameraInfo>("ir/camera_info", 1);
	}

	m_static_tf_broadcaster = std::make_shared<tf2_ros::StaticTransformBroadcaster>(shared_from_this());
}

void BerxelCameraDriver::destroyDevice()
{
	if(m_pHawkDevice)
	{
		RCLCPP_INFO(m_berxelLogger, "stop stream!!!");
		m_pHawkDevice->stopStreams(m_nStreadmType);
	}

	if(m_pContext)
	{
		RCLCPP_INFO(m_berxelLogger, "close device!!!");
		m_pContext->closeDevice(m_pHawkDevice);
        berxel::BerxelHawkContext::destroyBerxelContext(m_pContext);
		m_pContext = NULL;
		m_pHawkDevice = NULL;
	}
}

void __attribute__((__stdcall__)) BerxelCameraDriver::onNewFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame, void* pUserData)
{
	if (NULL != pUserData)
	{
		BerxelCameraDriver* pBerxelCamera = static_cast<BerxelCameraDriver*>(pUserData);
		if(NULL != pBerxelCamera) 
		{
			pBerxelCamera->berxelFrameCallback(streamType, pFrame);
		}
	}
}

void BerxelCameraDriver::berxelFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame)
{
	// pthread_mutex_lock(&m_mutex_cb);
	switch (streamType)
	{
	case berxel::BERXEL_HAWK_COLOR_STREAM:
		{
			berxelColorFrameCallback(pFrame);
		}
		break;
	case berxel::BERXEL_HAWK_DEPTH_STREAM:
		{
			berxelDepthFrameCallback(pFrame);
		}
		break;
	case berxel::BERXEL_HAWK_IR_STREAM:
		{
			berxelIrFrameCallback(pFrame);
		}
		break;
	default:
		break;
	}

	m_pHawkDevice->releaseFrame(pFrame);
	berxelPublishTF();

	// pthread_mutex_unlock(&m_mutex_cb);
}

void __attribute__((__stdcall__)) BerxelCameraDriver::onDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState, void* pUserData)
{
	if(NULL != pUserData)
	{
		BerxelCameraDriver* pBerxelCamera = static_cast<BerxelCameraDriver*>(pUserData);
		if(NULL != pBerxelCamera) 
		{
			pBerxelCamera->processDeviceStatusChange(deviceUri, deviceSerialNumber, deviceState);
		}
	}
}

int32_t BerxelCameraDriver::processDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState)
{
	switch(deviceState) 
	{
    case berxel::BERXEL_HAWK_DEVICE_DISCONNECT:
		{
			RCLCPP_INFO(m_berxelLogger, "Device %s Disconnected", deviceSerialNumber);
			if (strcmp(m_CurrentDeviceInfo.deviceAddress, deviceUri) == 0 && m_pHawkDevice != NULL)
			{
				m_pHawkDevice->stopStreams(m_nStreadmType);
				m_pContext->closeDevice(m_pHawkDevice);
				m_pHawkDevice = NULL;
			}
		}
		break;
	case berxel::BERXEL_HAWK_DEVICE_CONNECT:
		{
			RCLCPP_INFO(m_berxelLogger, "Device %s Connected", deviceSerialNumber);
			if (strcmp(m_CurrentDeviceInfo.serialNumber, deviceSerialNumber) == 0)
			{
				init();
			}
			
		}
		break;
	default:
		break;
	}

	return 0;
}

rclcpp::Time BerxelCameraDriver::timesamp2rostime(uint64_t timesamp){
	// std::string suanz = std::to_string(timesamp);
	// std::string sec_string = suanz.substr(0,10);
	// std::string nsec_string = suanz.substr(10,9);
	// while(nsec_string.length() < 9){
	// 	nsec_string += "0";
	// }
	// return rclcpp::Time(std::stol(sec_string),std::stol(nsec_string));

	uint32_t tempSec = (uint32_t)(timesamp / 1000 / 1000);
	uint32_t tempNsec = (uint32_t)(timesamp % 1000000) * 1000;
	return rclcpp::Time(tempSec,tempNsec);
}

void BerxelCameraDriver::berxelColorFrameCallback(berxel::BerxelHawkFrame* pFrame)
{
	if (pFrame == NULL)
		return;

	auto color_image = std::make_shared<sensor_msgs::msg::Image>();
	color_image->header.frame_id = m_color_frame_id;
	if (m_bEnableDeviceTimestamp) {
		color_image->header.stamp = timesamp2rostime(pFrame->getTimeStamp());
	}
	else {
		color_image->header.stamp = this->get_clock()->now();
	}
	
	color_image->height = pFrame->getHeight();
	color_image->width = pFrame->getWidth();
	color_image->encoding = "rgb8";

	color_image->is_bigendian = false;
	color_image->step = sizeof(unsigned char) * 3 * color_image->width;
	color_image->data.resize(pFrame->getDataSize());

	memcpy(reinterpret_cast<char *>(&color_image->data[0]), pFrame->getData(), pFrame->getDataSize());
	m_cameraInfo[BERXEL_HAWK_COLOR_STREAM].width  = color_image->width;
	m_cameraInfo[BERXEL_HAWK_COLOR_STREAM].height = color_image->height;
	m_cameraInfo[BERXEL_HAWK_COLOR_STREAM].header.frame_id = m_color_frame_id;
	m_cameraInfo[BERXEL_HAWK_COLOR_STREAM].header.stamp = color_image->header.stamp;
	//m_imgColorPublish.publish(color_image, m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]);
	m_pColorInfoPublish->publish(m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]);
	m_imgColorPublish.publish(color_image);

}

void BerxelCameraDriver::berxelDepthFrameCallback(berxel::BerxelHawkFrame* pFrame)
{
	if (pFrame == NULL)
		return;

	rclcpp::Time stamp = this->get_clock()->now();

	auto depth_image = std::make_shared<sensor_msgs::msg::Image>();
	depth_image->header.frame_id = m_depth_optical_frame_id;
	depth_image->header.stamp = stamp;
	if (m_bEnableDeviceTimestamp) {
		depth_image->header.stamp = timesamp2rostime(pFrame->getTimeStamp());
	}
	else {
		depth_image->header.stamp = this->get_clock()->now();
	}
	depth_image->height = pFrame->getHeight();
	depth_image->width = pFrame->getWidth();
	depth_image->encoding = sensor_msgs::image_encodings::MONO16;

	depth_image->is_bigendian = false;
	depth_image->step = sizeof(unsigned char) * 2 * depth_image->width;
	depth_image->data.resize(pFrame->getDataSize());

	convertDepthToCv16UC((uint16_t*)pFrame->getData(), (uint16_t*)&depth_image->data[0], pFrame->getWidth(), pFrame->getHeight(), pFrame->getPixelType());
	uint16_t* pData = (uint16_t*)pFrame->getData();
	m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM].width  = depth_image->width;
	m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM].binning_x = depth_image->height;
	m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM].header.frame_id = m_depth_optical_frame_id;
	m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM].header.stamp = depth_image->header.stamp;
	//m_imgDepthPublish.publish(depth_image, m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]);
	m_pDepthInfoPublish->publish(m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]);
	m_imgDepthPublish.publish(depth_image);

	if (m_bEnablePointCloud)
	{
		sensor_msgs::msg::PointCloud2 msg_pointcloud;
		msg_pointcloud.header.stamp = stamp;
		msg_pointcloud.header.frame_id = m_depth_optical_frame_id;
		msg_pointcloud.width = pFrame->getWidth();
		msg_pointcloud.height = pFrame->getHeight();
		msg_pointcloud.is_dense = true;

		sensor_msgs::PointCloud2Modifier modifier(msg_pointcloud);

		modifier.setPointCloud2Fields(4,
			"x", 1, sensor_msgs::msg::PointField::FLOAT32,
			"y", 1, sensor_msgs::msg::PointField::FLOAT32,
			"z", 1, sensor_msgs::msg::PointField::FLOAT32,
			"rgb", 1, sensor_msgs::msg::PointField::FLOAT32);
		modifier.setPointCloud2FieldsByString(2, "xyz", "rgb");

		sensor_msgs::PointCloud2Iterator<float> iter_x(msg_pointcloud, "x");
		sensor_msgs::PointCloud2Iterator<float> iter_y(msg_pointcloud, "y");
		sensor_msgs::PointCloud2Iterator<float> iter_z(msg_pointcloud, "z");

		sensor_msgs::PointCloud2Iterator<uint8_t> iter_r(msg_pointcloud, "r");
		sensor_msgs::PointCloud2Iterator<uint8_t> iter_g(msg_pointcloud, "g");
		sensor_msgs::PointCloud2Iterator<uint8_t> iter_b(msg_pointcloud, "b");

		m_pHawkDevice->convertDepthToPointCloud(pFrame, 1000.0, m_pPointClouds);

		// for (int i = 0; i < pFrame->getHeight(); i++)
		// {
		// 	for (int j = 0; j < pFrame->getWidth(); j++)
		// 	{
		// 		*iter_x = pointClouds[].x;
		// 		*iter_y = pointClouds.y;
		// 		*iter_z = pointClouds.z;
		// 	}
		// }
		uint32_t valid_count = 0;
		int nSize = pFrame->getHeight() * pFrame->getWidth();
		for (int nIndex = 0; nIndex < nSize; nIndex++)
		{

			bool valid_pixel(m_pPointClouds[nIndex].z > 0);
			if (valid_pixel || m_bOrderedCloudPoint)
			{
				if (valid_pixel == 0) {
					*iter_x = *iter_y = *iter_z = 0xFFFFFFFF;
					*iter_r = *iter_g = *iter_b = 0;
				} else {
					*iter_x = m_pPointClouds[nIndex].x;
					*iter_y = m_pPointClouds[nIndex].y;
					*iter_z = m_pPointClouds[nIndex].z;
					*iter_r = static_cast<uint8_t>(96);
					*iter_g = static_cast<uint8_t>(157);
					*iter_b = static_cast<uint8_t>(198);
				}

				valid_count++;
				++iter_x; ++iter_y; ++iter_z;
				++iter_r; ++iter_g; ++iter_b;
			}
		}

		if (!m_bOrderedCloudPoint)
		{
			msg_pointcloud.width = valid_count;
			msg_pointcloud.height = 1;
			msg_pointcloud.is_dense = true;
			modifier.resize(valid_count);
		}

		m_pCloudPointPublish->publish(msg_pointcloud);
	}
}

void BerxelCameraDriver::berxelIrFrameCallback(berxel::BerxelHawkFrame* pFrame)
{
	if (pFrame == NULL)
		return;

	auto ir_image = std::make_shared<sensor_msgs::msg::Image>();
	ir_image->header.frame_id = m_ir_frame_id;
	// ir_image->header.stamp = this->get_clock()->now();
	if (m_bEnableDeviceTimestamp) {
		ir_image->header.stamp = timesamp2rostime(pFrame->getTimeStamp());
	}
	else {
		ir_image->header.stamp = this->get_clock()->now();
	}
	ir_image->height = pFrame->getHeight();
	ir_image->width = pFrame->getWidth();
	ir_image->encoding = "mono16";

	ir_image->is_bigendian = false;
	ir_image->step = sizeof(unsigned char) * 2 * ir_image->width;
	ir_image->data.resize(pFrame->getDataSize());

	memcpy(reinterpret_cast<char *>(&ir_image->data[0]), pFrame->getData(), pFrame->getDataSize());
	m_cameraInfo[BERXEL_HAWK_IR_STREAM].width  = ir_image->width;
	m_cameraInfo[BERXEL_HAWK_IR_STREAM].height = ir_image->height;
	m_cameraInfo[BERXEL_HAWK_IR_STREAM].header.frame_id = m_ir_frame_id;
	m_cameraInfo[BERXEL_HAWK_IR_STREAM].header.stamp = ir_image->header.stamp;
	// m_imgIrPublish.publish(ir_image, m_cameraInfo[BERXEL_HAWK_IR_STREAM]);
	m_pIrInfoPublish->publish(m_cameraInfo[BERXEL_HAWK_IR_STREAM]);
	m_imgIrPublish.publish(ir_image);
}

void BerxelCameraDriver::convertDepthToCv16UC(uint16_t* pDepth, uint16_t* pCv16UC1,  int width, int height, berxel::BerxelHawkPixelType pixelType)
{
	if(pixelType == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) {
		for(uint32_t i = 0; i < width * height; ++i)
			pCv16UC1[i] = pDepth[i] >> 3;
	} else {
		for(uint32_t i = 0; i < width * height; ++i)
			pCv16UC1[i] = pDepth[i] >> 4;
	}
}

void BerxelCameraDriver::berxelPublishTF()
{
    tf2::Quaternion q_c2co;
    geometry_msgs::msg::TransformStamped b2c_msg;
    geometry_msgs::msg::TransformStamped c2co_msg;
    tf2::Quaternion ir1_2_ir1o;
    geometry_msgs::msg::TransformStamped b2ir1_msg;
    geometry_msgs::msg::TransformStamped ir1_2_ir1o_msg;

    rclcpp::Time transform_ts_ = this->get_clock()->now();
    tf2::Quaternion q_camera;
    geometry_msgs::msg::TransformStamped camera_msg;
    camera_msg.header.stamp = transform_ts_;
    camera_msg.header.frame_id = m_base_frame_id;
    camera_msg.child_frame_id = m_camera_frame_id;
    camera_msg.transform.translation.x = m_camera_link_x;
    camera_msg.transform.translation.y = m_camera_link_y;
    camera_msg.transform.translation.z = m_camera_link_z;
    camera_msg.transform.rotation.x = 0;
    camera_msg.transform.rotation.y = 0;
    camera_msg.transform.rotation.z = 0;
    camera_msg.transform.rotation.w = 1;
    m_static_tf_broadcaster->sendTransform(camera_msg);

    tf2::Quaternion q_rgb;
    geometry_msgs::msg::TransformStamped rgb_msg;
    geometry_msgs::msg::TransformStamped rgb_optical_msg;

    rgb_msg.header.stamp = transform_ts_;
    rgb_msg.header.frame_id = m_camera_frame_id;
    rgb_msg.child_frame_id = m_color_frame_id;
    rgb_msg.transform.translation.x = m_camera_rgb_frame_x;
    rgb_msg.transform.translation.y = m_camera_rgb_frame_y;
    rgb_msg.transform.translation.z = m_camera_rgb_frame_z;
    rgb_msg.transform.rotation.x = 0;
    rgb_msg.transform.rotation.y = 0;
    rgb_msg.transform.rotation.z = 0;
    rgb_msg.transform.rotation.w = 1;
    m_static_tf_broadcaster->sendTransform(rgb_msg);

    q_rgb.setRPY(m_camera_rgb_optical_frame_roll * M_PI, m_camera_rgb_optical_frame_pitch * M_PI, m_camera_rgb_optical_frame_yaw * M_PI);
    rgb_optical_msg.header.stamp = transform_ts_;
    rgb_optical_msg.header.frame_id = m_color_frame_id;
    rgb_optical_msg.child_frame_id = m_color_optical_frame_id;
    rgb_optical_msg.transform.translation.x = 0;
    rgb_optical_msg.transform.translation.y = 0;
    rgb_optical_msg.transform.translation.z = 0;
    rgb_optical_msg.transform.rotation.x = q_rgb.getX();
    rgb_optical_msg.transform.rotation.y = q_rgb.getY();
    rgb_optical_msg.transform.rotation.z = q_rgb.getZ();
    rgb_optical_msg.transform.rotation.w = q_rgb.getW();
    m_static_tf_broadcaster->sendTransform(rgb_optical_msg);


	tf2::Quaternion q_depth;
	geometry_msgs::msg::TransformStamped depth_msg;
	geometry_msgs::msg::TransformStamped depth_optical_msg;

	depth_msg.header.stamp = transform_ts_;
    depth_msg.header.frame_id = m_camera_frame_id;
    depth_msg.child_frame_id = m_depth_frame_id;
    depth_msg.transform.translation.x = m_camera_depth_frame_x;
    depth_msg.transform.translation.y = m_camera_depth_frame_y;
    depth_msg.transform.translation.z = m_camera_depth_frame_z;
    depth_msg.transform.rotation.x = 0;
    depth_msg.transform.rotation.y = 0;
    depth_msg.transform.rotation.z = 0;
    depth_msg.transform.rotation.w = 1;
    m_static_tf_broadcaster->sendTransform(depth_msg);

    q_depth.setRPY(m_camera_depth_optical_frame_roll * M_PI, m_camera_depth_optical_frame_pitch * M_PI, m_camera_depth_optical_frame_yaw * M_PI);
    depth_optical_msg.header.stamp = transform_ts_;
    depth_optical_msg.header.frame_id = m_depth_frame_id;
    depth_optical_msg.child_frame_id = m_depth_optical_frame_id;
    depth_optical_msg.transform.translation.x = 0;
    depth_optical_msg.transform.translation.y = 0;
    depth_optical_msg.transform.translation.z = 0;
    depth_optical_msg.transform.rotation.x = q_depth.getX();
    depth_optical_msg.transform.rotation.y = q_depth.getY();
    depth_optical_msg.transform.rotation.z = q_depth.getZ();
    depth_optical_msg.transform.rotation.w = q_depth.getW();
    m_static_tf_broadcaster->sendTransform(depth_optical_msg);
}

void BerxelCameraDriver::initCameraIntrinsic()
{
	uint32_t nValue = 1.0;
	if (m_nColorWidth == 640 || m_nColorHeight == 640)
		nValue = 2.0;
	else if (m_nColorWidth == 320 || m_nColorHeight == 320)
		nValue = 4.0;

	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].fxParam = m_rgbIntrinsicParams.fxParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].fyParam = m_rgbIntrinsicParams.fyParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].cxParam = m_rgbIntrinsicParams.cxParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].cyParam = m_rgbIntrinsicParams.cyParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].k1Param = m_rgbIntrinsicParams.k1Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].k2Param = m_rgbIntrinsicParams.k2Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].p1Param = m_rgbIntrinsicParams.p1Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].p2Param = m_rgbIntrinsicParams.p2Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_COLOR_STREAM].k3Param = m_rgbIntrinsicParams.k3Param / nValue;

	nValue = 1.0;
	if (m_nDepthWidth == 640 || m_nDepthHeight == 640)
		nValue = 2.0;
	else if (m_nDepthWidth == 320 || m_nDepthHeight == 320)
		nValue = 4.0;

	if (m_bEnableRegistration)
	{
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fxParam = m_rgbIntrinsicParams.fxParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fyParam = m_rgbIntrinsicParams.fyParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cxParam = m_rgbIntrinsicParams.cxParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cyParam = m_rgbIntrinsicParams.cyParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k1Param = m_rgbIntrinsicParams.k1Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k2Param = m_rgbIntrinsicParams.k2Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].p1Param = m_rgbIntrinsicParams.p1Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].p2Param = m_rgbIntrinsicParams.p2Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k3Param = m_rgbIntrinsicParams.k3Param / nValue;		
	}
	else
	{
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fxParam = m_depthIntrinsicParams.fxParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fyParam = m_depthIntrinsicParams.fyParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cxParam = m_depthIntrinsicParams.cxParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cyParam = m_depthIntrinsicParams.cyParam / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k1Param = m_depthIntrinsicParams.k1Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k2Param = m_depthIntrinsicParams.k2Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].p1Param = m_depthIntrinsicParams.p1Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].p2Param = m_depthIntrinsicParams.p2Param / nValue;
		m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].k3Param = m_depthIntrinsicParams.k3Param / nValue;	
	}

	if (m_nStreadmType == 0x20)
		nValue = 1;
	else
		nValue = 2;

	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].fxParam = m_irIntrinsicParams.fxParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].fyParam = m_irIntrinsicParams.fyParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].cxParam = m_irIntrinsicParams.cxParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].cyParam = m_irIntrinsicParams.cyParam / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k1Param = m_irIntrinsicParams.k1Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k2Param = m_irIntrinsicParams.k2Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].p1Param = m_irIntrinsicParams.p1Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].p2Param = m_irIntrinsicParams.p2Param / nValue;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k3Param = m_irIntrinsicParams.k3Param / nValue;	
}

void BerxelCameraDriver::updateCameraInfo(berxel::BerxelHawkStreamType type)
{
	m_cameraInfo[type].distortion_model = "plumb_bob";
	m_cameraInfo[type].binning_x = m_cameraInfo[type].binning_y = 1;

	m_cameraInfo[type].d.resize(5);
	m_cameraInfo[type].d.at(0) = m_cameraIntrinsic[type].k1Param;
	m_cameraInfo[type].d.at(1) = m_cameraIntrinsic[type].k2Param;
	m_cameraInfo[type].d.at(2) = m_cameraIntrinsic[type].p1Param;
	m_cameraInfo[type].d.at(3) = m_cameraIntrinsic[type].p2Param;
	m_cameraInfo[type].distortion_model.at(4) = m_cameraIntrinsic[type].k3Param;

	m_cameraInfo[type].k.at(0) = m_cameraIntrinsic[type].fxParam;
	m_cameraInfo[type].k.at(2) = m_cameraIntrinsic[type].cxParam;
	m_cameraInfo[type].k.at(4) = m_cameraIntrinsic[type].fyParam;
	m_cameraInfo[type].k.at(5) = m_cameraIntrinsic[type].cyParam;
	m_cameraInfo[type].k.at(8) = 1;

	m_cameraInfo[type].p.at(0) = m_cameraInfo[type].k.at(0);
	m_cameraInfo[type].p.at(1) = 0;
	m_cameraInfo[type].p.at(2) = m_cameraInfo[type].k.at(2);
	m_cameraInfo[type].p.at(3) = 0;
	m_cameraInfo[type].p.at(4) = 0;
	m_cameraInfo[type].p.at(5) = m_cameraInfo[type].k.at(4);
	m_cameraInfo[type].p.at(6) = m_cameraInfo[type].k.at(5);
	m_cameraInfo[type].p.at(7) = 0;
	m_cameraInfo[type].p.at(8) = 0;
	m_cameraInfo[type].p.at(9) = 0;
	m_cameraInfo[type].p.at(10) = 1;
	m_cameraInfo[type].p.at(11) = 0;

	m_cameraInfo[type].r.at(0) = 1.0;
	m_cameraInfo[type].r.at(1) = 0.0;
	m_cameraInfo[type].r.at(2) = 0.0;
	m_cameraInfo[type].r.at(3) = 0.0;
	m_cameraInfo[type].r.at(4) = 1.0;
	m_cameraInfo[type].r.at(5) = 0.0;
	m_cameraInfo[type].r.at(6) = 0.0;
	m_cameraInfo[type].r.at(7) = 0.0;
	m_cameraInfo[type].r.at(8) = 1.0;

}

}