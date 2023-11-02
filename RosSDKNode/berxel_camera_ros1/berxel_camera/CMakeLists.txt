cmake_minimum_required(VERSION 2.8.12)
project(berxel_camera)

find_package(catkin REQUIRED COMPONENTS
  message_generation
  camera_info_manager
  image_transport
  sensor_msgs
  roscpp
  rospy
  std_msgs
  tf
  dynamic_reconfigure
)
#find_package(PCL REQUIRED) 

generate_dynamic_reconfigure_options(
    config/berxel_camera.cfg
  )

add_service_files(
  FILES
  berxel_camera_image.srv
  berxel_device_log.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs
  sensor_msgs
)



catkin_package(

)


include_directories(
  ${catkin_INCLUDE_DIRS}
#  ${PCL_INCLUDE_DIRS}
  include
)

set(CMAKE_CXX_FLAGS "-O2 -fPIE -fPIC -std=c++11 -D_FORTIFY_SOURCE=2 -fstack-protector -Wformat -Wformat-security -Wall ${CMAKE_CXX_FLAGS}")

if(DEFINED ENV{BERXEL_SDK_LIBRARY})
    set(BERXEL_SDK_INCLUDE $ENV{BERXEL_SDK_LIBRARY}/Include)
    set(BERXEL_SDK_LIB $ENV{BERXEL_SDK_LIBRARY}/libs)
    include_directories(${BERXEL_SDK_INCLUDE})
    LINK_DIRECTORIES(${BERXEL_SDK_LIB})
else()
    include_directories(../../../Include/)
    LINK_DIRECTORIES(../../../libs/)
endif()
add_executable(${PROJECT_NAME} src/BerxelRebotCamera.cpp src/BerxelHawkCamera.cpp)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_generate_messages_cpp)
target_link_libraries(
   ${PROJECT_NAME}
   ${catkin_LIBRARIES}
   BerxelHawk
)

# Install nodelet library
install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
    )

if(DEFINED ENV{BERXEL_SDK_LIBRARY})
    file(GLOB_RECURSE BERXEL_SHARED_LIBS ${BERXEL_SDK_LIB}/*)
    file(GLOB_RECURSE BERXEL_INCLUDE ${BERXEL_SDK_INCLUDE}/*.h)
    install(FILES ${BERXEL_SHARED_LIBS} DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
    install(FILES ${BERXEL_INCLUDE} DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
else()
    file(GLOB_RECURSE BERXEL_SHARED_LIBS ../../../libs/*)
    file(GLOB_RECURSE BERXEL_INCLUDE ../../../Include/*.h)
    install(FILES ${BERXEL_SHARED_LIBS} DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
    install(FILES ${BERXEL_INCLUDE} DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})
endif()

install(DIRECTORY include
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
    )

install(DIRECTORY launch/
    DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
    )

# install(DIRECTORY rviz/
#     DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/rviz
#     )

