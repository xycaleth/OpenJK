#include "stdafx.h"
#include "MainForm.h"
#include <QtWidgets/QApplication>

int main ( int argc, char *argv[] )
{
    QApplication app (argc, argv);

    MainForm w;
    w.show();

    return app.exec();
}