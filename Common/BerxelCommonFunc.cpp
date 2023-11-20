#include <stdint.h>
#include <string.h>
#include <string>
#include <stdio.h>


#if defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>
#else
#include <sys/time.h>
#endif

#include "BerxelCommonFunc.h"

#define MAX_DEPTH 100000
#define MAXCOLORINDEX_FAR 8192
#define MAXCOLORINDEX_NEAR 4096

#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

#define MaxE(a,b)            (((a) > (b)) ? (a) : (b))
#define MinE(a,b)            (((a) < (b)) ? (a) : (b))

BerxelCommonFunc* BerxelCommonFunc::m_commonFunc = NULL;

BerxelCommonFunc::BerxelCommonFunc()
{
	onCreateColorBoard();
}



int BerxelCommonFunc::onCreateColorBoard()
{
	RGB888 rgb_t(RGB888{ 0, 0, 255 });
	for (int i = 0; i < MAXCOLORINDEX_NEAR; i++)
	{
		if (i < 1024 && rgb_t.g < 255) {
			if (i % 4 == 0)
				rgb_t.g++;
		}
		else if (i < 2048 && rgb_t.b > 0) {
			if (i % 4 == 0)
				rgb_t.b--;
		}
		else if (i < 3072 && rgb_t.r < 255) {
			if (i % 4 == 0)
				rgb_t.r++;
		}
		else if (i < 4096 && rgb_t.g > 0) {
			if (i % 4 == 0)
				rgb_t.g--;
		}

		m_NearColorBoar.push_back(rgb_t);
	}

	rgb_t.r = 0;
	rgb_t.g = 0;
	rgb_t.b = 255;
	for (int i = 0; i < MAXCOLORINDEX_FAR; i++)
	{
		if (i < 2048 && rgb_t.g < 255) {
			if (i % 8 == 0)
				rgb_t.g++;
		}
		else if (i < 4096 && rgb_t.b > 0) {
			if (i % 8 == 0)
				rgb_t.b--;
		}
		else if (i < 6144 && rgb_t.r < 255) {
			if (i % 8 == 0)
				rgb_t.r++;
		}
		else if (i < 8192 && rgb_t.g > 0) {
			if (i % 8 == 0)
				rgb_t.g--;
		}

		m_FarColorBoar.push_back(rgb_t);
	}

	return 0;
}


BerxelCommonFunc::~BerxelCommonFunc()
{
	if(m_commonFunc)
	{
		delete m_commonFunc;
		m_commonFunc = NULL;
	}
}

BerxelCommonFunc* BerxelCommonFunc::getInstance()
{
	if(m_commonFunc == NULL)
	{
		m_commonFunc = new BerxelCommonFunc();
	}
	return m_commonFunc;
}



void BerxelCommonFunc::ImageScaleColor(uint16_t* pSrcDepth, uint8_t* pDestColorDepth, int width, int height, berxel::BerxelHawkPixelType pixelType)
{

	uint16_t* pTempDepth = (uint16_t*)pSrcDepth;
	int32_t iSize = width * height;
	unsigned short udepth;
	int32_t iIndex = 0;
	for (int32_t i = 0; i < iSize; ++i)
	{
		udepth = pTempDepth[i];
		if (udepth > 0)
		{
			int index = udepth;
			if (pixelType == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_12I_4D)
			{
				index = index >> 4;
				if (index < 4096) {
					pDestColorDepth[iIndex + 2] = m_NearColorBoar.at(index).b;
					pDestColorDepth[iIndex + 1] = m_NearColorBoar.at(index).g;
					pDestColorDepth[iIndex] = m_NearColorBoar.at(index).r;
				}
				else {
					pDestColorDepth[iIndex + 2] = 0;
					pDestColorDepth[iIndex + 1] = 0;
					pDestColorDepth[iIndex] = 0;
				}
			}
			else
			{
				index = index >> 3;

				if (index < 8192) {
					pDestColorDepth[iIndex + 2] = m_FarColorBoar.at(index).b;
					pDestColorDepth[iIndex + 1] = m_FarColorBoar.at(index).g;
					pDestColorDepth[iIndex] = m_FarColorBoar.at(index).r;
				}
				else {
					pDestColorDepth[iIndex + 2] = 0;
					pDestColorDepth[iIndex + 1] = 0;
					pDestColorDepth[iIndex] = 0;
				}
			}
		}
		else
		{
			pDestColorDepth[iIndex + 2] = 0;
			pDestColorDepth[iIndex + 1] = 0;
			pDestColorDepth[iIndex] = 0;
		}
		iIndex += 3;
	}

}


