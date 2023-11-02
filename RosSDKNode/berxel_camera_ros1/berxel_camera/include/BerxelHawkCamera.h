#ifndef __BERXEL_HAWK_CAMERA_H__
#define __BERXEL_HAWK_CAMERA_H__

#include <stdio.h>
#include <string.h>
#include "ros/ros.h"
#include <pthread.h>
#include <queue>

#include "BerxelHawkContext.h"
#include "BerxelHawkDevice.h"
#include "BerxelHawkFrame.h"
#include "BerxelHawkDefines.h"

#include <image_transport/image_transport.h>
#include <camera_info_manager/camera_info_manager.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud2_iterator.h>
#include <tf/transform_broadcaster.h>
#include <mutex>

#include "berxel_camera/berxel_camera_image.h"
#include "berxel_camera/berxel_device_log.h"

//#define ENABLE_DYNAMIC_RECONFIGURE

#ifdef ENABLE_DYNAMIC_RECONFIGURE
#include <dynamic_reconfigure/server.h>
#include "berxel_camera/berxel_cameraConfig.h"
#endif

// #include <pcl/point_cloud.h>
// #include <pcl_conversions/pcl_conversions.h>

using namespace berxel;
typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}RGB888;

typedef enum
{
	BERXEL_ROS_DEVICE_CONNECTED = 2,		//device connected
	BERXEL_ROS_DEVICE_DISCONNECTED = 1,		//device disconnected
	BERXEL_ROS_SUCCESS = 0,					//success
	BERXEL_ROS_CONTEXT_INIT_FAILED = -1,	//init context error
	BERXEL_ROS_NOT_FIND_DEVICE = -2,		//not find deivce
	BERXEL_ROS_OPENDEVICE_FAILED = -3,		//open device failed
	BERXEL_ROS_START_STREAM_FRAILED = -4,	//start stream failed
	BERXEL_ROS_IVALID_PARAMS = -5,			//ivalid params
	BERXEL_ROS_OTHER_ERROR = -99,			//other
}BERXEL_ROS_ERROR_CODE;

typedef struct _BMPHEADER
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
} BMPHEADER;

typedef struct _BMPINFO
{
	uint32_t biSize;
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	uint32_t biXPelsPerMeter;
	uint32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
} BMPINFO;

class BerxelHawkCamera
{
public:
	BerxelHawkCamera(ros::NodeHandle &node);
	~BerxelHawkCamera();
	
	int32_t initBerxelCamera();
private:
	void initParams();
	void advertiseROSTopics();
	int32_t checkConfigureParams();
	int32_t checkResolution(BerxelHawkStreamType type, int32_t width, int32_t height);
	void readDeviceIntriscParams(berxel::BerxelHawkDeviceIntrinsicParams* pParams);
	void destroy();
	int32_t processDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState);
	void berxelFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame);
	void berxelColorFrameCallback(berxel::BerxelHawkFrame* pFrame);
	void berxelDepthFrameCallback(berxel::BerxelHawkFrame* pDepthFrame, berxel::BerxelHawkFrame* pColorFrame = NULL);
	void berxelIrFrameCallback(berxel::BerxelHawkFrame* pFrame);
	void berxelPublishTF();
	static void onDeviceStatusChange(const char* deviceUri, const char* deviceSerialNumber, berxel::BerxelHawkDeviceStatus deviceState, void* pUserData);
	static void onNewFrameCallback(berxel::BerxelHawkStreamType streamType, berxel::BerxelHawkFrame* pFrame, void* pUserData);
	static void* berxelColorPointCloudThread(void *obj);
	void berxelColorPointCloudHandle();
	inline void convertDepthToPointCloud(BerxelHawkFrame* pFrame ,BerxelHawkPoint3D* pPointClouds);
	inline ros::Time timesamp2rostime(uint64_t timesamp);
	inline void convertDepthToCv16UC(uint16_t* pDepth, uint16_t* pCv16UC1,  int width, int height, berxel::BerxelHawkPixelType pixelType);
	
	void setupCameraInfo();
	void initCameraIntrinsic();
	void updateCameraInfo(berxel::BerxelHawkStreamType type);
	void berxelPublishErrorCode(BERXEL_ROS_ERROR_CODE errorcode);
