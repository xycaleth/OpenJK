#include "stdafx.h"
#include "SceneTreeView.h"

#include <QtCore/QStack>
#include "glm_code.h"
#include "r_model.h"
#include "SceneTreeItem.h"
#include "SceneTreeModel.h"

struct SurfaceTreeApplication
{
    ModelContainer_t *container;
    mdxmHierarchyOffsets_t *pHierarchyOffsets;
    QStack<SceneTreeItem *> nodes;
};

void BeforeSurfaceChildrenAdded ( mdxmSurfHierarchy_t *surface, void *userData )
{
    SurfaceTreeApplication *app = static_cast<SurfaceTreeApplication *>(userData);

    SceneTreeItem *item = new SceneTreeItem (QString::fromLatin1 (surface->name), app->nodes.back());
    app->nodes.back()->AddChild (item);
    app->nodes.append (item);
}

void AfterSurfaceChildrenAdded ( mdxmSurfHierarchy_t *surface, void *userData )
{
    SurfaceTreeApplication *app = static_cast<SurfaceTreeApplication *>(userData);

    app->nodes.pop();
}

void SurfaceChildrenAdded ( mdxmSurfHierarchy_t *surface, int surfaceIndex, void *userData )
{
    SurfaceTreeApplication *app = static_cast<SurfaceTreeApplication *>(userData);

    R_GLM_SurfaceRecursiveApply (
        app->container->hModel,
        surfaceIndex,
        app->pHierarchyOffsets,
        BeforeSurfaceChildrenAdded,
        SurfaceChildrenAdded,
        AfterSurfaceChildrenAdded,
        static_cast<void *>(app));
}

void BeforeTagChildrenAdded ( mdxmSurfHierarchy_t *surface, void *userData )
{
    SurfaceTreeApplication *app = static_cast<SurfaceTreeApplication *>(userData);

    if ( surface->flags & G2SURFACEFLAG_ISBOLT )
    {
        app->nodes.back()->AddChild (new SceneTreeItem (QString::fromLatin1 (surface->name), app->nodes.back()));
    }
}

void TagChildrenAdded ( mdxmSurfHierarchy_t *surface, int surfaceIndex, void *userData )
{
    SurfaceTreeApplication *app = static_cast<SurfaceTreeApplication *>(userData);

    R_GLM_SurfaceRecursiveApply (
        app->container->hModel,
        surfaceIndex,
        app->pHierarchyOffsets,
        BeforeTagChildrenAdded,
        TagChildrenAdded,
        NULL,
        static_cast<void *>(app));
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

    mdxmHeader_t *pMDXMHeader = (mdxmHeader_t *) RE_GetModelData (container.hModel);
	mdxaHeader_t *pMDXAHeader = (mdxaHeader_t *) RE_GetModelData(pMDXMHeader->animIndex);
    mdxmHierarchyOffsets_t *pHierarchyOffsets = (mdxmHierarchyOffsets_t *) ((byte *) pMDXMHeader + sizeof(*pMDXMHeader));

    SurfaceTreeApplication app;
    app.container = &container;
    app.pHierarchyOffsets = pHierarchyOffsets;

    // Add surfaces
    app.nodes.append (surfacesItem);
    R_GLM_SurfaceRecursiveApply (
        container.hModel,
        0,
        pHierarchyOffsets,
        BeforeSurfaceChildrenAdded,
        SurfaceChildrenAdded,
        AfterSurfaceChildrenAdded,
        static_cast<void *>(&app));

    // Add tags
    app.nodes.clear();
    app.nodes.append (tagsItem);
    R_GLM_SurfaceRecursiveApply (
        container.hModel,
        0,
        pHierarchyOffsets,
        BeforeTagChildrenAdded,
        TagChildrenAdded,
        NULL,
        static_cast<void *>(&app));

    model.setRoot (root);
    #if 0
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