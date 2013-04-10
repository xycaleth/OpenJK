#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtOpenGL/QGLWidget>

class RenderWidget : public QGLWidget
{
    Q_OBJECT

public:
    RenderWidget ( QWidget *parent = 0 );

protected:
    void initializeGL();
    void paintGL();
    void resizeGL ( int width, int height );

    void keyPressEvent ( QKeyEvent *event );
    void keyReleaseEvent ( QKeyEvent *event );

    void mousePressEvent ( QMouseEvent *event );
    void mouseMoveEvent ( QMouseEvent *event );

private:
    int lastX, lastY;
    bool panning;
};

#endif