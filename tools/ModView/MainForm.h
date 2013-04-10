#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/QMainWindow>
#include <string>
#include "ui_MainForm.h"

class SceneTreeModel;
class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm ( QWidget *parent = 0 );

    void CurrentSceneName ( const std::string& sceneName );

private slots:
    void OnOpenModel();
    void OnChooseBackgroundColor();

    void OnChangeBackgroundColor ( const QColor& color );

    void OnAbout();

private:
    Ui::MainWindow ui;

    std::string currentSceneName;
    SceneTreeModel *treeModel;
};

#endif