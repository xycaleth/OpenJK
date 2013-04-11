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

private:
    Ui::MainWindow ui;

    QString currentSceneName;
    SceneTreeModel *treeModel;
};

#endif