#ifndef SCENETREEITEMACTION_H
#define SCENETREEITEMACTION_H

#include "ISceneTreeItemVisitor.h"

#include "model.h"

class SceneTreeItemDblClickAction : public ISceneTreeItemVisitor
{
public:
    void Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex );
    void Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex );
    void Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex );
    void Visit ( ModelHandle_t model, const char *skinName, int skinIndex );
};

class SceneTreeItemClickAction : public ISceneTreeItemVisitor
{
public:
    void Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex );
    void Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex );
    void Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex );
    void Visit ( ModelHandle_t model, const char *skinName, int skinIndex );
};

class SceneTreeItemRightClickAction : public ISceneTreeItemVisitor
{
public:
    void Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex );
    void Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex );
    void Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex );
    void Visit ( ModelHandle_t model, const char *skinName, int skinIndex );
};

#endif