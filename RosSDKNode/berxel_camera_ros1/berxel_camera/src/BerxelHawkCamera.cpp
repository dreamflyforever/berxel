#include "BerxelHawkCamera.h"
#include <fstream>
#include <unistd.h>
#include <std_msgs/Int32.h>
#define PI 3.1415926535897931
#include <sys/stat.h>

bool is_file(std::string filename) {
	struct stat   buffer;
	return (stat(filename.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool is_dir(std::string filefodler) {
	struct stat   buffer;
	return (stat(filefodler.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

BerxelHawkCamera::BerxelHawkCamera(ros::NodeHandle &node) : m_node(node)
{

	m_publishErrorCode = m_node.advertise<std_msgs::Int32>("errorcode", 1000);

	pthread_mutex_init(&m_mutex_cb, NULL);
	//Get SDK context
	m_pContext = BerxelHawkContext::getBerxelContext();
	//Register device hotplug event
	if (m_pContext != NULL)
		m_pContext->setDeviceStateCallback(onDeviceStatusChange,this);

#ifdef ENABLE_DYNAMIC_RECONFIGURE	
	//dynamic reconfige params
	m_callback = boost::bind(&BerxelHawkCamera::dynamic_params_callback, this, _1); 
    m_server.setCallback(m_callback); 
#endif

	//image service : save image
	m_imageServer = m_node.advertiseService("get_image_data", &BerxelHawkCamera::_ImageCallback, this);
	//log service : get device log
	m_deviceLogServer = m_node.advertiseService("get_device_log", &BerxelHawkCamera::_LogCallback, this);

}

BerxelHawkCamera::~BerxelHawkCamera()
{
	if (m_bThreadSuccess)
		pthread_join(m_nThreadID, NULL);
	pthread_mutex_destroy(&m_mutex_cb);

	destroy();

	if (m_pPointClouds)
	{
		free(m_pPointClouds);
		m_pPointClouds = NULL;
	}
	
}

int32_t BerxelHawkCamera::checkConfigureParams()
{
	if (m_bSupportRGB == false)
	{
		if (m_nStreamFlag != 1 || ((BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_DEPTH_STREAM && (BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_LIGHT_IR_STREAM))
		{
			ROS_ERROR("Current Device Only Support Depth!!!");
			return -1;
		}
	}

	if (m_nStreamFlag != BERXEL_HAWK_SINGULAR_STREAM_FLAG_MODE && m_nStreamFlag != BERXEL_HAWK_MIX_STREAM_FLAG_MODE && m_nStreamFlag != BERXEL_HAWK_MIX_HD_STREAM_FLAG_MODE && m_nStreamFlag != BERXEL_HAWK_MIX_QVGA_STREAM_FLAG_MODE)
	{
		ROS_ERROR("Set stream flag error, please check flag : %d", m_nStreamFlag);
	}

	if (m_nStreamFlag == 1)
	{
		if ((BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_IR_STREAM && (BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_COLOR_STREAM && 
			(BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_DEPTH_STREAM && (BerxelHawkStreamType)m_nStreadmType != BERXEL_HAWK_LIGHT_IR_STREAM)
		{
			ROS_ERROR("Set stream_type failed , Singular mode not support stream : %d", m_nStreadmType);
			return -1;
		}

		if ((BerxelHawkStreamType)m_nStreadmType == BERXEL_HAWK_IR_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_IR_STREAM, m_nIrWidth, m_nIrHeight) != 0)
			{
				return -1;
			}
		}

		if ((BerxelHawkStreamType)m_nStreadmType == BERXEL_HAWK_LIGHT_IR_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_LIGHT_IR_STREAM, m_nIrWidth, m_nIrHeight) != 0)
			{
				return -1;
			}
		}

		if ((BerxelHawkStreamType)m_nStreadmType == BERXEL_HAWK_COLOR_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_COLOR_STREAM, m_nColorWidth, m_nColorHeight) != 0)
			{
				return -1;
			}
		}

		if ((BerxelHawkStreamType)m_nStreadmType == BERXEL_HAWK_DEPTH_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_DEPTH_STREAM, m_nDepthWidth, m_nDepthHeight) != 0)
			{
				return -1;
			}
		}
	}
	else if (m_nStreamFlag == 2 || m_nStreamFlag == 3 || m_nStreamFlag == 4)
	{
		if ( m_nStreadmType & BERXEL_HAWK_IR_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_IR_STREAM, m_nIrWidth, m_nIrHeight) != 0)
			{
				return -1;
			}
		}

		if ( m_nStreadmType & BERXEL_HAWK_COLOR_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_COLOR_STREAM, m_nColorWidth, m_nColorHeight) != 0)
			{
				return -1;
			}
		}

		if ( m_nStreadmType & BERXEL_HAWK_DEPTH_STREAM)
		{
			if (checkResolution(BERXEL_HAWK_DEPTH_STREAM, m_nDepthWidth, m_nDepthHeight) != 0)
			{
				return -1;
			}
		}
	}
	else
	{
		ROS_ERROR("Set stream_flag error, not support : %d", m_nStreamFlag);
		return -1;
	}

	return 0;
}

int32_t BerxelHawkCamera::checkResolution(BerxelHawkStreamType type, int32_t width, int32_t height)
{
	const BerxelHawkStreamFrameMode* pDepthModeList = NULL;
	uint32_t nDepthModeListLen = 0;
	m_pHawkDevice->getSupportFrameModes(type, &pDepthModeList, &nDepthModeListLen);
	if (pDepthModeList[0].resolutionX > pDepthModeList[0].resolutionY)
	{
		m_bDeviceIsHorizontal = true;
	}
	else
	{
		m_bDeviceIsHorizontal = false;
	}

	if (m_bDeviceIsHorizontal)
	{
		if (height >= width)
		{
			ROS_ERROR("Current Device is horizontal, please set correct resolution!!!");
			return -1;
		}
	}
	else
	{
		if (height <= width)
		{
			ROS_ERROR("Current Device is vertical, please set correct resolution!!!");
			return -1;
		}
	}

	bool bFound = false;
	for (int i = 0; i < nDepthModeListLen; i++)
	{
		if (width == pDepthModeList[i].resolutionX && height == pDepthModeList[i].resolutionY)
		{
			bFound = true;
		}
	}

	if (!bFound)
	{
		ROS_ERROR("The resolution not support!");
		return -1;
	}

	BerxelHawkStreamFrameMode frameMode; 
	m_pHawkDevice->getCurrentFrameMode(type, &frameMode);

	frameMode.resolutionX = width;
	frameMode.resolutionY = height;
	//if ((type == BERXEL_HAWK_DEPTH_STREAM) || (m_nStreamFlag == 2) || (m_nStreamFlag == 4))
	{
		frameMode.framerate = m_nDepthFps;
	}

	m_pHawkDevice->setFrameMode(type, &frameMode);
	return 0;
}

