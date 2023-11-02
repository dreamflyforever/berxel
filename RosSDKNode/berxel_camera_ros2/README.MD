### 使用方式说明
该ROS驱动包仅支持ROS1发布的版本，环境搭建请参考ROS官方教程。
此驱动包需要配合Berxel SDK一起使用

eg: 
1. mkdir -p ros2_ws/src
2. 拷贝Berxel SDK发布的软件包到ros2_ws/src目录下并且解包，删除RosSDKNode/berxel_camera_ros1包，避免编译报错
3. cd ros2_ws
4. colcon build
5. source install/setup.bash
6. ros2 launch berxel_camera_ros2 berxel_camera_iHawk100.py  (PS:根据实际需要修改berxel_camera_iHawk100.py中加载的实际yaml文件)    
7. 对于多设备使用，可以使用"berxel_camera_multi_launch.py"
8. 此驱动包依赖Berxel SDK中的库文件，若单独将此驱动包拷贝到工作目录下，需要设置下Berxel SDK包中库路径到环境变量LD_LIBRARY_PATH中或者将SDK库放入系统目录下等。

### 参数说明               
"stream_flag"                               设备开启模式 1 ： 开启单个数据流  
                                                         2 :  Mix VGA 模式(即 彩色+深度 640*400分辨率)
                                                         3 ： Mix HD 模式(即彩色+深度 1280*800分辨率)
                                                         4 ： Mix QVGA 模式(即彩色+深度 320*200分辨率)

"stream_type"                               开启的流类型 1 ： 彩色
                                                         2 ： 深度
                                                         3 ： 深度+彩色
                                                         4 ： 泛光源红外
                                                         5 ： 点光源红外                   
"color_width"                               彩色图像宽度                  
"color_height"                              彩色图像高度                  
"depth_width"                               深度图像宽度                  
"depth_height"                              深度图像高度                  
"ir_width"                                  红外图像宽度                  
"ir_height"                                 红外图像高度
                                            泛光源 ： 640 * 400
                                            点光源 ： 1280 * 800
"depth_fps"                                 深度帧率                  
"enable_align"                              深度对齐彩色                  
"enable_pointcloud"                         点云使能开关  true : 开启点云发布  false ： 关闭点云发布
"enable_color_pointcloud"                   点云贴图彩色使能开关    true : 开启点云贴图彩色功能  false : 关闭点云贴图彩色功能
"enable_denoise"                            降噪使能开关    true : 打开降噪开关  false : 关闭降噪开关
"enable_device_timestamp"                   时间戳类型    true : 使用图像自带的时间戳    false : 使用ros::Time
"enable_temperature_compensation"           温度补偿使能开关    true ：打开温度补偿功能  false ：关闭温度补偿功能
"enable_ordered_pointcloud"                 有序点云使能    true : 有序点云  false : 无序点云
"enable_set_depth_Confidence"				深度置信度使能开关		true : 使能设置  false : 禁止设置  (设置深度置信度之前需要打开此开关)
"depth_confidence"							深度置信度值		范围：[3-5]
"depth_current"                             激光器电流          范围[8-15] 单位 (*100 ma)
"enable_depth_ae"                           深度AE使能开关      true: 使能自动曝光   false : 关闭自动曝光
"depth_exposure_time"                       深度曝光时间        范围[1-43] 单位( 0.1ms) 此项设置需在AE关闭状态下才可以设置
"depth_gain"                                深度增益            范围[1-4]   此项设置需在AE关闭状态下才可以设置
"enable_edge_optimization"                  物体边缘优化    true : 打开   false : 关闭
"enable_hight_fps_mode"                     固件高帧率模式  true : 打开  false : 关闭
"enable_adjust_ae_gain_range"               深度AE使能状态下Gain调节功能 true : 使能调节  false : 禁止调节
"depth_ae_gain_range_min"                   深度AE使能状态下Gain值动态调整范围下限 ：范围[1-4]      AE使能状态下调节有效
"depth_ae_gain_range_max"                   深度AE使能状态下Gain值动态调整范围上限 ：范围[1-4]      AE使能状态下调节有效