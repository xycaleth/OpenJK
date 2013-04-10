#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/QMainWindow>
#include "ui_MainForm.h"

#include <string>

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
    std::string currentSceneName;

    Ui::MainWindow ui;
};

#endif