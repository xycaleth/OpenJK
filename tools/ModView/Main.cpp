#include "stdafx.h"

#include "MainForm.h"
#include <QtWidgets/QApplication>

#include "model.h"
#include "textures.h"

int main ( int argc, char *argv[] )
{
    QApplication app (argc, argv);

    App_OnceOnly();
    FakeCvars_OnceOnlyInit();

    MainForm w;
    w.show();

    int errorCode = app.exec();

    Media_Delete();
    FakeCvars_Shutdown();

    return errorCode;
}