#ifdef ENABLE_DYNAMIC_RECONFIGURE
	void dynamic_params_callback(berxel_camera::berxel_cameraConfig &config);
#endif
	int32_t saveBmpImage(const char* fileName, const uint8_t* pData, uint32_t width, uint32_t height);

	void saveRawData(char* fileName, uint8_t* pData, uint32_t dataSize);

	void savePointCloudPly(const char* fileName, berxel::BerxelHawkPoint3D* pData, uint32_t width, uint32_t height);

	bool _ImageCallback(berxel_camera::berxel_camera_image::Request &req, berxel_camera::berxel_camera_image::Response &res);
	bool _LogCallback(berxel_camera::berxel_device_log::Request &req, berxel_camera::berxel_device_log::Response &res);
private:
	berxel::BerxelHawkDeviceIntrinsicParams	m_DeviceIntrinsicParams;
	berxel::BerxelHawkCameraIntrinsic m_rgbIntrinsicParams;
	berxel::BerxelHawkCameraIntrinsic m_irIntrinsicParams;
	berxel::BerxelHawkCameraIntrinsic m_depthIntrinsicParams;
	berxel::BerxelHawkCameraIntrinsic m_rotaParams;		//

	berxel::BerxelHawkContext*   m_pContext = NULL;
	berxel::BerxelHawkDevice*    m_pHawkDevice = NULL;
	berxel::BerxelHawkDeviceInfo m_CurrentDeviceInfo;

	std::queue<berxel::BerxelHawkFrame*> m_queDepthFrame;
	std::queue<berxel::BerxelHawkFrame*> m_queColorFrame;
	pthread_t m_nThreadID;

	int  		m_nStreamFlag = 1;
	int			m_nStreadmType = 2;
	int 		m_nColorWidth = 640;
	int			m_nColorHeight = 400;
	int 		m_nDepthWidth = 640;
	int			m_nDepthHeight = 400;
	int 		m_nIrWidth = 640;
	int			m_nIrHeight = 400;
	int			m_nDepthFps = 30;
	int			m_nDeviceBus = 0;
	int			m_nDepthConfidence = 3;
	int			m_nDepthElectricCurrent = 15;
	int			m_nDepthExposureTime = 33;
	int			m_nDepthGain = 1;
	int			m_nDepthAEGainRangeMin = 1;
	int			m_nDepthAEGainRangeMax = 4;
	int			m_nImageCacheCount = 10;

	bool		m_bRegistration = false;
	bool		m_bThreadSuccess = false;
	bool		m_bPubPointCloud = false;
	bool		m_bOrderedCloudPoint = false;
	bool 		m_bThreadStart = false;
	bool 		m_bDeviceIsHorizontal = false;
	bool		m_bDeviceTimeStamp = false;
	bool		m_bPubOriginalDepth = false;
	bool		m_bColorCloudPoint = false;
	bool		m_bSupportRGB = true;
	bool		m_bEnableDenoise = true;
	bool		m_bOpenSafetyMode = false;
	bool		m_bEnableTemperatureCompensation = false;
	bool		m_bEnableInvalidPointDataZero = false;
	bool		m_bEnableSetDepthConfidence = false;
	bool		m_bEnableDepthAE = true;
	bool		m_bEnableEdgeOptimization = false;
	bool		m_bEnableHightFpsMode = false;
	bool		m_bEnableAEGainRange = false;
	bool		m_bEnableImageCache = false;

    float m_camera_link_x = 0.0f;
	float m_camera_link_y = 0.0f;
	float m_camera_link_z = 0.0f;
	float m_camera_link_roll = 0.0f;
	float m_camera_link_pitch = 0.0f;
	float m_camera_link_yaw = 0.0f;
    float m_camera_rgb_frame_x = 0.0f;
	float m_camera_rgb_frame_y = 0.0f;
	float m_camera_rgb_frame_z = 0.0f;
	float m_camera_rgb_frame_roll = 0.0f;
	float m_camera_rgb_frame_pitch = 0.0f;
	float m_camera_rgb_frame_yaw = 0.0f;
    float m_camera_depth_frame_x = 0.0f;
	float m_camera_depth_frame_y = 0.0f;
	float m_camera_depth_frame_z = 0.0f;
	float m_camera_depth_frame_roll = 0.0f;
	float m_camera_depth_frame_pitch = 0.0f;
	float m_camera_depth_frame_yaw = 0.0f;
    float m_camera_rgb_optical_frame_x = 0.0f;
	float m_camera_rgb_optical_frame_y = 0.0f;
	float m_camera_rgb_optical_frame_z = 0.0f;
	float m_camera_rgb_optical_frame_roll = 0.0f;
	float m_camera_rgb_optical_frame_pitch = 0.0f;
	float m_camera_rgb_optical_frame_yaw = 0.0f;
    float m_camera_depth_optical_frame_x = 0.0f;
	float m_camera_depth_optical_frame_y = 0.0f;
	float m_camera_depth_optical_frame_z = 0.0f;
	float m_camera_depth_optical_frame_roll = 0.0f;
	float m_camera_depth_optical_frame_pitch = 0.0f;
	float m_camera_depth_optical_frame_yaw = 0.0f;

	std::string m_strDeviceName;
	std::string m_strSerialNumber;

	RGB888 m_bmpColor[1920*1080];

	berxel::BerxelHawkPoint3D* m_pPointClouds = NULL;

	image_transport::CameraPublisher m_pubColor;
	image_transport::CameraPublisher m_pubDepth;
	image_transport::CameraPublisher m_pubIr;
	ros::Publisher m_pubCloudPoint;
	ros::Publisher m_publishErrorCode;
	ros::NodeHandle &m_node;

	pthread_mutex_t m_mutex_cb;
	tf::TransformBroadcaster m_pubBroadcaster;
	tf::Transform m_camera_rgb_optical_frame;             //相机坐标系Cam_rgb
	tf::Transform m_camera_rgb_frame;             //相机坐标系Cam_rgb
	tf::Transform m_camera_link;             //相机坐标系Cam_link
	tf::Transform m_camera_depth_frame;
	tf::Transform m_camera_depth_optical_frame;
	tf::Transform m_camera_ir_frame;
	tf::Transform m_camera_ir_optical_frame;

	std::string m_str_base_link;
	std::string m_str_camera_link;
	std::string m_str_rgb_frame;
	std::string m_str_depth_frame;
	std::string m_str_ir_frame;
	std::string m_str_rgb_optical_frame;
	std::string m_str_depth_optical_frame;
	std::string m_str_ir_optical_frame;
	std::string m_str_device_port;
	sensor_msgs::PointCloud2 msg_pointcloud;
	std::map<berxel::BerxelHawkStreamType, sensor_msgs::CameraInfoPtr> m_cameraInfo;
	std::map<berxel::BerxelHawkStreamType, berxel::BerxelHawkCameraIntrinsic> m_cameraIntrinsic;

#ifdef ENABLE_DYNAMIC_RECONFIGURE
	dynamic_reconfigure::Server<berxel_camera::berxel_cameraConfig> m_server;
    dynamic_reconfigure::Server<berxel_camera::berxel_cameraConfig>::CallbackType m_callback;
#endif
	std::mutex m_ColorCacheLocker;
	std::mutex m_DepthCacheLocker;
	std::mutex m_PointCacheLocker;

	ros::ServiceServer m_imageServer;
	ros::ServiceServer m_deviceLogServer;
	std::queue<sensor_msgs::ImagePtr> m_queueDepth;
	std::queue<sensor_msgs::ImagePtr> m_queueColor;
	std::queue<sensor_msgs::PointCloud2Ptr> m_queuePoint;
};


#endif