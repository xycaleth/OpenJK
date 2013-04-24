#include "stdafx.h"
#include "RenderWidget.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/gl.h>
#else
#include <gl/GL.h>
#endif
#include "model.h"
#include "text.h"
#include "textures.h"

const float MOUSE_ROT_SCALE = 0.5f;
const float MOUSE_XPOS_SCALE = 0.1f;
const float MOUSE_YPOS_SCALE = 0.1f;
const float MOUSE_ZPOS_SCALE = 0.4f;

int g_iScreenHeight;
int g_iScreenWidth;

RenderWidget::RenderWidget ( QWidget *parent )
    : QGLWidget (parent)
    , lastX (0)
    , lastY (0)
    , panning (false)
{
}

void RenderWidget::initializeGL()
{
    GL_CacheDriverInfo();
    OnceOnly_GLVarsInit();
}

void RenderWidget::resizeGL ( int width, int height )
{
	g_iScreenWidth = width;
	g_iScreenHeight = height;
}

void RenderWidget::keyPressEvent ( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Alt )
    {
        panning = true;
    }
}

void RenderWidget::keyReleaseEvent ( QKeyEvent *event )
{
    if ( event->key() == Qt::Key_Alt )
    {
        panning = false;
    }
}

void RenderWidget::mousePressEvent ( QMouseEvent *event )
{
    lastX = event->x();
    lastY = event->y();
}

void RenderWidget::mouseMoveEvent ( QMouseEvent *event )
{
    Qt::MouseButtons buttonsDown = event->buttons();
    int x = event->x();
    int y = event->y();

    if ( buttonsDown & Qt::LeftButton )
    {
        if ( !panning )
        {
            AppVars.rotAngleY += (float)(x - lastX) * MOUSE_ROT_SCALE;
            AppVars.rotAngleX += (float)(y - lastY) * MOUSE_ROT_SCALE;

            if ( AppVars.rotAngleY > 360.0f )
            {
                AppVars.rotAngleY = AppVars.rotAngleY - 360.0f;
            }
            else if ( AppVars.rotAngleY < -360.0f )
            {
                AppVars.rotAngleY = AppVars.rotAngleY + 360.0f;
            }

            if ( AppVars.rotAngleX > 360.0f )
            {
                AppVars.rotAngleX = AppVars.rotAngleX - 360.0f;
            }
            else if ( AppVars.rotAngleX < -360.0f )
            {
                AppVars.rotAngleX = AppVars.rotAngleX + 360.0f;
            }
        }
        else
        {
            AppVars.xPos += ((float)(x - lastX) / 10.0f) * MOUSE_XPOS_SCALE;
			AppVars.yPos -= ((float)(y - lastY) / 10.0f) * MOUSE_YPOS_SCALE;
        }

        ModelList_ForceRedraw();
    }

    if ( buttonsDown & Qt::RightButton )
    {
        AppVars.zPos += ((float)(y - lastY) / 10.0f) * MOUSE_ZPOS_SCALE;
        AppVars.zPos = min (max (AppVars.zPos, -1000.0f), 1000.0f);

        ModelList_ForceRedraw();
    }

    lastX = x;
    lastY = y;
}

void SaveScreenshot ( const std::string& filename, int width, int height )
{
    int oldPackAlignment = 0;
    
    glGetIntegerv (GL_PACK_ALIGNMENT, &oldPackAlignment);
    glPixelStorei (GL_PACK_ALIGNMENT, 1);
    
    unsigned char *buffer = new unsigned char[width * height * 3];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    
    QImage image (buffer, width, height, width * 3, QImage::Format_RGB888);
    image.mirrored().save (QString::fromStdString (filename));
    
    delete [] buffer;
    glPixelStorei (GL_PACK_ALIGNMENT, oldPackAlignment);
}

void RenderWidget::paintGL()
{
    QSize frameSize (size());
    if ( AppVars.takeScreenshot )
    {
        AppVars.takeScreenshot = false;
        
        gbTextInhibit = true;
        ModelList_Render (frameSize.width(), frameSize.height());
        gbTextInhibit = false;
        
        SaveScreenshot (AppVars.screenshotFileName, frameSize.width(), frameSize.height());
    }
    
    ModelList_Render (frameSize.width(), frameSize.height());
}
