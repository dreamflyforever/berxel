### 使用方式说明
该ROS驱动包仅支持ROS1发布的版本，环境搭建请参考ROS官方教程。
此驱动包需要配合Berxel SDK一起使用

1. mkdir -p catkin_ws/src
2. 拷贝Berxel SDK发布的软件包"BerxelSDK-Linux-Version"到catkin_ws/src目录下并且解包，删除RosSDKNode/berxel_camera_ros2目录(ros2编译方式和ros1不一样，会导致编译失败)
3. cd catkin_ws
4. catkin_make install
5. source install/setup.bash
6. roslaunch berxel_camera berxel_camera.launch  (PS:根据实际需要修改berxel_camera.launch中加载的实际xml文件，仅tf坐标区别)    
7. 对于多设备使用，可以使用"berxel_camera_multi_device.launch", 
            1. 可以通过配置序列号的方式实现多个设备开启 $(arg serial_no1) $(arg serial_no2)
            2. 可以通过配置bus-port的方式实现多个设备开启(优先级最高，在序列号和bus-port都配置的情况下以bus-port配置为准)
            $(arg usb_bus1) $(arg usb_bus2) $(arg usb_port1) $(arg usb_port2)

如果单独拷贝的berxel_camera到工作目录下，则需要设置SDK库和头文件的环境变量
1. mkdir -p catkin_ws/src
2. 拷贝berxel_camera 到catkin_ws/src
3. 进入SDK包目录： cd "BerxelSDK-Linux-Version"
4. source berxel_set_env.sh(设置动态库和头文件路径)
5. 进入catkin_ws目录 cd catkin_ws
6. catkin_make install
7. source install/setup.bash
8. roslaunch berxel_camera berxel_camera.launch  (PS:根据实际需要修改berxel_camera.launch中加载的实际xml文件，仅tf坐标区别)    

### 参数说明
"serial_no"                                 开启指定序列号设备，不填则默认开启设备列表中第一个设备
"usb_bus"                                   usb bus nubmber  (int)
"usb_port"                                  usb port number (str,  eg: "2" , "1-2", "1-2-3")
"camera"                                    指定节点名                     
"tf_prefix"                                 暂未使用，可通过camera 修改frame_id和tf命名                  
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
                                            泛光源 ： 640 * 400 帧率30
                                            点光源 ： 1280 * 800 帧率8
"depth_fps"                                 深度帧率  (640*400/ 320*200 可选帧率[5,10,15,20,25,30]   1280*800可选帧率 8)                
"enable_align"                              深度对齐彩色  true :  打开配准功能   false : 关闭配准功能               
"enable_pointcloud"                         点云使能开关  true : 开启点云发布  false ： 关闭点云发布
"enable_color_pointcloud"                   点云贴图彩色使能开关    true : 开启点云贴图彩色功能  false : 关闭点云贴图彩色功能
"enable_denoise"                            降噪使能开关    true : 打开降噪开关  false : 关闭降噪开关
"enable_temperature_compensation"           温度补偿使能开关    true ：打开温度补偿功能  false ：关闭温度补偿功能
"enable_distance_check"                     距离感应检查使能开关    true : 打开距离感应检测功能   false : 关闭距离感应使能开关
"enable_ordered_pointcloud"                 有序点云使能    true : 有序点云  false : 无序点云
"enable_device_timestamp"                   时间戳类型    true : 使用图像自带的时间戳    false : 使用ros::Time
"enable_invalid_point_data_zero"			有序点云无效点数据格式	 true : 无效点数据 0(会在坐标原点形成一个噪点)   false ： 无效点数据 0xFFFFFFFF
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


### launch文件说明
launch/include 目录下包含了很多xml文件。不同的xml对应不同的设备或者不同的安装方式，所有的xml仅tf参数不同。
文件名后缀带back表示安装点以相机背面中心位置为base_link
文件名后缀带bottom表示安装点以相机底部实际安装位置为base_link  
