#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtOpenGL/QGLWidget>

class RenderWidget : public QGLWidget
{
    Q_OBJECT

public:
    RenderWidget ( QWidget *parent = 0 );

protected:
    void paintGL();

};

#endif