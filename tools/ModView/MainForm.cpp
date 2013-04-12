#include "stdafx.h"
#include "MainForm.h"

#include <QtWidgets/QColorDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTimer>
#include "generic_stuff.h"
#include "model.h"
#include "SceneTreeItem.h"
#include "SceneTreeModel.h"
#include "SceneTreeView.h"
#include "textures.h"

void StartRenderTimer ( QWidget *parent, RenderWidget *renderWidget )
{
    QTimer *fpsTimer = new QTimer (parent);
    fpsTimer->setInterval (10);
    fpsTimer->setTimerType (Qt::PreciseTimer);
    QObject::connect (fpsTimer, SIGNAL (timeout()), parent, SLOT (OnUpdateAnimation()));
    fpsTimer->start();
}


MainForm::MainForm ( QWidget *parent )
    : QMainWindow (parent)
    , treeModel (new SceneTreeModel (this))
{
    ui.setupUi (this);

    ui.treeView->setModel (treeModel);
    CurrentSceneName (tr ("Untitled"));

    StartRenderTimer (this, ui.renderWidget);
}

void MainForm::OnUpdateAnimation()
{
    if ( ModelList_Animation() )
    {
        ui.renderWidget->updateGL();
    }
}

void MainForm::OnAbout()
{
    QMessageBox::about (this,
        tr ("About ModView"),
        tr ("<p><b>ModView 3.0</b><br />"
            "Written by Alex 'Xycaleth' Lo.</p>"
            "<p><b>ModView 2.5</b><br />"
            "Written by Ste Cork and Mike Crowns.</p>"
            "<p>Copyright (c) 2000 - 2013, Raven Software.<br />"
            "Released under GNU General Public License, version 2.0.</p>"
            "<p>Current formats supported: Ghoul 2 (.glm, .gla)</p>"));
}

void MainForm::OnChangeBackgroundColor ( const QColor& color )
{
    AppVars._R = color.red();
    AppVars._G = color.green();
    AppVars._B = color.blue();
}

void MainForm::OnChooseBackgroundColor()
{
    QColorDialog *dialog = new QColorDialog (this);
    QColor color (AppVars._R, AppVars._G, AppVars._B);

    dialog->setCurrentColor (color);
    dialog->open (this, SLOT (OnChangeBackgroundColor (const QColor&)));
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
            CurrentSceneName (modelName[0]);
            SetupSceneTreeModel (modelName[0], AppVars.Container, *treeModel);
        }
    }
}

void MainForm::CurrentSceneName ( const QString& sceneName )
{
    currentSceneName = sceneName;
    setWindowTitle (tr ("%1 - ModView").arg (currentSceneName));
}

void MainForm::OnResetViewPoint()
{
    AppVars_ResetViewParams();
    ModelList_ForceRedraw();
}

void MainForm::OnAnimationStart()
{
    Model_StartAnim();
}

void MainForm::OnAnimationPause()
{
    Model_StopAnim();
}

void MainForm::OnAnimationStartWithLooping()
{
    Model_StartAnim (true);
}

void MainForm::OnAnimationRewind()
{
    ModelList_Rewind();
    ModelList_ForceRedraw();
}

void MainForm::OnAnimationGoToEndFrame()
{
    ModelList_GoToEndFrame();
    ModelList_ForceRedraw();
}

void MainForm::OnAnimationSpeedUp()
{
    AppVars.dAnimSpeed *= ANIM_FASTER;

    // Need to update the FPS text
    ModelList_ForceRedraw();
}

void MainForm::OnAnimationSlowDown()
{
    AppVars.dAnimSpeed *= ANIM_SLOWER;

    // Need to update the FPS text
    ModelList_ForceRedraw();
}

void MainForm::OnToggleInterpolation()
{
    AppVars.bInterpolate = !AppVars.bInterpolate;
    ModelList_ForceRedraw();
}

void MainForm::OnNextFrame()
{
    ModelList_StepFrame (1);
}

void MainForm::OnPrevFrame()
{
    ModelList_StepFrame (-1);
}

void MainForm::OnScreenshotToFile()
{
    
}

void MainForm::OnScreenshotToClipboard()
{
}

void MainForm::OnToggleCleanScreenshot()
{
}

void MainForm::OnCycleFieldOfView()
{
    const float VALID_FOVS[] = { 10.0f, 80.0f, 90.0f };
    const std::size_t NUM_FOVS = sizeof (VALID_FOVS) / sizeof (VALID_FOVS[0]);
    for ( int i = 0; i < NUM_FOVS; i++ )
    {
        if ( AppVars.dFOV == VALID_FOVS[i] )
        {
            AppVars.dFOV = VALID_FOVS[(i + 1) % NUM_FOVS];
            break;
        }
    }

    ModelList_ForceRedraw();
}

void MainForm::OnIncreaseLOD()
{
    // Decrement, because lower LOD *number*,
    // means an *increased* level of detail in the model.
    ChangeLOD (AppVars.iLOD - 1);
}

void MainForm::OnDecreaseLOD()
{
    // Increment, because higher LOD *number*,
    // means a *decreased* level of detail in the model.
    ChangeLOD (AppVars.iLOD + 1);
}

void MainForm::OnPicmipTo0() { ChangePicmip (0); }
void MainForm::OnPicmipTo1() { ChangePicmip (1); }
void MainForm::OnPicmipTo2() { ChangePicmip (2); }
void MainForm::OnPicmipTo3() { ChangePicmip (3); }

void MainForm::ChangeLOD ( int lod )
{
    if ( lod < 0 )
    {
        lod = 0;
    }
    else if ( lod >= AppVars.Container.iNumLODs )
    {
        lod = AppVars.Container.iNumLODs - 1;
    }

    AppVars.iLOD = lod;
    ModelList_ForceRedraw();
}

void MainForm::ChangePicmip ( int level )
{
    TextureList_ReMip (level);
    ModelList_ForceRedraw();
}

void MainForm::OnShowOpenGLInfo()
{
    // FIXME: Message box is too narrow.
    QMessageBox::information (this, "OpenGL Driver Info", QString::fromLatin1 (GL_GetInfo()));
}

void MainForm::OnRefreshTextures()
{
    TextureList_Refresh();
    ModelList_ForceRedraw();
}

void MainForm::OnDoubleClickedTreeView ( const QModelIndex& index )
{
    if ( !index.isValid() )
    {
        return;
    }

    SceneTreeItem *item = static_cast<SceneTreeItem *>(index.internalPointer());
    item->Accept (&treeItemDblClickAction);
}

void MainForm::OnClickedTreeView ( const QModelIndex& index )
{
    if ( !index.isValid() )
    {
        return;
    }

    SceneTreeItem *item = static_cast<SceneTreeItem *>(index.internalPointer());
    item->Accept (&treeItemClickAction);
}