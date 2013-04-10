#include "stdafx.h"

#include "RenderWidget.h"
#include "model.h"

RenderWidget::RenderWidget ( QWidget *parent )
    : QGLWidget (parent)
{
}

void RenderWidget::paintGL()
{
    ModelList_Render (size().width(), size().height());
}