#include "stdafx.h"
#include "MainForm.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QTimer>
#include "generic_stuff.h"
#include "model.h"

MainForm::MainForm ( QWidget *parent )
    : QMainWindow (parent)
{
    ui.setupUi (this);
    CurrentSceneName ("Untitled");

    QTimer *fpsTimer = new QTimer (this);
    fpsTimer->setInterval (10);
    fpsTimer->setTimerType (Qt::PreciseTimer);
    connect (fpsTimer, SIGNAL (timeout()), ui.renderWidget, SLOT (updateGL()));
    fpsTimer->start();
}

MainForm::~MainForm()
{
    
}

void MainForm::OnOpenModel()
{
    const char *directory = Filename_PathOnly(Model_GetFullPrimaryFilename());

    QFileDialog openDialog (this);
    openDialog.setDirectory (QString::fromLatin1 (directory));
    openDialog.setFileMode (QFileDialog::ExistingFile);
    openDialog.setNameFilter (tr ("Model files (*.glm)"));
    openDialog.setAcceptMode (QFileDialog::AcceptOpen);

    if ( openDialog.exec() )
    {
        QStringList modelName = openDialog.selectedFiles();
        if ( modelName.isEmpty() )
        {
            return;
        }

        if ( Model_LoadPrimary (modelName[0].toLatin1()) )
        {
            currentSceneName = modelName[0].toStdString();
        }
    }
}

void MainForm::CurrentSceneName ( const std::string& sceneName )
{
    currentSceneName = sceneName;
    //setWindowTitle (tr ("%1 - ModView").arg (QString::fromStdString (sceneName)));
}