void BerxelHawkCamera::initParams()
{
	m_node.param("stream_flag", m_nStreamFlag, 0x01);
	m_node.param("stream_type", m_nStreadmType, 0x01);
	m_node.param("color_width", m_nColorWidth, 400);
	m_node.param("color_height", m_nColorHeight, 640);
	m_node.param("depth_width", m_nDepthWidth, 400);
	m_node.param("depth_height", m_nDepthHeight, 640);
	m_node.param("ir_width", m_nIrWidth, 400);
	m_node.param("ir_height", m_nIrHeight, 640);
	m_node.param("depth_fps", m_nDepthFps, 30);
	
	if (m_nStreadmType == 0x05)
		m_nStreadmType = 0x20;

	m_node.param("enable_align", m_bRegistration, false);
	m_node.param("enable_pointcloud", m_bPubPointCloud, false);
	m_node.param("enable_color_pointcloud", m_bColorCloudPoint, false);
	m_node.param("enable_denoise", m_bEnableDenoise, false);
	m_node.param("enable_distance_check", m_bOpenSafetyMode, false);
	m_node.param("enable_ordered_pointcloud", m_bOrderedCloudPoint, true);
	m_node.param("enable_device_timestamp", m_bDeviceTimeStamp, true);
	m_node.param("enable_temperature_compensation", m_bEnableTemperatureCompensation, false);
	m_node.param("enable_invalid_point_data_zero", m_bEnableInvalidPointDataZero, false);
	m_node.param("enable_set_depth_Confidence", m_bEnableSetDepthConfidence, false);
	m_node.param("depth_confidence", m_nDepthConfidence, 3);
	m_node.param("depth_current", m_nDepthElectricCurrent, 15);
	m_node.param("enable_depth_ae", m_bEnableDepthAE, true);
	m_node.param("depth_exposure_time", m_nDepthExposureTime, 33);
	m_node.param("depth_gain", m_nDepthGain, 1);
	m_node.param("enable_edge_optimization", m_bEnableEdgeOptimization, false);
	m_node.param("enable_hight_fps_mode", m_bEnableHightFpsMode, false);
	m_node.param("enable_adjust_ae_gain_range", m_bEnableAEGainRange, false);
	m_node.param("depth_ae_gain_range_min", m_nDepthAEGainRangeMin, 1);
	m_node.param("depth_ae_gain_range_max", m_nDepthAEGainRangeMax, 4);
	m_node.param("image_cache_count", m_nImageCacheCount, 10);
	m_node.param("enable_image_cache", m_bEnableImageCache, false);

	m_node.param<std::string>("serial_no", m_strSerialNumber, "");
	m_node.param("usb_bus", m_nDeviceBus, 0);
	m_node.param<std::string>("usb_port", m_str_device_port, "");
	m_node.param<std::string>("camera", m_strDeviceName, "berxel_camera");

	m_node.param("camera_link_x", m_camera_link_x, 0.0f);
    m_node.param("camera_link_y", m_camera_link_y, 0.0f);
    m_node.param("camera_link_z", m_camera_link_z, 0.0f);
    m_node.param("camera_link_roll", m_camera_link_roll, 0.0f);
	m_node.param("camera_link_pitch", m_camera_link_pitch, 0.0f);
    m_node.param("camera_link_yaw", m_camera_link_yaw, 0.0f);
    m_node.param("camera_rgb_frame_x", m_camera_rgb_frame_x, 0.0f);
    m_node.param("camera_rgb_frame_y", m_camera_rgb_frame_y, 0.0f);
    m_node.param("camera_rgb_frame_z", m_camera_rgb_frame_z, 0.0f);
    m_node.param("camera_rgb_frame_roll", m_camera_rgb_frame_roll, 0.0f);
    m_node.param("camera_rgb_frame_pitch", m_camera_rgb_frame_pitch, 0.0f);
    m_node.param("camera_rgb_frame_yaw", m_camera_rgb_frame_yaw, 0.0f);
    m_node.param("camera_depth_frame_x", m_camera_depth_frame_x, 0.0f);
    m_node.param("camera_depth_frame_y", m_camera_depth_frame_y, 0.0f);
    m_node.param("camera_depth_frame_z", m_camera_depth_frame_z, 0.0f);
    m_node.param("camera_depth_frame_roll", m_camera_depth_frame_roll, 0.0f);
    m_node.param("camera_depth_frame_pitch", m_camera_depth_frame_pitch, 0.0f);
    m_node.param("camera_depth_frame_yaw", m_camera_depth_frame_yaw, 0.0f);
    m_node.param("camera_rgb_optical_frame_x", m_camera_rgb_optical_frame_x, 0.0f);
    m_node.param("camera_rgb_optical_frame_y", m_camera_rgb_optical_frame_y, 0.0f);
    m_node.param("camera_rgb_optical_frame_z", m_camera_rgb_optical_frame_z, 0.0f);
    m_node.param("camera_rgb_optical_frame_roll", m_camera_rgb_optical_frame_roll, 0.0f);
    m_node.param("camera_rgb_optical_frame_pitch", m_camera_rgb_optical_frame_pitch, 0.0f);
    m_node.param("camera_rgb_optical_frame_yaw", m_camera_rgb_optical_frame_yaw, 0.0f);
    m_node.param("camera_depth_optical_frame_x", m_camera_depth_optical_frame_x, 0.0f);
    m_node.param("camera_depth_optical_frame_y", m_camera_depth_optical_frame_y, 0.0f);
    m_node.param("camera_depth_optical_frame_z", m_camera_depth_optical_frame_z, 0.0f);
    m_node.param("camera_depth_optical_frame_roll", m_camera_depth_optical_frame_roll, 0.0f);
    m_node.param("camera_depth_optical_frame_pitch", m_camera_depth_optical_frame_pitch, 0.0f);
    m_node.param("camera_depth_optical_frame_yaw", m_camera_depth_optical_frame_yaw, 0.0f);

    tf::Quaternion camera_q;
    camera_q.setRPY(m_camera_link_roll * PI, m_camera_link_pitch * PI, m_camera_link_yaw * PI);                 //设置旋转坐标
    m_camera_link.setRotation(camera_q);              
    m_camera_link.setOrigin(tf::Vector3(m_camera_link_x, m_camera_link_y, m_camera_link_z));
    
    tf::Quaternion rgb_q;
    rgb_q.setRPY(m_camera_rgb_frame_roll * PI, m_camera_rgb_frame_pitch * PI, m_camera_rgb_frame_yaw * PI);                 //设置旋转坐标
    m_camera_rgb_frame.setRotation(rgb_q);              
    m_camera_rgb_frame.setOrigin(tf::Vector3(m_camera_rgb_frame_x,m_camera_rgb_frame_y,m_camera_rgb_frame_z));

    tf::Quaternion rgb_optical_q;
    rgb_optical_q.setRPY(m_camera_rgb_optical_frame_roll * PI, m_camera_rgb_optical_frame_pitch * PI, m_camera_rgb_optical_frame_yaw * PI);                 //设置旋转坐标
    m_camera_rgb_optical_frame.setRotation(rgb_optical_q);              
    m_camera_rgb_optical_frame.setOrigin(tf::Vector3(m_camera_rgb_optical_frame_x,m_camera_rgb_optical_frame_y,m_camera_rgb_optical_frame_z));
    
    tf::Quaternion detpth_q;
    detpth_q.setRPY(m_camera_depth_frame_roll * PI, m_camera_depth_frame_pitch * PI,  m_camera_depth_frame_yaw * PI);                   //设置旋转坐标
    m_camera_depth_frame.setRotation(detpth_q);              
    m_camera_depth_frame.setOrigin(tf::Vector3(m_camera_depth_frame_x,m_camera_depth_frame_y,m_camera_depth_frame_z));

	tf::Quaternion ir_q;
    detpth_q.setRPY(m_camera_depth_frame_roll * PI, m_camera_depth_frame_pitch * PI,  m_camera_depth_frame_yaw * PI);                   //设置旋转坐标
    m_camera_ir_frame.setRotation(detpth_q);              
    m_camera_ir_frame.setOrigin(tf::Vector3(m_camera_depth_frame_x,m_camera_depth_frame_y,m_camera_depth_frame_z));
    
    tf::Quaternion detpth__optical_q;
    detpth__optical_q.setRPY(m_camera_depth_optical_frame_roll * PI, m_camera_depth_optical_frame_pitch * PI, m_camera_depth_optical_frame_yaw * PI);                   //设置旋转坐标
    m_camera_depth_optical_frame.setRotation(detpth__optical_q);              
    m_camera_depth_optical_frame.setOrigin(tf::Vector3(m_camera_depth_optical_frame_x,m_camera_depth_optical_frame_y,m_camera_depth_optical_frame_z));

	tf::Quaternion ir__optical_q;
    ir__optical_q.setRPY(m_camera_depth_optical_frame_roll * PI, m_camera_depth_optical_frame_pitch * PI, m_camera_depth_optical_frame_yaw * PI);                   //设置旋转坐标
    m_camera_ir_optical_frame.setRotation(ir__optical_q);              
    m_camera_ir_optical_frame.setOrigin(tf::Vector3(m_camera_depth_optical_frame_x,m_camera_depth_optical_frame_y,m_camera_depth_optical_frame_z));

	m_pPointClouds = (berxel::BerxelHawkPoint3D*)malloc(m_nDepthWidth * m_nDepthHeight * sizeof(berxel::BerxelHawkPoint3D));

	advertiseROSTopics();
}

