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

    int numSurfacesInTree = surfacesItem->ChildCountRecursive();
    if ( numSurfacesInTree != pMDXMHeader->numSurfaces )
    {
        ErrorBox (va ("Model has %d surfaces, but only %d of them are connected through the heirarchy, the rest will never be recursed into.\n\n"
                        "This model needs rebuilding.",
                        pMDXMHeader->numSurfaces, numSurfacesInTree));
    }

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

    // And add the items to model!
    modelItem->AddChild (surfacesItem);

    if ( tagsItem->ChildCount() > 0 )
    {
        modelItem->AddChild (tagsItem);
    }

    modelItem->AddChild (bonesItem);

    model.setRoot (root);
    #if 0
	// send bone heirarchy to tree...
	//
	mdxaSkelOffsets_t *pSkelOffsets = (mdxaSkelOffsets_t *) ((byte *)pMDXAHeader + sizeof(*pMDXAHeader));

	R_GLM_AddBoneToTree( hModel, hTreeItem_Bones, 0, pSkelOffsets);
    #endif
}