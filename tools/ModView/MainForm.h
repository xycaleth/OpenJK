#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/QMainWindow>
#include "ui_MainForm.h"

class MainForm : public QMainWindow
{
    Q_OBJECT

public:
    MainForm ( QWidget *parent = 0 );
    ~MainForm();

private:
    Ui::MainWindow ui;
};

#endif