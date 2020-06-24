#ifndef RENDERWIDGET_H
#define RENDERWIDGET_H

#include <QtOpenGL/QGLWidget>
#include <string>

class RenderWidget : public QGLWidget
{
    Q_OBJECT

  public:
    RenderWidget(QWidget* parent = 0);

  signals:
    void tookScreenshotForClipboard(const QImage& image);

  protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

  private:
    void SaveScreenshot(const std::string& filename, int width, int height);

  private:
    int lastX, lastY;
    bool panning;
};

extern int g_iScreenWidth, g_iScreenHeight;

#endif