int32_t BerxelHawkCamera::initBerxelCamera()
{
	if (m_pContext == NULL){
		ROS_ERROR("Get context failed");
		berxelPublishErrorCode(BERXEL_ROS_CONTEXT_INIT_FAILED);
		return -1;
	}

	initParams();

	BerxelHawkDeviceInfo* pDeviceInfo = NULL;
	uint32_t deviceCount = 0;
	//Get device lists
	m_pContext->getDeviceList(&pDeviceInfo, &deviceCount);
    if((deviceCount <= 0) || (NULL == pDeviceInfo)) {
		ROS_ERROR("Get No Connected BerxelDevice");
		berxelPublishErrorCode(BERXEL_ROS_NOT_FIND_DEVICE);
		return -1;
    }

	memset(&m_CurrentDeviceInfo, 0x00, sizeof(m_CurrentDeviceInfo));
	//find device info by launch
	if (m_str_device_port.empty()) 
	{
		if (m_strSerialNumber.empty())
		{
			m_CurrentDeviceInfo = pDeviceInfo[0];
		}
		else
		{
			for (int i = 0; i < deviceCount; i++) 
			{
				if (strcmp(pDeviceInfo[i].serialNumber, m_strSerialNumber.c_str()) == 0) 
				{
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
		ROS_ERROR("Get Current Device Info Error !!!");
		berxelPublishErrorCode(BERXEL_ROS_IVALID_PARAMS);
		return -1;
	}

	ROS_INFO("Device SN : %s, address : %s", m_CurrentDeviceInfo.serialNumber, m_CurrentDeviceInfo.deviceAddress);
	//open device
	m_pHawkDevice = m_pContext->openDevice(m_CurrentDeviceInfo);
    if(NULL == m_pHawkDevice) {       
		ROS_ERROR("Open BerxelDevice Failed");
		berxelPublishErrorCode(BERXEL_ROS_OPENDEVICE_FAILED);
        return -1;
    }

	if ((m_CurrentDeviceInfo.productId == 0x0004 && m_CurrentDeviceInfo.vendorId == 0x0603) || 
		(m_CurrentDeviceInfo.productId == 0x0004 && m_CurrentDeviceInfo.vendorId == 0x0c45) ||
		(m_CurrentDeviceInfo.productId == 0x000B && m_CurrentDeviceInfo.vendorId == 0x0603)) 
	{
		m_bSupportRGB = false;
	}

	//sync host time to device
	m_pHawkDevice->setSystemClock();

	//set temperature compensation status
	m_pHawkDevice->setTemperatureCompensationEnable(m_bEnableTemperatureCompensation);

	//set depth denoise status 
	m_pHawkDevice->setDenoiseStatus(m_bEnableDenoise);

	//set distance-check status
	m_pHawkDevice->setSafetyMode(m_bOpenSafetyMode);

	//set align status
	m_pHawkDevice->setRegistrationEnable(m_bRegistration);
	
	//set depth confidence
	if (m_bEnableSetDepthConfidence) {
		int ret = m_pHawkDevice->setDepthConfidence(m_nDepthConfidence);
		ROS_INFO("Set Depth Confidence : %d, ret : %d", m_nDepthConfidence, ret);
	}

	//set depth current
	m_pHawkDevice->setDepthElectricCurrent(m_nDepthElectricCurrent * 100);

	//set auto exposure status
	uint32_t depthAeStatus = 0;
	m_pHawkDevice->getDepthAEStatus(&depthAeStatus);
	bool enable_ae = depthAeStatus ? true : false;
	if (enable_ae != m_bEnableDepthAE)
	{
		ROS_INFO("Set AE Status : %d", m_bEnableDepthAE);
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

	//Get camera info
	memset((uint8_t*)&m_DeviceIntrinsicParams, 0x00, sizeof(berxel::BerxelHawkDeviceIntrinsicParams));
	m_pHawkDevice->getDeviceIntriscParams(&m_DeviceIntrinsicParams);
	memcpy((uint8_t*)&m_rgbIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.colorIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
    memcpy((uint8_t*)&m_irIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.irIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
    memcpy((uint8_t*)&m_depthIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.liteIrIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
    memcpy((uint8_t*)&m_rotaParams, (uint8_t*)&m_DeviceIntrinsicParams.rotateIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));

	//Init camera info
	setupCameraInfo();

	//set stream flag 0x01 : Singular  0x02 : Mix VGA  0x03 : Mix HD  0x04 : Mix QVGA
	m_pHawkDevice->setStreamFlagMode((BerxelHawkStreamFlagMode)m_nStreamFlag);

	//check params
	if (checkConfigureParams() != 0) {
		ROS_ERROR("The configure params error, please check!!!");
		berxelPublishErrorCode(BERXEL_ROS_IVALID_PARAMS);
		return -1;
	}

	//Thread : rgb point cloud 
	if (m_bColorCloudPoint) {
		int ret = pthread_create(&m_nThreadID, NULL, berxelColorPointCloudThread, this);
		if (ret != 0) {
			ROS_ERROR("thread create failed.ret = %d", ret);
			m_bThreadSuccess = false;
			berxelPublishErrorCode(BERXEL_ROS_OTHER_ERROR);
			return -1;
		}

		m_bThreadSuccess = true;
	}

	//start stream
	int ret = m_pHawkDevice->startStreams(m_nStreadmType, onNewFrameCallback, this);
	if(ret != 0) {
		ROS_ERROR("Open Berxel Stream Failed");
		berxelPublishErrorCode(BERXEL_ROS_START_STREAM_FRAILED);
		return ret;
	}

	return 0;
}

void BerxelHawkCamera::setupCameraInfo()
{
	m_str_base_link = m_strDeviceName + "/Cam_base_link";
	m_str_camera_link = m_strDeviceName + "/Cam_link";
	m_str_rgb_frame = m_strDeviceName + "/rgb_frame";
	m_str_depth_frame = m_strDeviceName + "/depth_frame";
	m_str_ir_frame = m_strDeviceName + "/ir_frame";
	m_str_rgb_optical_frame = m_strDeviceName + "/rgb_optical_frame";
	m_str_depth_optical_frame = m_strDeviceName + "/depth_optical_frame";
	m_str_ir_optical_frame = m_strDeviceName + "/ir_optical_frame";

	m_cameraInfo[BERXEL_HAWK_COLOR_STREAM] = boost::make_shared<sensor_msgs::CameraInfo>();
	m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM] = boost::make_shared<sensor_msgs::CameraInfo>();
	m_cameraInfo[BERXEL_HAWK_IR_STREAM] = boost::make_shared<sensor_msgs::CameraInfo>();

	initCameraIntrinsic();
	updateCameraInfo(BERXEL_HAWK_COLOR_STREAM);
	updateCameraInfo(BERXEL_HAWK_DEPTH_STREAM);
	updateCameraInfo(BERXEL_HAWK_IR_STREAM);

}

void BerxelHawkCamera::initCameraIntrinsic()
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

	if (m_bRegistration && m_bSupportRGB)
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

	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].fxParam = m_depthIntrinsicParams.fxParam / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].fyParam = m_depthIntrinsicParams.fyParam / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].cxParam = m_depthIntrinsicParams.cxParam / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].cyParam = m_depthIntrinsicParams.cyParam / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k1Param = m_depthIntrinsicParams.k1Param / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k2Param = m_depthIntrinsicParams.k2Param / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].p1Param = m_depthIntrinsicParams.p1Param / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].p2Param = m_depthIntrinsicParams.p2Param / 2;
	m_cameraIntrinsic[BERXEL_HAWK_IR_STREAM].k3Param = m_depthIntrinsicParams.k3Param / 2;	
}

