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
#include "textures.h"

void StartRenderTimer ( QWidget *parent, RenderWidget *renderWidget )
{
    QTimer *fpsTimer = new QTimer (parent);
    fpsTimer->setInterval (10);
    fpsTimer->setTimerType (Qt::PreciseTimer);
    QObject::connect (fpsTimer, SIGNAL (timeout()), parent, SLOT (OnUpdateAnimation()));
    fpsTimer->start();
}

void SetupSceneTreeModel ( const QString& modelName, ModelContainer_t& container, SceneTreeModel& model )
{
    SceneTreeItem *root = new SceneTreeItem ("");
    SceneTreeItem *modelItem = new SceneTreeItem (QString ("==> %1 <==").arg (QString::fromLatin1 (Filename_WithoutPath (modelName.toLatin1()))), root);

    root->AddChild (modelItem);

    SceneTreeItem *surfacesItem = new SceneTreeItem (QObject::tr ("Surfaces"), modelItem);
    SceneTreeItem *tagsItem = new SceneTreeItem (QObject::tr ("Tags"), modelItem);
    SceneTreeItem *bonesItem = new SceneTreeItem (QObject::tr ("Bones"), modelItem);
    modelItem->AddChild (surfacesItem);
    modelItem->AddChild (tagsItem);
    modelItem->AddChild (bonesItem);

    model.setRoot (root);
    #if 0
    TreeItemData_t	TreeItemData={0};
					TreeItemData.iModelHandle = hModel;
									
	TreeItemData.iItemType	= TREEITEMTYPE_MODELNAME;
	pContainer->hTreeItem_ModelName = ModelTree_InsertItem(va("==>  %s  <==",Filename_WithoutPath(/*Filename_WithoutExt*/(psLocalFilename))), hTreeItem_Parent, TreeItemData.uiData);

	TreeItemData.iItemType	= TREEITEMTYPE_SURFACEHEADER;
HTREEITEM hTreeItem_Surfaces		= ModelTree_InsertItem("Surfaces",	pContainer->hTreeItem_ModelName, TreeItemData.uiData);

	TreeItemData.iItemType	= TREEITEMTYPE_TAGSURFACEHEADER;
HTREEITEM hTreeItem_TagSurfaces	= ModelTree_InsertItem("Tag Surfaces",	pContainer->hTreeItem_ModelName, TreeItemData.uiData);

	TreeItemData.iItemType	= TREEITEMTYPE_BONEHEADER;
	hTreeItem_Bones			= ModelTree_InsertItem("Bones",		pContainer->hTreeItem_ModelName, TreeItemData.uiData);

	// send surface heirarchy to tree...
	//
	mdxmHierarchyOffsets_t *pHierarchyOffsets = (mdxmHierarchyOffsets_t *) ((byte *) pMDXMHeader + sizeof(*pMDXMHeader));

	R_GLM_AddSurfaceToTree( hModel, hTreeItem_Surfaces, 0, pHierarchyOffsets, false);
	R_GLM_AddSurfaceToTree( hModel, hTreeItem_TagSurfaces, 0, pHierarchyOffsets, true);

	// special error check for badly-hierarchied surfaces... (bad test data inadvertently supplied by Rob Gee :-)
	//
	int iNumSurfacesInTree = ModelTree_GetChildCount(hTreeItem_Surfaces);
	if (iNumSurfacesInTree != pMDXMHeader->numSurfaces)
	{
		ErrorBox(va("Model has %d surfaces, but only %d of them are connected up through the heirarchy, the rest will never be recursed into.\n\nThis model needs rebuilding, guys...",pMDXMHeader->numSurfaces,iNumSurfacesInTree));
		bReturn = false;
	}

	if (!ModelTree_ItemHasChildren( hTreeItem_TagSurfaces ))
	{
		ModelTree_DeleteItem( hTreeItem_TagSurfaces );
	}

	// send bone heirarchy to tree...
	//
	mdxaSkelOffsets_t *pSkelOffsets = (mdxaSkelOffsets_t *) ((byte *)pMDXAHeader + sizeof(*pMDXAHeader));

	R_GLM_AddBoneToTree( hModel, hTreeItem_Bones, 0, pSkelOffsets);
    #endif
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