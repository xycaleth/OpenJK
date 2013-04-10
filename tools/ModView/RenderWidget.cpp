#include "stdafx.h"
#include "RenderWidget.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <gl/GL.h>
#include "model.h"
#include "textures.h"

const float MOUSE_ROT_SCALE = 0.5f;
const float MOUSE_XPOS_SCALE = 0.1f;
const float MOUSE_YPOS_SCALE = 0.1f;
const float MOUSE_ZPOS_SCALE = 0.4f;

RenderWidget::RenderWidget ( QWidget *parent )
    : QGLWidget (parent)
    , lastX (0)
    , lastY (0)
    , panning (false)
{
}

void RenderWidget::initializeGL()
{
    OnceOnly_GLVarsInit();
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
    }

    if ( buttonsDown & Qt::RightButton )
    {
        AppVars.zPos += ((float)(y - lastY) / 10.0f) * MOUSE_ZPOS_SCALE;
        AppVars.zPos = min (max (AppVars.zPos, -1000.0f), 1000.0f);
    }

    lastX = x;
    lastY = y;
}

void RenderWidget::paintGL()
{
    ModelList_Animation();
    ModelList_Render (size().width(), size().height());
}