void BerxelHawkCamera::updateCameraInfo(berxel::BerxelHawkStreamType type)
{
	m_cameraInfo[type]->distortion_model = "plumb_bob";
	m_cameraInfo[type]->binning_x = m_cameraInfo[type]->binning_y = 1;

	m_cameraInfo[type]->D.resize(5);
	m_cameraInfo[type]->D.at(0) = m_cameraIntrinsic[type].k1Param;
	m_cameraInfo[type]->D.at(1) = m_cameraIntrinsic[type].k2Param;
	m_cameraInfo[type]->D.at(2) = m_cameraIntrinsic[type].p1Param;
	m_cameraInfo[type]->D.at(3) = m_cameraIntrinsic[type].p2Param;
	m_cameraInfo[type]->D.at(4) = m_cameraIntrinsic[type].k3Param;

	m_cameraInfo[type]->K.at(0) = m_cameraIntrinsic[type].fxParam;
	m_cameraInfo[type]->K.at(2) = m_cameraIntrinsic[type].cxParam;
	m_cameraInfo[type]->K.at(4) = m_cameraIntrinsic[type].fyParam;
	m_cameraInfo[type]->K.at(5) = m_cameraIntrinsic[type].cyParam;
	m_cameraInfo[type]->K.at(8) = 1;

	m_cameraInfo[type]->P.at(0) = m_cameraInfo[type]->K.at(0);
	m_cameraInfo[type]->P.at(1) = 0;
	m_cameraInfo[type]->P.at(2) = m_cameraInfo[type]->K.at(2);
	m_cameraInfo[type]->P.at(3) = 0;
	m_cameraInfo[type]->P.at(4) = 0;
	m_cameraInfo[type]->P.at(5) = m_cameraInfo[type]->K.at(4);
	m_cameraInfo[type]->P.at(6) = m_cameraInfo[type]->K.at(5);
	m_cameraInfo[type]->P.at(7) = 0;
	m_cameraInfo[type]->P.at(8) = 0;
	m_cameraInfo[type]->P.at(9) = 0;
	m_cameraInfo[type]->P.at(10) = 1;
	m_cameraInfo[type]->P.at(11) = 0;

	m_cameraInfo[type]->R.at(0) = 1.0;
	m_cameraInfo[type]->R.at(1) = 0.0;
	m_cameraInfo[type]->R.at(2) = 0.0;
	m_cameraInfo[type]->R.at(3) = 0.0;
	m_cameraInfo[type]->R.at(4) = 1.0;
	m_cameraInfo[type]->R.at(5) = 0.0;
	m_cameraInfo[type]->R.at(6) = 0.0;
	m_cameraInfo[type]->R.at(7) = 0.0;
	m_cameraInfo[type]->R.at(8) = 1.0;

}

void BerxelHawkCamera::berxelPublishTF()
{
	ros::Time time_now = ros::Time::now();
	m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_link,time_now,m_str_base_link,m_str_camera_link));
	if (m_bSupportRGB)
		m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_rgb_frame,time_now,m_str_camera_link,m_str_rgb_frame));
	m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_depth_frame,time_now,m_str_camera_link,m_str_depth_frame));
	m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_ir_frame,time_now,m_str_camera_link,m_str_ir_frame));
	if (m_bSupportRGB)
		m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_rgb_optical_frame,time_now,m_str_rgb_frame,m_str_rgb_optical_frame));
	m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_depth_optical_frame,time_now,m_str_depth_frame,m_str_depth_optical_frame));
	m_pubBroadcaster.sendTransform(tf::StampedTransform(m_camera_ir_optical_frame,time_now,m_str_ir_frame,m_str_ir_optical_frame));
}

void BerxelHawkCamera::destroy()
{
	if(m_pContext)
	{
		m_pContext->closeDevice(m_pHawkDevice);
        berxel::BerxelHawkContext::destroyBerxelContext(m_pContext);
		m_pContext = NULL;
		m_pHawkDevice = NULL;
	}

	if(m_pHawkDevice)
	{
		while(!m_queDepthFrame.empty())
		{
			berxel::BerxelHawkFrame* pFrame = m_queDepthFrame.front();
			m_pHawkDevice->releaseFrame(pFrame);
			m_queDepthFrame.pop();
		}
			
		while(!m_queColorFrame.empty())
		{
			berxel::BerxelHawkFrame* pFrame = m_queColorFrame.front();
			m_pHawkDevice->releaseFrame(pFrame);
			m_queColorFrame.pop();
		}

		m_pHawkDevice->stopStreams(m_nStreadmType);
	}

	while(!m_queueDepth.empty())
	{
		m_queueDepth.pop();
	}

	while(!m_queueColor.empty())
	{
		m_queueColor.pop();
	}

	while(!m_queuePoint.empty())
	{
		m_queuePoint.pop();
	}
}

void BerxelHawkCamera::advertiseROSTopics()
{
	// ros::NodeHandle color_node(m_node, m_strDeviceName);
	image_transport::ImageTransport color_it(m_node);
	// ros::NodeHandle ir_node(m_node, m_strDeviceName);
	image_transport::ImageTransport ir_it(m_node);
	// ros::NodeHandle depth_node(m_node, m_strDeviceName);
	image_transport::ImageTransport depth_it(m_node);

	if (m_nStreadmType & 0x01) {
		m_pubColor = color_it.advertiseCamera("rgb/rgb_raw", 1);
	}

	if (m_nStreadmType & 0x02) {
		m_pubDepth = depth_it.advertiseCamera("depth/depth_raw", 1);
		if (m_bPubPointCloud) {
			m_pubCloudPoint = m_node.advertise<sensor_msgs::PointCloud2>("depth/berxel_cloudpoint", 1);
		}
	}

	if (m_nStreadmType & 0x04 || m_nStreadmType & 0x20) {
		m_pubIr = ir_it.advertiseCamera("ir/ir_raw", 1);
	}
}

void BerxelHawkCamera::onNewFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame, void* pUserData)
{
	if (NULL != pUserData)
	{
		BerxelHawkCamera* pBerxelCamera = static_cast<BerxelHawkCamera*>(pUserData);
		if(NULL != pBerxelCamera) 
		{
			pBerxelCamera->berxelFrameCallback(streamType, pFrame);
		}
	}
}

void BerxelHawkCamera::berxelFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame)
{
	pthread_mutex_lock(&m_mutex_cb);
	if (m_bColorCloudPoint && (m_nStreadmType & BERXEL_HAWK_COLOR_STREAM) && (m_nStreadmType & BERXEL_HAWK_DEPTH_STREAM))
	{
		if (streamType == berxel::BERXEL_HAWK_COLOR_STREAM)
		{
			if (m_queColorFrame.size() >= 3)
			{
				berxel::BerxelHawkFrame* pFrame = m_queColorFrame.front();
				m_pHawkDevice->releaseFrame(pFrame);
				m_queColorFrame.pop();
			}

			m_queColorFrame.push(pFrame);
		}
		else if (streamType == berxel::BERXEL_HAWK_DEPTH_STREAM)
		{
			if (m_queDepthFrame.size() >= 3)
			{
				berxel::BerxelHawkFrame* pFrame = m_queDepthFrame.front();
				m_pHawkDevice->releaseFrame(pFrame);
				m_queDepthFrame.pop();
			}

			m_queDepthFrame.push(pFrame);
		}
		else
		{
			ROS_ERROR("Color Point Cloud Need Color And Depth, Please Check!!!");
		}
	}
	else
	{
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
		case berxel::BERXEL_HAWK_LIGHT_IR_STREAM:
			{
				berxelIrFrameCallback(pFrame);
			}
			break;
		default:
			break;
		}

		m_pHawkDevice->releaseFrame(pFrame);
	}

	berxelPublishTF();

	pthread_mutex_unlock(&m_mutex_cb);
}

void BerxelHawkCamera::berxelPublishErrorCode(BERXEL_ROS_ERROR_CODE errorcode)
{
	std_msgs::Int32 msg;
	msg.data = errorcode;
	m_publishErrorCode.publish(msg);
}

ros::Time BerxelHawkCamera::timesamp2rostime(uint64_t timesamp){
	// std::string suanz = std::to_string(timesamp);
	// std::string sec_string = suanz.substr(0,10);
	// std::string nsec_string = suanz.substr(10,9);
	// while(nsec_string.length() < 9){
		// nsec_string += "0";
	// }
	// return ros::Time(std::stoi(sec_string),std::stoi(nsec_string));
	uint32_t tempSec = (uint32_t)(timesamp / 1000 / 1000);
	uint32_t tempNsec = (uint32_t)(timesamp % 1000000) * 1000;
	return ros::Time(tempSec,tempNsec);
}

void BerxelHawkCamera::berxelColorFrameCallback(berxel::BerxelHawkFrame* pFrame)
{
	if (m_pHawkDevice && pFrame != NULL)
	{
		sensor_msgs::ImagePtr image(new sensor_msgs::Image);
		ros::Time ros_now = ros::Time::now();
		if (!m_bDeviceTimeStamp) {
			image->header.stamp = ros_now;
		} else  {
			image->header.stamp = timesamp2rostime(pFrame->getTimeStamp());
		}

		image->width = pFrame->getWidth();
		image->height = pFrame->getHeight();
		std::size_t data_size = pFrame->getDataSize();
		image->data.resize(data_size);
    	memcpy(&image->data[0], pFrame->getData(), data_size);
		image->is_bigendian = 0;
		image->encoding = sensor_msgs::image_encodings::RGB8;
		image->step = sizeof(unsigned char) * 3 * image->width;
		image->header.frame_id = m_str_rgb_optical_frame;

		if (m_bEnableImageCache)
		{
			std::lock_guard<std::mutex> locker(m_ColorCacheLocker);
			if (m_queueColor.size() >= m_nImageCacheCount)
			{
				m_queueColor.pop();
			}
			m_queueColor.push(image);	
		}

		m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]->width  = image->width;
		m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]->height = image->height;
		m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]->header.frame_id = m_str_rgb_optical_frame;
		m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]->header.stamp = image->header.stamp;
		m_pubColor.publish(image, m_cameraInfo[BERXEL_HAWK_COLOR_STREAM]);
	}
}

