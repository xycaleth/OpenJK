#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/QMainWindow>
#include "ui_MainForm.h"

class SceneTreeModel;
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm ( QWidget *parent = 0 );

    void CurrentSceneName ( const QString& sceneName );

private slots:
    void OnOpenModel();
    void OnChooseBackgroundColor();

    void OnChangeBackgroundColor ( const QColor& color );

    void OnUpdateAnimation();
    void OnAbout();

    void OnAnimationStart();
    void OnAnimationPause();
    void OnAnimationStartWithLooping();
    void OnAnimationRewind();
    void OnAnimationGoToEndFrame();
    void OnAnimationSpeedUp();
    void OnAnimationSlowDown();
    void OnToggleInterpolation();
    void OnNextFrame();
    void OnPrevFrame();

    void OnScreenshotToFile();
    void OnScreenshotToClipboard();
    void OnToggleCleanScreenshot();
    void OnResetViewPoint();
    void OnCycleFieldOfView();
    void OnIncreaseLOD();
    void OnDecreaseLOD();
    void OnPicmipTo0();
    void OnPicmipTo1();
    void OnPicmipTo2();
    void OnPicmipTo3();
    void OnShowOpenGLInfo();

private:
    void ChangeLOD ( int lod );
    void ChangePicmip ( int level );

private:
    Ui::MainWindow ui;

    QString currentSceneName;
    SceneTreeModel *treeModel;
};

#endif