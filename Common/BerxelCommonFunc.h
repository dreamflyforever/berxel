#ifndef __BERXEL_COMMON_FUNC_H__
#define __BERXEL_COMMON_FUNC_H__

#include<stdint.h>
#include <string>
#include <vector>
#include "BerxelHawkDefines.h"

#define MAX_DEPTH_HIST 9000
#pragma pack (push, 1)

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
}RGB888;


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

#pragma pack (pop)

using namespace std;

class BerxelCommonFunc
{
private:
	BerxelCommonFunc();

protected:
	   ~BerxelCommonFunc();

public: 	
	static BerxelCommonFunc* getInstance();

private:

	int onCreateColorBoard();

public:
	void ImageScaleColor(uint16_t* pSrcDepth, uint8_t* pDestColorDepth, int height, int width,  berxel::BerxelHawkPixelType pixelType);

	void covertHist(float* pHist, int histSize, uint16_t *psrcData, uint32_t srcwidth, uint32_t srcheight);
	void convertDepthToRGB(uint16_t* pDepth, RGB888* pRgb ,int width, int height, berxel::BerxelHawkPixelType pixelType);
	void convertDepthToRgbByHist(uint16_t* pDepth, RGB888* pRgb ,int width, int height, berxel::BerxelHawkPixelType pixelType);
	void convertIrToRGB(uint16_t* pIr, RGB888* pRgb ,int width, int height);
	int32_t takePhoto(const char* imageName,int index, const uint8_t* pframe, int width, int height);
	void saveRawData(uint8_t* pRawData, int dataSize , string stringName, int index);

private:
	static BerxelCommonFunc* m_commonFunc;
	int32_t		m_bmpColor[1920*1080*3];
	uint16_t    m_depthOri[1280 *800];
	float       m_depthHist[MAX_DEPTH_HIST];
	std::vector<RGB888>   m_NearColorBoar;
	std::vector<RGB888>   m_FarColorBoar;
};





#endif