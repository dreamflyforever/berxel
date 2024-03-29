#include "core.hpp"

void save_raw_data(uint8_t* pRawData,
			int dataSize,
			string stringName,
			int index)
{
	char strRawDataName[128] =  {0};
	sprintf(strRawDataName,"%s_%d.raw" ,stringName.c_str(),  index);

	FILE* pFile = fopen(strRawDataName, "wb");
	if (pFile) {
		fwrite(pRawData, dataSize, 1, pFile);
		fclose(pFile);
		printf("save raw data  Success !\n");
	} else {
		printf("save raw data  Failed !\n");
	}
}

/*user API*/
void * rgb_cb(void * argv)
{
	static int index;
	img_str * entity = (img_str *)argv;
	os_printf("color picture handle , %d\n", entity->size);
	save_raw_data(entity->img, entity->size, "color", index++);
	//std::shared_ptr<ob::ColorFrame> colorFrame = *(std::shared_ptr<ob::ColorFrame> *) argv;
	return NULL;
}

/*user API*/
void * depth_cb(void * argv)
{
	static int index;
	img_str * entity = (img_str *)argv;
	os_printf("depth picture handle, %d\n", entity->size);
	save_raw_data(entity->img, entity->size, "depth", index++);
	return NULL;
}


int main()
{
	berxel_str * entity;
	set_view(1);
	rgbd_init(&entity, rgb_cb, depth_cb);
	rgb_start(entity);
	depth_start(entity);
	berxel_run(entity);
	while (1){
		sleep(10);
	};
	rgb_stop(entity);
	depth_stop(entity);
	rgbd_deinit(entity);
}