void BerxelHawkCamera::convertDepthToPointCloud(BerxelHawkFrame* pFrame ,BerxelHawkPoint3D* pPointClouds)
{
    int32_t index = 0;
    uint16_t* pData = (uint16_t*)pFrame->getData();
	float tmpDepthValue = 0;
    for(int j = 0; j < m_nDepthHeight; ++j) {
        for(int i = 0; i < m_nDepthWidth; ++ i) {
            index = j * m_nDepthWidth + i;
			if (pFrame->getPixelType() == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_12I_4D) {
				tmpDepthValue = (pData[index] * 1.0) / 16000.0;
			} else {
				tmpDepthValue = (pData[index] * 1.0)/8000.0;
			}
			if(tmpDepthValue > 0) {
				pPointClouds[index].z = tmpDepthValue;
				pPointClouds[index].x = ((float)(i - m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cxParam) * pPointClouds[index].z) / m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fxParam;
				pPointClouds[index].y = ((float)(j - m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].cyParam) * pPointClouds[index].z) / m_cameraIntrinsic[BERXEL_HAWK_DEPTH_STREAM].fyParam;
   
			}
			else {
				pPointClouds[index].z = pPointClouds[index].x = pPointClouds[index].y = 0.0f;
			}
		}
	}
}

void BerxelHawkCamera::convertDepthToCv16UC(uint16_t* pDepth, uint16_t* pCv16UC1,  int width, int height, berxel::BerxelHawkPixelType pixelType)
{
	if(pixelType == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D) {
		for(uint32_t i = 0; i < width * height; ++i)
			pCv16UC1[i] = pDepth[i] >> 3;
	} else {
		for(uint32_t i = 0; i < width * height; ++i)
			pCv16UC1[i] = pDepth[i] >> 4;
	}
}

void BerxelHawkCamera::berxelDepthFrameCallback(berxel::BerxelHawkFrame* pDepthFrame, berxel::BerxelHawkFrame* pColorFrame)
{
	if (m_pHawkDevice && pDepthFrame != NULL)
	{
		sensor_msgs::ImagePtr image(new sensor_msgs::Image);
		ros::Time ros_now = ros::Time::now();
		if (!m_bDeviceTimeStamp) {
			image->header.stamp = ros_now;
		} else {
			image->header.stamp = timesamp2rostime(pDepthFrame->getTimeStamp());
		}

		image->width = m_nDepthWidth;
		image->height = m_nDepthHeight;
		std::size_t data_size = pDepthFrame->getDataSize();
		image->data.resize(data_size);
		convertDepthToCv16UC((uint16_t*)pDepthFrame->getData(), (uint16_t*)&image->data[0], pDepthFrame->getWidth(), pDepthFrame->getHeight(), pDepthFrame->getPixelType());
		image->is_bigendian = 0;
		image->encoding = sensor_msgs::image_encodings::MONO16;
		image->step = sizeof(unsigned char) * 2 * image->width;
		image->header.frame_id = m_str_depth_optical_frame;
		m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]->width  = image->width;
		m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]->height = image->height;
		m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]->header.frame_id = m_str_depth_optical_frame;
		m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]->header.stamp = image->header.stamp;

		if (m_bPubPointCloud)
		{
			uint32_t valid_count = 0;
			sensor_msgs::PointCloud2Modifier modifier(msg_pointcloud);
			modifier.resize(m_nDepthHeight * m_nDepthWidth);
			
			msg_pointcloud.header.stamp = image->header.stamp;
			msg_pointcloud.header.frame_id = m_str_depth_optical_frame;
			if (m_bOrderedCloudPoint)
			{
				msg_pointcloud.width = m_nDepthWidth;
				msg_pointcloud.height = m_nDepthHeight;
				msg_pointcloud.is_dense = false;
			}

			if (m_bColorCloudPoint && pColorFrame != NULL)
			{
				RGB888* pColorData = (RGB888*)pColorFrame->getData();
				modifier.setPointCloud2Fields(4,
					"x", 1, sensor_msgs::PointField::FLOAT32,
					"y", 1, sensor_msgs::PointField::FLOAT32,
					"z", 1, sensor_msgs::PointField::FLOAT32,
					"rgb", 1, sensor_msgs::PointField::FLOAT32);
				modifier.setPointCloud2FieldsByString(2, "xyz", "rgb");
				sensor_msgs::PointCloud2Iterator<float> iter_x(msg_pointcloud, "x");
				sensor_msgs::PointCloud2Iterator<float> iter_y(msg_pointcloud, "y");
				sensor_msgs::PointCloud2Iterator<float> iter_z(msg_pointcloud, "z");
				sensor_msgs::PointCloud2Iterator<uint8_t> iter_r(msg_pointcloud, "r");
				sensor_msgs::PointCloud2Iterator<uint8_t> iter_g(msg_pointcloud, "g");
				sensor_msgs::PointCloud2Iterator<uint8_t> iter_b(msg_pointcloud, "b");
				m_pHawkDevice->convertDepthToPointCloud(pDepthFrame, 1000.0, m_pPointClouds, true);
				for (int nIndex = 0; nIndex < m_nDepthHeight * m_nDepthWidth; nIndex++)
				{
					bool valid_pixel(m_pPointClouds[nIndex].z > 0);
					if (valid_pixel || m_bOrderedCloudPoint)
					{
						// *iter_x = m_pPointClouds[nIndex].x;
						// *iter_y = m_pPointClouds[nIndex].y;
						// *iter_z = m_pPointClouds[nIndex].z;
						// *iter_r = pColorData[nIndex].r;
						// *iter_g = pColorData[nIndex].g;
						// *iter_b = pColorData[nIndex].b;
						if (valid_pixel == 0) {
							if (m_bEnableInvalidPointDataZero) {
								*iter_x = *iter_y = *iter_z = 0x00;
							}
							else {
								*iter_x = *iter_y = *iter_z = 0xFFFFFFFF;
							}
							*iter_r = *iter_g = *iter_b = 0;
						}
						else {
							*iter_x = m_pPointClouds[nIndex].x;
							*iter_y = m_pPointClouds[nIndex].y;
							*iter_z = m_pPointClouds[nIndex].z;
							*iter_r = pColorData[nIndex].r;
							*iter_g = pColorData[nIndex].g;
							*iter_b = pColorData[nIndex].b;
						}

						valid_count++;

						++iter_x; ++iter_y; ++iter_z;
						++iter_r; ++iter_g; ++iter_b;
					}
			
				}
			}
			else
			{
				modifier.setPointCloud2Fields(3,
					"x", 1, sensor_msgs::PointField::FLOAT32,
					"y", 1, sensor_msgs::PointField::FLOAT32,
					"z", 1, sensor_msgs::PointField::FLOAT32);
				modifier.setPointCloud2FieldsByString(1, "xyz");
				sensor_msgs::PointCloud2Iterator<float> iter_x(msg_pointcloud, "x");
				sensor_msgs::PointCloud2Iterator<float> iter_y(msg_pointcloud, "y");
				sensor_msgs::PointCloud2Iterator<float> iter_z(msg_pointcloud, "z");
				m_pHawkDevice->convertDepthToPointCloud(pDepthFrame, 1000.0, m_pPointClouds, true);
				for (int nIndex = 0; nIndex < m_nDepthHeight * m_nDepthWidth; nIndex++)
				{
					bool valid_pixel(m_pPointClouds[nIndex].z > 0);
					if (valid_pixel || m_bOrderedCloudPoint)
					{
						if (valid_pixel == 0) {
							if (m_bEnableInvalidPointDataZero) {
								*iter_x = *iter_y = *iter_z = 0x00;
							}
							else {
								*iter_x = *iter_y = *iter_z = 0xFFFFFFFF;
							}
						}
						else {
							*iter_x = m_pPointClouds[nIndex].x;
							*iter_y = m_pPointClouds[nIndex].y;
							*iter_z = m_pPointClouds[nIndex].z;
						}
						valid_count++;
						++iter_x; ++iter_y; ++iter_z;
					}
					
					
				}
			}

			if (!m_bOrderedCloudPoint)
			{
				msg_pointcloud.width = valid_count;
				msg_pointcloud.height = 1;
				msg_pointcloud.is_dense = true;
				modifier.resize(valid_count);
			}

			if (m_bEnableImageCache)
			{
				std::lock_guard<std::mutex> locker(m_PointCacheLocker);
				if (m_queuePoint.size() >= m_nImageCacheCount)
				{
					m_queuePoint.pop();
				}
				sensor_msgs::PointCloud2Ptr point_ptr(new sensor_msgs::PointCloud2(msg_pointcloud));
				m_queuePoint.push(point_ptr);	
			}

			m_pubCloudPoint.publish(msg_pointcloud);

			// pcl::PointCloud<pcl::PointXYZ> cloud;
			// if (m_bOrderedCloudPoint)
			// {
			// 	cloud.width = m_nDepthWidth;
			// 	cloud.height = m_nDepthHeight;
			// }
			// else
			// {
			// 	cloud.width = m_nDepthWidth * m_nDepthHeight;
			// 	cloud.height = 1;
			// }
			// cloud.is_dense = false;
			// cloud.points.clear();
			// cloud.points.resize(m_nDepthWidth * m_nDepthHeight);
       		// m_pHawkDevice->convertDepthToPointCloud(pDepthFrame, 1000.0, m_pPointClouds);
			// for(int i =0;i < m_nDepthWidth * m_nDepthHeight  ;i++)
			// {
			// 	if (m_bOrderedCloudPoint)
			// 	{
			// 		if (m_pPointClouds[i].z == 0.0)
			// 		{
			// 			cloud.points[i].x = cloud.points[i].y = cloud.points[i].z = 0xFFFFFFFF;
			// 		}
			// 		else
			// 		{
			// 			cloud.points[i].x = m_pPointClouds[i].x;
			// 			cloud.points[i].y = m_pPointClouds[i].y;
			// 			cloud.points[i].z = m_pPointClouds[i].z;
			// 		}
			// 	}
			// 	else
			// 	{
			// 		cloud.points[i].x = m_pPointClouds[i].x;
			// 		cloud.points[i].y = m_pPointClouds[i].y;
			// 		cloud.points[i].z = m_pPointClouds[i].z;
			// 	}
			// }
			// sensor_msgs::PointCloud2 output;
			// pcl::toROSMsg(cloud, output);
			// output.header.frame_id = m_str_depth_optical_frame;
			// output.header.stamp = image->header.stamp;
			// m_pubCloudPoint.publish(output);
		}

		if (m_bEnableImageCache)
		{
			std::lock_guard<std::mutex> locker(m_DepthCacheLocker);
			if (m_queueDepth.size() >= m_nImageCacheCount)
			{
				m_queueDepth.pop();
			}
			m_queueDepth.push(image);	
		}

		m_pubDepth.publish(image, m_cameraInfo[BERXEL_HAWK_DEPTH_STREAM]);
	}
}

