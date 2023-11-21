
#if defined(_WIN32)
#include <GL/glew.h>
#endif

#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>

#include "BerxelImageRender.h"
#include "BerxelHawkDefines.h"

#if defined(_WIN32)
	#define BERXELSnprintf _snprintf_s
#else
	#define BERXELSnprintf snprintf
#endif


Positions BerxelImageRender::m_CursorPos;

BerxelImageRender* BerxelImageRender::m_pImageRender = NULL;

void BerxelImageRender::glutKeyboardCallBack(unsigned char key, int x, int y)
{
	m_pImageRender->onKey(key, x, y);
}
BerxelImageRender::BerxelImageRender(int32_t argc, char **argv,const char* pWinName, const uint32_t winWidth, const uint32_t winHeight) :
	m_strWindowsName(pWinName),
	m_nTexMapX(winWidth),
	m_nTexMapY(winHeight),
	m_pTexMap(NULL),
	m_pImageDataCB(NULL),
	m_pKeyboardCB(NULL),
	m_pMouseCallBack(NULL),
	m_glWin(0),
	m_nWindowRealWidth(winWidth),
	m_nWindowHeight(winHeight)
	
{
	m_pImageRender = this;
	memset(&m_rect, 0, sizeof(m_rect));

	m_pTexMap = new RGB888[m_nTexMapX * m_nTexMapY];
	//return initOpenGL(argc, argv);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(m_nTexMapX, m_nTexMapY);
	m_glWin = glutCreateWindow (m_strWindowsName.c_str());
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glutKeyboardFunc(glutKeyboardCallBack);
	glutDisplayFunc(glutDisplayCallBack);
	glutReshapeFunc(glutWindowReshapeCallBack);
	glutMouseFunc(glutMouseCallBack);
	glClearColor(0.275f, 0.353f, 0.392f, 1.0f);

}

BerxelImageRender::~BerxelImageRender()
{
	if( NULL != m_pTexMap )
	{
		delete[] m_pTexMap;
		m_pTexMap = NULL;
	}
	m_pImageRender = NULL;
}



void BerxelImageRender::setInfoCallback(berxelImageDataCallback renderImage, berxelKeyboardCallback  keyCB)
{
	m_pImageDataCB = renderImage;
	m_pKeyboardCB = keyCB;
}

bool BerxelImageRender::startView()
{
    if(m_pImageDataCB) 
	{
        glutIdleFunc(glutIdleCallBack);
    }

    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
	glutMainLoop();
	return true;
}

void BerxelImageRender::initView()
{
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, m_nTexMapX, m_nTexMapY, 0, -1.0, 1.0);
}

void BerxelImageRender::updateView()
{
	glutSwapBuffers();
}

void BerxelImageRender::display()
{
	if(NULL != m_pImageDataCB)
	{
		m_pImageDataCB();
	}
}


void BerxelImageRender::drawString(const char* str, uint32_t x, uint32_t y,  void* font)
{
	if (NULL == str)
	{
		return;
	}

	glDisable(GL_TEXTURE_2D);
	glRasterPos2i(x, y);

	for (; *str != '\0'; ++str)
	{
		if( font ==   ((void *)0x0008))
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);	
		}
		else
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *str);
		}

		//glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *str);	
		//glutBitmapCharacter(font, *str);
	}

	glEnable(GL_TEXTURE_2D);
}


void BerxelImageRender::drawRect(int x, int y, int width, int height, WinRect& rect)
{
	glDisable(GL_TEXTURE_2D);

	float coordinates1[6] = {x+rect.x, y, 0, x+width+rect.x, y, 0};

	glLineWidth(1.0);

	glColor3f(1.0, 0.0, 0.0);
	glPointSize(2);
	glVertexPointer(3, GL_FLOAT, 0, coordinates1);
	glDrawArrays(GL_LINES, 0, 2);

	float coordinates2[6] = {x+rect.x, y+rect.y, 0, x+rect.x, y+rect.y+height, 0};
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(2);
	glVertexPointer(3, GL_FLOAT, 0, coordinates2);
	glDrawArrays(GL_LINES, 0, 2);

	float coordinates3[6] = {x+width+rect.x, y+rect.y, 0, x+width+rect.x, y+rect.y+height, 0};
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(2);
	glVertexPointer(3, GL_FLOAT, 0, coordinates3);
	glDrawArrays(GL_LINES, 0, 2);

	float coordinates4[6] = {x+rect.x, y+rect.y+height, 0, x+width+rect.x, y+rect.y+height, 0};
	glColor3f(1.0, 0.0, 0.0);
	glPointSize(2);
	glVertexPointer(3, GL_FLOAT, 0, coordinates4);
	glDrawArrays(GL_LINES, 0, 2);

	glEnable(GL_TEXTURE_2D);
}

