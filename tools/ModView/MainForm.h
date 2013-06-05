#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtCore/QSettings>
#include <QtWidgets/QMainWindow>
#include "ui_MainForm.h"

#include "SceneTreeItemAction.h"

class SceneTreeModel;
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm ( QSettings& settings, QWidget *parent = 0 );

    void CurrentSceneName ( const QString& sceneName );

private slots:
    void OnOpenModel();
    void OnOpenRecentModel();
    void OnClearRecentFiles();
    void OnChooseBackgroundColor();

    void OnChangeBackgroundColor ( const QColor& color );
    void OnRefreshTextures();

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

    void OnDoubleClickedTreeView ( const QModelIndex& index );
    void OnClickedTreeView ( const QModelIndex& index );
    void OnRightClickTreeView ( const QPoint& point );

private:
    void ChangeLOD ( int lod );
    void ChangePicmip ( int level );

    void PopulateMRUFiles ( const QStringList& mruFiles );
    void OpenModel ( const QString& modelPath );

private:
    Ui::MainWindow ui;

    QSettings *settings;
    SceneTreeModel *treeModel;

    SceneTreeItemClickAction treeItemClickAction;
    SceneTreeItemDblClickAction treeItemDblClickAction;
    SceneTreeItemRightClickAction treeItemRightClickAction;
};

#endif