void BerxelHawkCamera::berxelIrFrameCallback(berxel::BerxelHawkFrame* pFrame)
{
	if (m_pHawkDevice && pFrame != NULL)
	{
		sensor_msgs::ImagePtr image(new sensor_msgs::Image);
		ros::Time ros_now = ros::Time::now();
		if (!m_bDeviceTimeStamp)
		{
			image->header.stamp = ros_now;
		}
		else 
		{
			image->header.stamp = timesamp2rostime(pFrame->getTimeStamp());
		}

		image->width = pFrame->getWidth();
		image->height = pFrame->getHeight();
		std::size_t data_size = pFrame->getDataSize();
		image->data.resize(data_size);
    	memcpy(&image->data[0], pFrame->getData(), data_size);
		image->is_bigendian = 0;
		image->encoding = sensor_msgs::image_encodings::MONO16;
		image->step = sizeof(unsigned char) * 2 * image->width;
		image->header.frame_id = m_str_ir_optical_frame;
		m_cameraInfo[BERXEL_HAWK_IR_STREAM]->width  = image->width;
		m_cameraInfo[BERXEL_HAWK_IR_STREAM]->height = image->height;
		m_cameraInfo[BERXEL_HAWK_IR_STREAM]->header.frame_id = m_str_ir_optical_frame;
		m_cameraInfo[BERXEL_HAWK_IR_STREAM]->header.stamp = image->header.stamp;
		m_pubIr.publish(image, m_cameraInfo[BERXEL_HAWK_IR_STREAM]);
	}
}

void BerxelHawkCamera::readDeviceIntriscParams(berxel::BerxelHawkDeviceIntrinsicParams* pParams)
{
	if (m_pHawkDevice != NULL)
	{
		m_pHawkDevice->getDeviceIntriscParams(pParams);
	}
}

void BerxelHawkCamera::onDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState, void* pUserData)
{
	if(NULL != pUserData) 
	{
		BerxelHawkCamera* pBerxelCamera = static_cast<BerxelHawkCamera*>(pUserData);
		if(NULL != pBerxelCamera) 
		{
			pBerxelCamera->processDeviceStatusChange(deviceUri, deviceSerialNumber, deviceState);
		}
	}
}