void BerxelImageRender::drawDepthValue(berxel::BerxelHawkFrame* pFrame, const WinRect& rect)
{
	if(NULL == pFrame)
	{
		return;
	}

	if ((m_CursorPos.x < rect.x) || (m_CursorPos.y < rect.y) || (m_CursorPos.x >= rect.x + rect.w) || (m_CursorPos.y >= (rect.y + rect.h)))
	{
		return;
	}

	//Draw Cursor X,Y,Z Value

	uint32_t curIndex = pFrame->getWidth()*(m_CursorPos.y - rect.y)*((pFrame->getHeight()*1.0)/rect.h) + (m_CursorPos.x - rect.x)*((pFrame->getWidth()*1.0)/rect.w);

	if( curIndex >= ( pFrame->getWidth() * pFrame->getHeight()))
	{
		return;
	}
	uint16_t* pData = (uint16_t *)pFrame->getData() ;

	static float depthArray[30] = { 0 };
	static int index  =0 ;
	static float squareValue = 0.0f;

	//for(int i = 0 ; i <30 ; i++)
	uint16_t oriDepth = pData[curIndex];
	float   depth = 0.0;
	if (pFrame->getPixelType() !=  berxel::BERXEL_HAWK_PIXEL_TYPE_DEP_16BIT_13I_3D)
		depth = (oriDepth * 1.0)/16;
	else
		depth = (oriDepth * 1.0)/8;
	index ++;

	

	if(index <= 10)
	{
		depthArray[index -1] = depth;
		if(index== 10)
		{
			squareValue = 0.0f;

			
			int notZeroCount = 0;
			for(int i = 0 ; i <10 ; i++)
			{
				if (depthArray[i] != 0) {
					notZeroCount++;
				}

				squareValue += depthArray[i];		
			}

			if (notZeroCount != 0) {
				squareValue = (squareValue * 1.0) / (notZeroCount * 1.0);
			}
			else {
				squareValue = 0;
			}
			

			index = 0;
		}
	}

	glDisable(GL_TEXTURE_2D);

	//Draw Cursor Point
	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glVertex2i(m_CursorPos.x, m_CursorPos.y);
	glEnd();


	glEnable(GL_TEXTURE_2D);
	char buff[32];

	int XposOffset = 5;
	int YposOffsetX = 20;
	int YposOffsetY = 40;
	int YposOffsetZ = 60;
	if(m_CursorPos.x+85 > (rect.x + rect.w)) {
		XposOffset = -85;
	}
	if(m_CursorPos.y+45 > (rect.y + rect.h)) {
		YposOffsetX = -45;
		YposOffsetY = -25;
		YposOffsetZ = -5;
	}

	memset(buff, 0, sizeof(buff));
	BERXELSnprintf(buff, 16, "X=%d", m_CursorPos.x - rect.x);
	drawString(buff, m_CursorPos.x + XposOffset, m_CursorPos.y + YposOffsetX ,GLUT_BITMAP_HELVETICA_18);

	memset(buff, 0, sizeof(buff));
	BERXELSnprintf(buff, 16, "Y=%d", m_CursorPos.y - rect.y);
	drawString(buff, m_CursorPos.x + XposOffset, m_CursorPos.y + YposOffsetY , GLUT_BITMAP_HELVETICA_18);

	memset(buff, 0, sizeof(buff));
	BERXELSnprintf(buff, 16, "Z=%0.4f", squareValue);
	drawString(buff, m_CursorPos.x + XposOffset, m_CursorPos.y + YposOffsetZ , GLUT_BITMAP_HELVETICA_18);
}



void BerxelImageRender::drawPoint(int x ,int y, const WinRect& rect)
{


	/*if ((m_CursorPos.x < rect.x) || (m_CursorPos.y < rect.y) || (m_CursorPos.x >= rect.x + rect.w) || (m_CursorPos.y >= (rect.y + rect.h)))
	{
	return;
	}*/

	glDisable(GL_TEXTURE_2D);

	//Draw Cursor Point
	glColor3f(1.0f, 0.0f, 0.0f);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();


	glEnable(GL_TEXTURE_2D);
	/*char buff[32];

	int XposOffset = 5;
	int YposOffsetX = 20;
	int YposOffsetY = 40;
	int YposOffsetZ = 60;
	if(m_CursorPos.x+85 > (rect.x + rect.w)) {
	XposOffset = -85;
	}
	if(m_CursorPos.y+45 > (rect.y + rect.h)) {
	YposOffsetX = -45;
	YposOffsetY = -25;
	YposOffsetZ = -5;
	}

	memset(buff, 0, sizeof(buff));
	BERXELSnprintf(buff, 16, "X=%d", x - rect.x);
	drawString(buff, x + XposOffset, y + YposOffsetX ,GLUT_BITMAP_HELVETICA_18);

	memset(buff, 0, sizeof(buff));
	BERXELSnprintf(buff, 16, "Y=%d", y - rect.y);
	drawString(buff, x + XposOffset, y + YposOffsetY , GLUT_BITMAP_HELVETICA_18);*/

}


