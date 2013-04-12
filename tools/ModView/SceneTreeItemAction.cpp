#include "stdafx.h"
#include "SceneTreeItemAction.h"

#include "model.h"

void SceneTreeItemAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
{
    OutputDebugString ("Clicked a surface!\n");
}

void SceneTreeItemAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex )
{
    // multiseqlock or single lock?...
    //
    if (Model_MultiSeq_IsActive (model, true))
    {
        Model_MultiSeq_Add (model, sequenceIndex, true);
    }
    else
    {
        Model_Sequence_Lock (model, sequenceIndex, true);
        ModelList_Rewind();
    }
}