int32_t BerxelHawkCamera::processDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState)
{
	switch(deviceState) 
	{
    case berxel::BERXEL_HAWK_DEVICE_DISCONNECT:
		{
			if (strcmp(m_CurrentDeviceInfo.serialNumber, deviceSerialNumber) == 0)
			{
				berxelPublishErrorCode(BERXEL_ROS_DEVICE_DISCONNECTED);
				ROS_INFO("Device Disconnect!");
				if (m_pHawkDevice != NULL)
				{
					pthread_mutex_lock(&m_mutex_cb);
					while(!m_queDepthFrame.empty())
					{
						berxel::BerxelHawkFrame* pFrame = m_queDepthFrame.front();
						m_pHawkDevice->releaseFrame(pFrame);
						m_queDepthFrame.pop();
					}
						
					while(!m_queColorFrame.empty())
					{
						berxel::BerxelHawkFrame* pFrame = m_queColorFrame.front();
						m_pHawkDevice->releaseFrame(pFrame);
						m_queColorFrame.pop();
					}
					pthread_mutex_unlock(&m_mutex_cb);
					m_pHawkDevice->stopStreams(m_nStreadmType);

					m_pContext->closeDevice(m_pHawkDevice);
					m_pHawkDevice = NULL;

					if (m_bThreadSuccess)
					{
						m_bThreadSuccess = false;
						if (m_bThreadSuccess)
							pthread_join(m_nThreadID, NULL);	
					}
				}
			}
		}
		break;
	case berxel::BERXEL_HAWK_DEVICE_CONNECT:
		{
			if (strcmp(m_CurrentDeviceInfo.serialNumber, deviceSerialNumber) == 0)
			{
				berxelPublishErrorCode(BERXEL_ROS_DEVICE_CONNECTED);
				ROS_INFO("Device Connected!");
				BerxelHawkDeviceInfo* pDeviceInfo = NULL;
				uint32_t deviceCount = 0;
				m_pContext->getDeviceList(&pDeviceInfo, &deviceCount);
				if((deviceCount <= 0) || (NULL == pDeviceInfo))
				{
					ROS_ERROR("Get No Connected BerxelDevice");
					return -1;
				}

				bool bFound = false;
				for (int i = 0; i < deviceCount; i++)
				{
					if (strcmp(pDeviceInfo[i].serialNumber, deviceSerialNumber) == 0)
					{
						bFound = true;
						m_CurrentDeviceInfo = pDeviceInfo[i];
						break;
					}
				}

				if (bFound == false || m_pHawkDevice != NULL)
				{
					ROS_ERROR("Device(%s) no exist", deviceUri);
					return -1;
				}

				ROS_INFO("m_CurrentDeviceInfo -- > SN : %s", m_CurrentDeviceInfo.serialNumber);
				m_pHawkDevice = m_pContext->openDevice(m_CurrentDeviceInfo);
				if(NULL == m_pHawkDevice)
				{       
					ROS_ERROR("Open BerxelDevice Failed");
					return -1;
				}

				memset((uint8_t*)&m_DeviceIntrinsicParams, 0x00, sizeof(berxel::BerxelHawkDeviceIntrinsicParams));
				m_pHawkDevice->getDeviceIntriscParams(&m_DeviceIntrinsicParams);
				memcpy((uint8_t*)&m_rgbIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.colorIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
				memcpy((uint8_t*)&m_irIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.irIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
				memcpy((uint8_t*)&m_depthIntrinsicParams, (uint8_t*)&m_DeviceIntrinsicParams.liteIrIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));
				memcpy((uint8_t*)&m_rotaParams, (uint8_t*)&m_DeviceIntrinsicParams.rotateIntrinsicParams, sizeof(BerxelHawkCameraIntrinsic));

				m_pHawkDevice->setSystemClock();
				m_pHawkDevice->setTemperatureCompensationEnable(m_bEnableTemperatureCompensation);
				m_pHawkDevice->setDenoiseStatus(m_bEnableDenoise);
				m_pHawkDevice->setSafetyMode(m_bOpenSafetyMode);
				m_pHawkDevice->setRegistrationEnable(m_bRegistration);

				setupCameraInfo();

				m_pHawkDevice->setStreamFlagMode((BerxelHawkStreamFlagMode)m_nStreamFlag);
				
				//校验参数是否合法
				if (checkConfigureParams() != 0) {
					ROS_ERROR("The configure params error, please check!!!");
					berxelPublishErrorCode(BERXEL_ROS_IVALID_PARAMS);
					return -1;
				}

				//彩色点云处理线程
				if (m_bColorCloudPoint && m_bThreadSuccess == false) {
					int ret = pthread_create(&m_nThreadID, NULL, berxelColorPointCloudThread, this);
					if (ret != 0) {
						ROS_ERROR("thread create failed.ret = %d", ret);
						m_bThreadSuccess = false;
						berxelPublishErrorCode(BERXEL_ROS_OTHER_ERROR);
						return -1;
					}

					m_bThreadSuccess = true;
				}


				int ret = m_pHawkDevice->startStreams(m_nStreadmType, onNewFrameCallback, this);
				if(ret != 0)
				{
					ROS_ERROR("Open Berxel Stream Failed");
					berxelPublishErrorCode(BERXEL_ROS_START_STREAM_FRAILED);
					return -1;
				}
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

void* BerxelHawkCamera::berxelColorPointCloudThread(void *obj)
{
	if (obj != NULL)
	{
		BerxelHawkCamera* pBerxelHawkCamera = static_cast<BerxelHawkCamera*> (obj);
		if (pBerxelHawkCamera != NULL)
		{
			pBerxelHawkCamera->berxelColorPointCloudHandle();
		}
	}
}

void BerxelHawkCamera::berxelColorPointCloudHandle()
{
	while (ros::ok() && m_bThreadSuccess)
	{
		usleep(3000);

		pthread_mutex_lock(&m_mutex_cb);

		if (m_queDepthFrame.size() > 0 && m_queColorFrame.size() > 0)
		{
			// ROS_INFO("berxelColorPointCloudHandle");
			berxel::BerxelHawkFrame* pDepthFrame = m_queDepthFrame.front();
			berxel::BerxelHawkFrame* pColorFrame = m_queColorFrame.front();
			

			berxelDepthFrameCallback(pDepthFrame, pColorFrame);
			berxelColorFrameCallback(pColorFrame);

			m_queDepthFrame.pop();
			m_pHawkDevice->releaseFrame(pDepthFrame);

			m_queColorFrame.pop();
			m_pHawkDevice->releaseFrame(pColorFrame);
		}

		pthread_mutex_unlock(&m_mutex_cb);
	}
}

#ifdef ENABLE_DYNAMIC_RECONFIGURE
void BerxelHawkCamera::dynamic_params_callback(berxel_camera::berxel_cameraConfig &config)
{	
	if (m_pHawkDevice) 
	{
		if (m_nDepthConfidence != config.depth_confidence) {
			m_nDepthConfidence = config.depth_confidence;
			ROS_INFO("Device : %s -> Set Depth Confidence : %d", m_CurrentDeviceInfo.serialNumber, m_nDepthConfidence);
			m_pHawkDevice->setDepthConfidence(m_nDepthConfidence);
		}

		if (m_nDepthElectricCurrent != config.depth_current) {
			m_nDepthElectricCurrent = config.depth_current;
			ROS_INFO("Device : %s -> Set Depth Electric Current : %d", m_CurrentDeviceInfo.serialNumber, m_nDepthElectricCurrent * 100);
			m_pHawkDevice->setDepthElectricCurrent(m_nDepthElectricCurrent * 100);
		}

		if (m_bEnableTemperatureCompensation != config.enable_temperature_compensation) {
			m_bEnableTemperatureCompensation = config.enable_temperature_compensation;
			ROS_INFO("Device : %s -> Set Temperatur Compensation Status : %d", m_CurrentDeviceInfo.serialNumber, m_bEnableTemperatureCompensation);
			m_pHawkDevice->setTemperatureCompensationEnable(m_bEnableTemperatureCompensation);
		}

		if (m_bOpenSafetyMode != config.enable_distance_check) {
			m_bOpenSafetyMode = config.enable_distance_check;
			ROS_INFO("Device : %s -> Set Distance Check Status : %d", m_CurrentDeviceInfo.serialNumber, m_bOpenSafetyMode);
			m_pHawkDevice->setSafetyMode(m_bOpenSafetyMode);
		}

		if (m_bEnableDenoise != config.enable_denoise) {
			m_bEnableDenoise = config.enable_denoise;
			ROS_INFO("Device : %s -> Set Denoist Status : %d", m_CurrentDeviceInfo.serialNumber, m_bEnableDenoise);
			m_pHawkDevice->setDenoiseStatus(m_bEnableDenoise);
		}

		if (m_bRegistration != config.enable_align) {
			m_bRegistration = config.enable_align;
			ROS_INFO("Device : %s -> Set Align Status : %d", m_CurrentDeviceInfo.serialNumber, m_bRegistration);
			m_pHawkDevice->setRegistrationEnable(m_bRegistration);
		}

		if (m_bEnableDepthAE != config.enable_depth_ae) {
			m_bEnableDepthAE = config.enable_depth_ae;
			ROS_INFO("Device : %s -> Set AE Status : %d", m_CurrentDeviceInfo.serialNumber, m_bEnableDepthAE);
			m_pHawkDevice->setDepthAEStatus(m_bEnableDepthAE);

			if (m_bEnableDepthAE == false)
			{
				uint32_t nExposure = 0;
				m_pHawkDevice->getDepthExposure(&nExposure);
				m_nDepthExposureTime = nExposure;

				uint32_t nGain = 0;
				m_pHawkDevice->getDepthGain(&nGain);
				m_nDepthGain = nGain;
			}
		}

		if (m_bEnableDepthAE == false)
		{
			if (m_nDepthExposureTime != config.depth_exposure_time)
			{
				m_nDepthExposureTime = config.depth_exposure_time;
				ROS_INFO("Device : %s -> Set Exposure Time : %d", m_CurrentDeviceInfo.serialNumber, m_nDepthExposureTime);
				m_pHawkDevice->setDepthExposure(m_nDepthExposureTime);
			}

			if (m_nDepthGain != config.depth_gain)
			{
				m_nDepthGain = config.depth_gain;
				ROS_INFO("Device : %s -> Set Gain : %d", m_CurrentDeviceInfo.serialNumber, m_nDepthGain);
				m_pHawkDevice->setDepthGain(m_nDepthGain);
			}

		}

		if (m_bEnableEdgeOptimization != config.enable_edge_optimization)
		{
			m_bEnableEdgeOptimization = config.enable_edge_optimization;
			ROS_INFO("Device : %s -> Set Edge Optimization : %d", m_CurrentDeviceInfo.serialNumber, m_bEnableEdgeOptimization);
			m_pHawkDevice->setEdgeOptimizationStatus(m_bEnableEdgeOptimization);
		}
	}
}
#endif


int32_t BerxelHawkCamera::saveBmpImage(const char* fileName, const uint8_t* pData, uint32_t width, uint32_t height)
{
	BMPHEADER bmfh; // bitmap file header
	BMPINFO bmih; // bitmap info header (windows)
	const int OffBits = 54;
	int32_t imagePixSize = width * height;
	memset(&bmfh, 0, sizeof(BMPHEADER));
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfType      = 0x4d42;
	bmfh.bfOffBits   = OffBits;
	bmfh.bfSize      = imagePixSize * 3 + OffBits;
	memset(&bmih, 0, sizeof(BMPINFO));
	bmih.biSize      = 40;
	bmih.biPlanes    = 1;
	bmih.biSizeImage = imagePixSize * 3;
	bmih.biBitCount    = 24;
	bmih.biCompression = 0;
	bmih.biWidth       = width;
	bmih.biHeight      = height;

	// rgb -> bgr
	RGB888* pRgb = m_bmpColor;
	RGB888* pSrc = (RGB888*)pData;
	int tmpindex1(0), tmpindex2(0);

	for(int i = 0; i < height; ++i)
	{
		tmpindex1 = i * width;
		tmpindex2 = (height - i - 1) * width;
		for(int j = 0; j < width; ++j)
		{
			pRgb[tmpindex1 + j].r = pSrc[tmpindex2 + j].b;
			pRgb[tmpindex1 + j].g = pSrc[tmpindex2 + j].g;
			pRgb[tmpindex1 + j].b = pSrc[tmpindex2 + j].r;
		}
	}

	FILE* pSaveBmp = fopen(fileName, "wb");
	if(NULL == pSaveBmp)
	{
		ROS_ERROR("Open file [%s] error", fileName);
		return -1;
	}

	fwrite(&bmfh, 8, 1, pSaveBmp);
	fwrite(&bmfh.bfReserved2, sizeof(bmfh.bfReserved2), 1, pSaveBmp);
	fwrite(&bmfh.bfOffBits, sizeof(bmfh.bfOffBits), 1, pSaveBmp);
	fwrite(&bmih, sizeof(BMPINFO), 1, pSaveBmp );
	fwrite(m_bmpColor, imagePixSize*3, 1, pSaveBmp);
	fclose(pSaveBmp);
	return 0;
}

void BerxelHawkCamera::saveRawData(char* fileName, uint8_t* pData, uint32_t dataSize)
{
	FILE* pFile = fopen(fileName, "wb");
	if(pFile)
	{
		fwrite(pData, dataSize, 1, pFile);
		fclose(pFile);
		ROS_INFO("Save Raw Data Success!");
	}
	else
	{
		ROS_ERROR("Open file [%s] error!", fileName);
	}
}

void BerxelHawkCamera::savePointCloudPly(const char* fileName, berxel::BerxelHawkPoint3D* pData, uint32_t width, uint32_t height)
{
	std::ofstream fout(fileName, std::ios::binary);
	fout<<"ply"<<"\r\n";	
	fout<<"format ascii 1.0"<<"\r\n";
	// if(width == 800 || width == 1280)
	// {
	// 	fout<<"element vertex 1024000"<<"\r\n";
	// }
	// else if (width == 400 || width == 640)
	// {
	// 	fout<<"element vertex 256000"<<"\r\n";
	// }
	// else
	// {
	// 	fout<<"element vertex 64000"<<"\r\n";
	// }
	fout<<"element vertex "<< width * height<< "\r\n";
	fout<<"property float x"<<"\r\n";
	fout<<"property float y"<<"\r\n";
	fout<<"property float z"<<"\r\n";
	fout<<"end_header"<<"\r\n";
	int pcdCount = width * height;
	for(int i = 0; i < pcdCount; ++i) 
	{	
		fout<< pData[i].x << " " <<  pData[i].y << " " <<  pData[i].z << "\r\n";	
	}
	fout.close();
}

bool BerxelHawkCamera::_ImageCallback(berxel_camera::berxel_camera_image::Request &req, berxel_camera::berxel_camera_image::Response &res)
{
	if(!m_bEnableImageCache) {
		ROS_ERROR("Image cache disable!!!");
		return false;
	}
		
	{
		std::lock_guard<std::mutex> locker(m_ColorCacheLocker);
		if (m_nStreadmType & 0x01)
		{ 
			if (m_queueColor.size() > 0)
			{
				sensor_msgs::ImagePtr pImage = m_queueColor.front();
				float difftime = std::abs((pImage->header.stamp.toSec() - req.timestamp.toSec()));
				for (int i = 0; i < m_queueColor.size(); i++)
				{
					m_queueColor.push(m_queueColor.front());
					m_queueColor.pop();

					float temtime = std::abs((m_queueColor.front()->header.stamp.toSec() - req.timestamp.toSec()));
					if (difftime > temtime)
					{
						difftime = temtime;
						pImage = m_queueColor.front();
					}
				}
				ROS_INFO("color image timsstamp : %.9f, request time : %.9f, color difftime : %.9f", 
					pImage->header.stamp.toSec(), req.timestamp.toSec(), difftime);
				res.color_data = *pImage;
				res.color_state = true;
			}
			else
			{
				ROS_ERROR("No Color Data.");
				res.color_state = false;
			}
		}
	}

	{
		std::lock_guard<std::mutex> locker(m_PointCacheLocker);
		if (m_nStreadmType & 0x02 && m_bPubPointCloud)
		{
			if (m_queuePoint.size() > 0)
			{
				sensor_msgs::PointCloud2Ptr pPointCloud = m_queuePoint.front();//header.stamp
				float difftime = std::abs((pPointCloud->header.stamp.toSec() - req.timestamp.toSec()));
				for (int i = 0; i < m_queuePoint.size(); i++)
				{
					m_queuePoint.push(m_queuePoint.front());
					m_queuePoint.pop();

					float temtime = std::abs((m_queuePoint.front()->header.stamp.toSec() - req.timestamp.toSec()));
					if (difftime > temtime)
					{
						difftime = temtime;
						pPointCloud = m_queuePoint.front();
					}
				}
				ROS_INFO("point cloud timsstamp : %.9f, request time : %.9f, point cloud difftime : %.9f", 
					pPointCloud->header.stamp.toSec(), req.timestamp.toSec(), difftime);
				res.point_data = *pPointCloud;
				res.point_state = true;
			}
			else
			{
				ROS_ERROR("No Point Cloud Data.");
				res.point_state = false;
			}	

		}

	}

	{
		std::lock_guard<std::mutex> locker(m_DepthCacheLocker);
		if (m_nStreadmType & 0x02)
		{
			if (m_queueDepth.size() > 0)
			{
				sensor_msgs::ImagePtr pImage = m_queueDepth.front();
				float difftime = std::abs((pImage->header.stamp.toSec() - req.timestamp.toSec()));
				for (int i = 0; i < m_queueDepth.size(); i++)
				{
					m_queueDepth.push(m_queueDepth.front());
					m_queueDepth.pop();

					float temtime = std::abs((m_queueDepth.front()->header.stamp.toSec() - req.timestamp.toSec()));
					if (difftime > temtime)
					{
						difftime = temtime;
						pImage = m_queueDepth.front();
					}
				}
				ROS_INFO("depth image timsstamp : %.9f, request time : %.9f, depth difftime : %.9f", 
					pImage->header.stamp.toSec(), req.timestamp.toSec(), difftime);
				res.depth_data = *pImage;
				res.depth_state = true;
			}
			else
			{
				ROS_ERROR("No Color Data.");
				res.color_state = false;
			}			
		}
	}

    return true;
}

bool BerxelHawkCamera::_LogCallback(berxel_camera::berxel_device_log::Request &req, berxel_camera::berxel_device_log::Response &res)
{
	bool bStatus = false;
	if (m_pHawkDevice == NULL) {
		ROS_ERROR("Device is null.");
		return bStatus;
	}
	
	uint32_t logSize = 0;
	m_pHawkDevice->getDeviceLogSize(&logSize);
	ROS_INFO("Device log size : %u", logSize);

	if (logSize == 0) {
		res.status = false;
		ROS_ERROR("Get device log error, log size is zero.");
		return bStatus;
	}

	char* logData = (char*)malloc(logSize);
	int32_t ret = m_pHawkDevice->getDeviceLog(logData, logSize);
	if (ret != 0) {
		ROS_ERROR("Get device error : %d", ret);
		res.status = false;
		return bStatus;
	}
	
	char fileName[512] = {0};
	auto time = ros::Time::now().toSec();
	if (is_dir(req.path)) {
		std::string name_path = req.path;
		char str = name_path.at(name_path.size() - 1);
		
		if (str != '/')
		{
			name_path += '/';
		}

		sprintf(fileName, "%s%f.gz", name_path.c_str(), time);
	}
	else {
		ROS_ERROR("Path (%s) not exisits, please check it.", req.path.c_str());
		sprintf(fileName, "%f.gz", time);
	}

	ROS_INFO("Log file name : %s", fileName);
	FILE* fp = fopen(fileName, "wb");
	if (fp != NULL)
	{
		fwrite(logData, logSize, 1, fp);
		bStatus = true;
		res.status = true;
		fclose(fp);
	}

	return bStatus;
}
