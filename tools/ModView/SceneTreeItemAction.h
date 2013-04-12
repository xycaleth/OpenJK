#ifndef SCENETREEITEMACTION_H
#define SCENETREEITEMACTION_H

#include "ISceneTreeItemVisitor.h"

#include "model.h"

class SceneTreeItemAction : public ISceneTreeItemVisitor
{
public:
    void Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex );
    void Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex );
};

#endif