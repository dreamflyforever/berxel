#include <stdio.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unistd.h>

// Berxel Head File
#include "BerxelHawkContext.h"
#include "BerxelHawkDevice.h"
#include "BerxelHawkFrame.h"
#include "BerxelHawkDefines.h"
#include "BerxelCommonFunc.h"
#include "BerxelImageRender.h"

/* debug printf*/
#define DEBUG 1
#if DEBUG
#define os_printf(format, ...) \
	{printf("[%s : %s : %d] ", \
	__FILE__, __func__, __LINE__); \
	printf(format, ##__VA_ARGS__);}
#else
#define os_printf(format, ...) 
#endif

typedef void *(*func_cb)(void * argv);

typedef struct berxel_str {
	//ob::Config * config;
	bool rgb_flag;
	bool depth_flag;
	func_cb rgb_cb;
	func_cb depth_cb;
	double rgb_count;
	double depth_count;
} berxel_str;


typedef struct img_str {
	uint8_t * img;
	int size;
} img_str;

int rgbd_init(berxel_str ** entity, func_cb rgb, func_cb depth);
int depth_stop(berxel_str * entity);
int rgbd_deinit(berxel_str * entity);
int depth_start(berxel_str * entity);
int rgb_start(berxel_str * entity);
int rgb_stop(berxel_str * entity);
int berxel_run(berxel_str * entity);
/*1: view 
  0: close view
*/
int set_view(bool flag);
bool get_view();
