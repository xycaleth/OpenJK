#include "stdafx.h"
#include "SceneTreeView.h"

#include <QtCore/QStack>
#include <algorithm>
#include <vector>
#include "glm_code.h"
#include "r_model.h"
#include "SceneTreeItem.h"
#include "SceneTreeModel.h"
#include "sequence.h"

struct BoneTreeApplication
{
    ModelContainer_t *container;
    mdxaSkelOffsets_t *pSkeletonOffsets;
    QStack<SceneTreeItem *> nodes;
};

void BeforeBoneChildrenAdded ( mdxaSkel_t *bone, void *userData )
{
    BoneTreeApplication *app = static_cast<BoneTreeApplication *>(userData);

    SceneTreeItem *item = new SceneTreeItem (QString::fromLatin1 (bone->name), app->nodes.back());
    app->nodes.back()->AddChild (item);
    app->nodes.append (item);
}

void AfterBoneChildrenAdded ( mdxaSkel_t *, void *userData )
{
    BoneTreeApplication *app = static_cast<BoneTreeApplication *>(userData);

    app->nodes.pop();
}

void BoneChildrenAdded ( mdxaSkel_t *bone, int boneIndex, void *userData )
{
    BoneTreeApplication *app = static_cast<BoneTreeApplication *>(userData);

    R_GLM_BoneRecursiveApply (
        app->container->hModel,
        boneIndex,
        app->pSkeletonOffsets,
        BeforeBoneChildrenAdded,
        BoneChildrenAdded,
        AfterBoneChildrenAdded,
        static_cast<void *>(app));
}

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

struct CompareSequencesByFrame
{
    const SequenceList_t& sequences;

    CompareSequencesByFrame ( const SequenceList_t& sequences )
        : sequences (sequences)
    {
    }

    bool operator () ( int left, int right )
    {
        return sequences[left].iStartFrame < sequences[right].iStartFrame;
    }
};

void AddSequencesToTree ( SceneTreeItem *root, const SequenceList_t& sequenceList )
{
    std::vector<int> sequenceIndices (sequenceList.size(), -1);
    for ( int i = 0; i < sequenceList.size(); i++ )
    {
        sequenceIndices[i] = i;
    }

    std::sort (sequenceIndices.begin(), sequenceIndices.end(), CompareSequencesByFrame (sequenceList));

    for ( int i = 0; i < sequenceList.size(); i++ )
    {
        root->AddChild (new SceneTreeItem (Sequence_CreateTreeName (&sequenceList[sequenceIndices[i]]), root));
    }
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

    SurfaceTreeApplication surfaceApp;
    surfaceApp.container = &container;
    surfaceApp.pHierarchyOffsets = pHierarchyOffsets;

    // Add surfaces
    surfaceApp.nodes.append (surfacesItem);
    R_GLM_SurfaceRecursiveApply (
        container.hModel,
        0,
        pHierarchyOffsets,
        BeforeSurfaceChildrenAdded,
        SurfaceChildrenAdded,
        AfterSurfaceChildrenAdded,
        static_cast<void *>(&surfaceApp));

    int numSurfacesInTree = surfacesItem->ChildCountRecursive();
    if ( numSurfacesInTree != pMDXMHeader->numSurfaces )
    {
        ErrorBox (va ("Model has %d surfaces, but only %d of them are connected through the heirarchy, the rest will never be recursed into.\n\n"
                        "This model needs rebuilding.",
                        pMDXMHeader->numSurfaces, numSurfacesInTree));
    }

    // Add tags
    surfaceApp.nodes.clear();
    surfaceApp.nodes.append (tagsItem);
    R_GLM_SurfaceRecursiveApply (
        container.hModel,
        0,
        pHierarchyOffsets,
        BeforeTagChildrenAdded,
        TagChildrenAdded,
        NULL,
        static_cast<void *>(&surfaceApp));

    // Add bones
    mdxaSkelOffsets_t *pSkelOffsets = (mdxaSkelOffsets_t *)((byte *)pMDXAHeader + sizeof(*pMDXAHeader));
    BoneTreeApplication boneApp;
    boneApp.container = &container;
    boneApp.nodes.append (bonesItem);
    boneApp.pSkeletonOffsets = pSkelOffsets;

    R_GLM_BoneRecursiveApply (
        container.hModel,
        0,
        pSkelOffsets,
        BeforeBoneChildrenAdded,
        BoneChildrenAdded,
        AfterBoneChildrenAdded,
        static_cast<void *>(&boneApp));

    // Add animation sequences
    SceneTreeItem *sequencesItem = NULL;
    if ( !container.SequenceList.empty() )
    {
        sequencesItem = new SceneTreeItem (QObject::tr ("Sequences"), modelItem);
        AddSequencesToTree (sequencesItem, container.SequenceList);
    }

    // And add the items to model!
    modelItem->AddChild (surfacesItem);

    if ( tagsItem->ChildCount() > 0 )
    {
        modelItem->AddChild (tagsItem);
    }

    modelItem->AddChild (bonesItem);

    if ( sequencesItem != NULL )
    {
        modelItem->AddChild (sequencesItem);
    }

    model.setRoot (root);
}