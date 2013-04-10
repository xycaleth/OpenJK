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
    ~MainForm();

    void CurrentSceneName ( const std::string& sceneName );

private slots:
    void OnOpenModel();

private:
    std::string currentSceneName;

    Ui::MainWindow ui;
};

#endif