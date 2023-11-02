
#ifndef __BERXEL_IMAGE_RENDER_H__
#define __BERXEL_IMAGE_RENDER_H__

#include <string>
#include <stdint.h>
#include <string.h>
#include "BerxelHawkDefines.h"
#include "BerxelCommonFunc.h"
#include "BerxelHawkFrame.h"
#if defined(_WIN32)
#include <GL/glew.h>
#endif

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>


typedef struct _WinRect
{
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;

	_WinRect()
	{
		memset(this, 0, sizeof(*this));
	}

	_WinRect(uint32_t param_x, uint32_t param_y, uint32_t param_w, uint32_t param_h):
			  x(param_x),y(param_y),w(param_w),h(param_h)
	{
	}

}WinRect;

typedef struct _Positions
{
	int32_t x;
	int32_t y;
	
	_Positions()
	{
		memset(this, 0, sizeof(*this));
	}
}Positions;

typedef bool (*berxelImageDataCallback)();
typedef void (*berxelKeyboardCallback)(unsigned char key);
typedef void (*berxelMouseCallback)(int button, int state, int x, int y);

class BerxelImageRender
{
public:
	BerxelImageRender(int32_t argc, char **argv, const char* winName, const uint32_t winWidth, const uint32_t winHeight);
	virtual ~BerxelImageRender();

	bool startView();
	void initView();
	void updateView(); 

	
	virtual void drawColorImage(uint8_t* pRgbBuffer, int imageWidth, int imageHieght, const WinRect& rect);
	virtual void drawColorImage2(uint8_t* pRgbBuffer, int imageWidth, int imageHieght, const WinRect& rect);
	
	void drawString(const char* str, uint32_t x, uint32_t y, void* font);
    void drawRect(int x, int y, int width, int height, WinRect& rect);
	void drawLine(int x, int y, int width, int height );

	void drawLine2(int x1, int y1, int x2, int y2, char* buf);

	void setInfoCallback(berxelImageDataCallback  imageCB , berxelKeyboardCallback  keyCB);
	void setMosueCallback(berxelMouseCallback  mouseCB){m_pMouseCallBack = mouseCB ;}

	void drawDepthValue(berxel::BerxelHawkFrame* pFrame, const WinRect& rect);

	void drawPoint(int x ,int y, const WinRect& rect);

protected:
	virtual void display();
	virtual void onKey(unsigned char key, int32_t x, int32_t y);
private:
	/*BerxelImageRender(const BerxelImageRender&);
	BerxelImageRender& operator=(BerxelImageRender&);*/

	static void glutIdleCallBack();
	static void glutDisplayCallBack();
	static void glutWindowReshapeCallBack(int width, int height);	
	static void glutKeyboardCallBack(unsigned char key, int x, int y);
	static void glutMouseCallBack(int button, int state, int x, int y);

private:
	berxelImageDataCallback   m_pImageDataCB;
	berxelKeyboardCallback	  m_pKeyboardCB;
	
	berxelMouseCallback       m_pMouseCallBack;
private:

	static BerxelImageRender* m_pImageRender;

	uint32_t        m_nTexMapX;
    uint32_t        m_nTexMapY;
	RGB888*			m_pTexMap;

	WinRect 		m_rect;
	

	
	uint32_t 			m_glWin;	
	static Positions	m_CursorPos;
	std::string			m_strWindowsName;

	 GLuint texture;

	 int m_nWindowRealWidth;
	 int m_nWindowHeight;
};


#endif 