void BerxelImageRender::drawLine(int x, int y, int width, int height)
{
	float pointers[6] = {x, y, 0, x+width, y, 0};

	glDisable(GL_TEXTURE_2D);
	
	glLineWidth(0.8);
	glPointSize(2);
	glColor3f(1.0, 1.0, 1.0);
	glVertexPointer(3, GL_FLOAT, 0, pointers);
	glDrawArrays(GL_LINES, 0, 2);
	glEnable(GL_TEXTURE_2D);



}


void BerxelImageRender::drawLine2(int x1, int y1, int x2, int y2,  char* buff)
{
	float pointers[6] = {x1, y1, 0, x2, y2, 0};

	glDisable(GL_TEXTURE_2D);

	glLineWidth(0.8);
	glPointSize(2);
	glColor3f(1.0, 0, 0);
	glVertexPointer(3, GL_FLOAT, 0, pointers);
	glDrawArrays(GL_LINES, 0, 2);
	glEnable(GL_TEXTURE_2D);

	drawString(buff, (0x1 + x2)/2, (y1 + y2)/2 ,GLUT_BITMAP_HELVETICA_18);
}



void BerxelImageRender::drawColorImage(uint8_t* pRgbBuffer, int imageWidth, int imageHieght, const WinRect& rect)
{
	static RGB888 tmpColorMap[1920 * 1080] = {0};
	int tmpTexMapX = imageWidth;
	int tmpTexMapY = imageHieght;

	memset(tmpColorMap, 0, tmpTexMapX * tmpTexMapY * sizeof(RGB888));

	memcpy(tmpColorMap,pRgbBuffer ,imageWidth * imageHieght *3);

	

	//glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tmpTexMapX, tmpTexMapY, 0, GL_RGB, GL_UNSIGNED_BYTE, tmpColorMap);

	// Display the OpenGL texture map
	glColor4f(1,1,1,1);

	glBegin(GL_QUADS);

	int32_t nXRes = imageWidth; 
	int32_t nYRes = imageHieght; 

	// upper left
	glTexCoord2f(0, 0);
	glVertex2f((GLfloat)rect.x, (GLfloat)rect.y);
	// upper right
	glTexCoord2f((float)nXRes/(float)tmpTexMapX, 0);
	glVertex2f((GLfloat)(rect.x + rect.w), (GLfloat)rect.y);
	// bottom right
	glTexCoord2f((float)nXRes/(float)tmpTexMapX, (float)nYRes/(float)tmpTexMapY);
	glVertex2f((GLfloat)(rect.x + rect.w), (GLfloat)(rect.y +  + rect.h));
	// bottom left
	glTexCoord2f(0, (float)nYRes/(float)tmpTexMapY);
	glVertex2f((GLfloat)rect.x, (GLfloat)(rect.y  + rect.h));

	glEnd();

	

}


void BerxelImageRender::drawColorImage2(uint8_t* pRgbBuffer, int imageWidth, int imageHieght, const WinRect& rect)	
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHieght, 0, GL_RGB, GL_UNSIGNED_BYTE, pRgbBuffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

	glBegin(GL_QUADS);
	glColor4f(1.0f, 1.0f, 1.0f, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUAD_STRIP);
	{
		glTexCoord2f(0.f, 1.f);
		glVertex2f(rect.x, rect.y +rect.h);
		glTexCoord2f(0.f, 0.f);
		glVertex2f(rect.x,  rect.y);
		glTexCoord2f(1.f, 1.f);
		glVertex2f(rect.x + rect.w, rect.y + rect.h);
		glTexCoord2f(1.f, 0.f);
		glVertex2f(rect.w + rect.x, rect.y);
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);

}


void BerxelImageRender::glutIdleCallBack()
{
	BerxelImageRender::m_pImageRender->display();
}

void BerxelImageRender::glutDisplayCallBack()
{
	
}


void BerxelImageRender::glutMouseCallBack(int button, int state, int x, int y)
{
	//if(state != GLUT_DOWN)
	//{
	//	return;
	//}

	//left button down mark the Cursor position
	if (GLUT_LEFT_BUTTON == button)
	{
		m_CursorPos.x = x * m_pImageRender->m_nTexMapX/m_pImageRender->m_nWindowRealWidth;
		m_CursorPos.y = y * m_pImageRender->m_nTexMapY/m_pImageRender->m_nWindowHeight;
	}

	//right button clear the Cusor position
	if(GLUT_RIGHT_BUTTON == button)
	{
		m_CursorPos.x = 0;
		m_CursorPos.y = 0;
	}


	if(m_pImageRender->m_pMouseCallBack != NULL)
	{
		m_pImageRender->m_pMouseCallBack(button, state, x, y);
	}
}

void BerxelImageRender::onKey(unsigned char key, int32_t x, int32_t y)
{
	switch (key)
	{
    case 'Q':
    case 'q':
           glutLeaveMainLoop();
           break;
	}

	if (m_pKeyboardCB) {
		m_pKeyboardCB(key);
	}

}

void BerxelImageRender::glutWindowReshapeCallBack(int width, int height)
{
	m_pImageRender->m_nWindowRealWidth = width;
	m_pImageRender->m_nWindowHeight = height;
	glViewport(0,0,width,height);
}
