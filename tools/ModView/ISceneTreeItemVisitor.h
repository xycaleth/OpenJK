#ifndef ISCENETREEITEMVISITOR_H
#define ISCENETREEITEMVISITOR_H

#include "model.h"
#include "sequence.h"

class ISceneTreeItemVisitor
{
public:
    virtual ~ISceneTreeItemVisitor() { }

    virtual void Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex ) = 0;
    virtual void Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex ) = 0;
    virtual void Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex ) = 0;
    virtual void Visit ( ModelHandle_t model, const char *skinName ) = 0;
};

#endif