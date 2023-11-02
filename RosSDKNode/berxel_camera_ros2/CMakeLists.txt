cmake_minimum_required(VERSION 3.5)
project(berxel_camera_ros2)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_FLAGS "-O2 -fPIE -fPIC -D_FORTIFY_SOURCE=2 -fstack-protector -Wformat -Wformat-security -Wall ${CMAKE_CXX_FLAGS}")

find_package(ament_cmake REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2 REQUIRED)

include_directories(
  include
)

if(DEFINED ENV{BERXEL_SDK_LIBRARY})
    set(BERXEL_SDK_INCLUDE $ENV{BERXEL_SDK_LIBRARY}/Include)
    set(BERXEL_SDK_LIB $ENV{BERXEL_SDK_LIBRARY}/libs)
    include_directories(${BERXEL_SDK_INCLUDE})
    LINK_DIRECTORIES(${BERXEL_SDK_LIB})
else()
    include_directories(../../Include/)
    LINK_DIRECTORIES(../../libs/)
endif()

add_executable(${PROJECT_NAME}
  include/berxel_camera_ros2_driver.h
  src/berxel_camera_ros2_driver.cpp
  src/berxel_camera_ros2_node.cpp
)

target_link_libraries(${PROJECT_NAME} BerxelHawk)

ament_target_dependencies(${PROJECT_NAME}
  cv_bridge
  image_transport
  rclcpp
  std_msgs
  sensor_msgs
  tf2
  tf2_ros
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

if(DEFINED ENV{BERXEL_SDK_LIBRARY})
    file(GLOB_RECURSE BERXEL_SHARED_LIBS ${BERXEL_SDK_LIB}/*)
    file(GLOB_RECURSE BERXEL_INCLUDE ${BERXEL_SDK_INCLUDE}/*.h)
    install(FILES ${BERXEL_SHARED_LIBS} DESTINATION lib)
    install(FILES ${BERXEL_INCLUDE} DESTINATION include)
else()
    file(GLOB_RECURSE BERXEL_SHARED_LIBS ../../libs/*)
    file(GLOB_RECURSE BERXEL_INCLUDE ../../Include/*.h)
    install(FILES ${BERXEL_SHARED_LIBS} DESTINATION lib)
    install(FILES ${BERXEL_INCLUDE} DESTINATION include)
endif()

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

install(DIRECTORY
  config
  DESTINATION share/${PROJECT_NAME}/
)

install(DIRECTORY
  rviz
  DESTINATION share/${PROJECT_NAME}/
)


ament_package()