void BerxelCommonFunc::covertHist(float* pHist, int histSize, uint16_t *psrcData, uint32_t srcwidth, uint32_t srcheight)
{
	const uint16_t* pDepth = (const uint16_t*)psrcData;
	unsigned int nPointsCount = 0;

	memset(pHist, 0, histSize*sizeof(float));
	int height = srcheight;
	int width  = srcwidth;
	
	for(int y = 0; y < height; ++y)
	{
		for(int x = 0; x < width; ++x, ++pDepth)
		{
			if(*pDepth != 0)
			{
				pHist[*pDepth]++;
				nPointsCount++;
			}
		}
	}

	for(int nIndex = 1; nIndex < histSize; ++nIndex)
	{
		if (nIndex != 4095 && nIndex != 8191) {

			pHist[nIndex] += pHist[nIndex - 1];
		}
	}

	if(nPointsCount)
	{
		for(int nIndex = 1; nIndex < histSize; ++nIndex)
		{
			pHist[nIndex] = (256 * (1.0f - (pHist[nIndex] / nPointsCount)));
		}
	}
}




void BerxelCommonFunc::convertDepthToRgbByHist(uint16_t* pDepth, RGB888* pRgb ,int width, int height,berxel::BerxelHawkPixelType pixelType)
{

	memset(m_depthHist, 0, sizeof(m_depthHist));
	memcpy(m_depthOri, pDepth, width * height *2);

	/* 如何获取深度图精度
	1. pixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_12I_4D
		深度图共16位,前面12位为整数部分，单位是1mm，后面4位位小数部分，单位是0.0625mm

		uint16_t depthOri = pDepth[i];

		float  depthFront = depthOri >> 4;
		float  depthTail  = (depthOri & 0x000f)/16; 

		float depth =depthFront + depthTail;


	2. pxelType ==  BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D 
		  
		深度图共16位,前面13位为整数部分，单位是1mm，后面3位位小数部分，单位是0.125mm

		uint16_t depthOri = pDepth[i];

		float  depthFront = depthOri >> 3;
		float  depthTail  = (depthOri & 0x0007)/8; 

		float depth =depthFront + depthTail;

	*/
		
		//获取深度图整数部分

	if(pixelType == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D)
	{
		for(uint32_t i = 0; i < width * height ; ++i)
		{
			m_depthOri[i] = m_depthOri[i] >> 3;
		}
	}
	else
	{
		for(uint32_t i = 0; i < width * height ; ++i)
		{
			m_depthOri[i] = m_depthOri[i] >> 4;
		}
	}
		
	//获取深度图整数部分
	covertHist(m_depthHist, MAX_DEPTH_HIST, (uint16_t *)m_depthOri, width, height);	
	for(int i = 0; i < width * height; ++i) 
	{
		pRgb[i].r = m_depthHist[m_depthOri[i]];
		pRgb[i].g = pRgb[i].r;
		pRgb[i].b = 0;
	}
}


