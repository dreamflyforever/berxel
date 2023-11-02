### Usage Instructions
The ROS driver package only supports the version released by ROS1. Please refer to the official ROS tutorial for setting up the environment.
This driver package needs to be used with the "Berxel SDK".

1. mkdir -p catkin_ws/src
2. Copy the package "BerxelSDK-Linux-XXXXX.tar.gz" to the "catkin_ws/src" directory, unpack the package, and delete the "RosSDKNode/berxel_camera_ros2" directory.(The compilation method of ros2 is different from that of ros1, which will cause compilation failure)
3. cd catkin_ws
4. catkin_make install
5. source install/setup.bash
6. roslaunch berxel_camera berxel_camera.launch
7. If you use multiple devices，please use the launch "berxel_camera_multi_device.launch".
            1.You can enable multiple devices by configuring serial numbers. $(arg serial_no1) $(arg serial_no2)
            2.You can enable multiple devices by configuring the 'bus-port'.
            $(arg usb_bus1) $(arg usb_bus2) $(arg usb_port1) $(arg usb_port2)

If you just use the berxel_camera directory : 
1. mkdir -p catkin_ws/src
2. Copyt the "berxel_camera" directory to the "catkin_ws/src" directory.
3. Sets environment variables for dynamic libraries and include file. (See CMakeLists.txt)
4. cd catkin_ws
5. catkin_make install
6. source install/setup.bash
7. roslaunch berxel_camera berxel_camera.launch

### Params Instructions
"serial_no"                                 Open the device by serial number
"usb_bus"                                   Usb bus nubmber  (int)
"usb_port"                                  Usb port number (str,  eg: "2" , "1-2", "1-2-3")
"camera"                                    Node name                     
"tf_prefix"                                 Unused            
"stream_flag"                               Stream Mode  1 ： Single stream 
                                                         2 :  Mix VGA (rgb + depth  640*400)
                                                         3 ： Mix HD (rgb + depth 1280*800)
                                                         4 ： Mix QVGA (rgb + depth 320*200)

"stream_type"                               Stream Type  1 ： color
                                                         2 ： depth
                                                         3 ： color + depth
                                                         4 ： ir
                                                         5 ： light ir                  
"color_width"                               Rgb width                  
"color_height"                              Rgb height                  
"depth_width"                               Dpeht width                  
"depth_height"                              Depth height                  
"ir_width"                                  Ir width                  
"ir_height"                                 Ir height
                                            Ir ： 640 * 400 @30fps
                                            Light ir ： 1280 * 800 @8fps
"depth_fps"                                 Depth fps  (640*400/ 320*200 @fps[5,10,15,20,25,30]   1280*800-> 8fps)                
"enable_align"                              Dpeht align to rgb  true : enable   false : disable               
"enable_pointcloud"                         Publish point cloud  true : enable  false ： disable
"enable_color_pointcloud"                   Publish the rgb + point cloud  true : enable  false : disable
"enable_denoise"                            Depth denoise    true : enable  false : disable
"enable_temperature_compensation"           Temperature compensation    true ：enable  false ：disable
"enable_distance_check"                     Distance check   true : enable   false : disable
"enable_ordered_pointcloud"                 Ordered point cloud    true : ordered point cloud  false : disordered point cloud
"enable_device_timestamp"                   Timestamp type    true : use image timestamp    false : use ros::time
"enable_invalid_point_data_zero"			Ordered point cloud invalid point data format  true : 0   false : 0xFFFFFFFF
"enable_set_depth_Confidence"				Enable depth confidence	  true : enable  false : disable
"depth_confidence"							Depth confidece value		range : [3-5]
"depth_current"                             Laser current          range [8-15] (*100 ma)
"enable_depth_ae"                           Enable depth auto exposure      true: enable   false : disable
"depth_exposure_time"                       Depth exposure time        range[1-43] ( *0.1ms)
"depth_gain"                                Depth gain            range[1-4]
"enable_edge_optimization"                  Edge optimization    true : enable   false : disable
"enable_hight_fps_mode"                     Hight fps mode  true : enable  false : disable
"enable_adjust_ae_gain_range"               Enable adjust the range that depth gain     true : enable  false : disable
"depth_ae_gain_range_min"                   Lower limit of range [1-4]      Need depth auto exposure
"depth_ae_gain_range_max"                   Lower limit of range [1-4]      Need depth auto exposure