void BerxelCommonFunc::convertDepthToRGB(uint16_t* pDepth, RGB888* pRgb,int width , int height, berxel::BerxelHawkPixelType pixelType)
{
	//uint16_t* pde = (uint16_t*)pHawkFrame->getData();
	
	/* 如何获取深度图精度
	1. pixelType == BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_12I_4D
		 深度图共16位,前面12位为整数部分，单位是1mm，后面4位位小数部分，单位是0.0625mm

			uint16_t depthOri = pDepth[i];

			float  depthFront = depthOri >> 4;
			float  depthTail  = (depthOri & 0x000f)/16; 

			float depth =depthFront + depthTail;


	  2. pxelType ==  BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D 
		  
		  深度图共16位,前面13位为整数部分，单位是1mm，后面3位位小数部分，单位是0.125mm

		  uint16_t depthOri = pDepth[i];

		  float  depthFront = depthOri >> 3;
		  float  depthTail  = (depthOri & 0x0007)/8; 

		  float depth =depthFront + depthTail;

	  */
		
		//获取深度图整数部分
	if(pixelType == berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D)
	{
		for(uint32_t i = 0; i < width * height ; ++i)
		{	
			uint16_t depthOri = pDepth[i] >> 3;

			//将深度图转换为RGB
			pRgb[i].r = depthOri >> 3;
			pRgb[i].g = pRgb[i].r;
			pRgb[i].b = pRgb[i].r;
		} 
	}
	else
	{
		for(uint32_t i = 0; i < width * height ; ++i)
		{	
			uint16_t depthOri = pDepth[i] >> 4;

			//将深度图转换为RGB
			pRgb[i].r = depthOri >> 3;
			pRgb[i].g = pRgb[i].r;
			pRgb[i].b = pRgb[i].r;
		} 
	}
}

void BerxelCommonFunc::convertIrToRGB(uint16_t* pIr, RGB888* pRgb,  int width, int height)
{
	for(uint32_t i = 0; i < width * height; ++i)
	{
		pRgb[i].r = pIr[i] >> 2;
		pRgb[i].g = pRgb[i].r;
		pRgb[i].b = pRgb[i].r;
	} 

}

int32_t BerxelCommonFunc::takePhoto(const char* imageName,int index, const uint8_t* pframe, int width, int height)
{

	char bmpImagePath[128] = {0};

	sprintf(bmpImagePath,"%s_%d.bmp" ,imageName,  index);


	BMPHEADER bmfh; // bitmap file header
	BMPINFO bmih; // bitmap info header (windows)

	const int OffBits = 54;

	int32_t imagePixSize = width * height;

	memset(&bmfh, 0, sizeof(BMPHEADER));
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfType      = 0x4d42;
	bmfh.bfOffBits   = OffBits; // 头部信息54字节
	bmfh.bfSize      = imagePixSize * 3 + OffBits;

	memset(&bmih, 0, sizeof(BMPINFO));
	bmih.biSize      = 40; // 结构体大小为40
	bmih.biPlanes    = 1;
	bmih.biSizeImage = imagePixSize * 3;

	bmih.biBitCount    = 24;
	bmih.biCompression = 0;
	bmih.biWidth       = width;
	bmih.biHeight      = height;

	// rgb -> bgr
	RGB888* pRgb = (RGB888*)m_bmpColor;
	RGB888* pSrc = (RGB888*)pframe;
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

	char buf[128]= {0};
	std::string fullPath = bmpImagePath;

	FILE* pSaveBmp = fopen(fullPath.c_str(), "wb");
	if(NULL == pSaveBmp)
	{
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

void BerxelCommonFunc::saveRawData(uint8_t* pRawData, int dataSize , string stringName, int index)
{
	char strRawDataName[128] =  {0};
	sprintf(strRawDataName,"%s_%d.raw" ,stringName.c_str(),  index);

	FILE* pFile = fopen(strRawDataName, "wb");
	if(pFile)
	{
		fwrite(pRawData, dataSize, 1, pFile);
		fclose(pFile);
		printf("save raw data  Success !\n");
	}
	else
	{
		printf("save raw data  Failed !\n");
	}
}







