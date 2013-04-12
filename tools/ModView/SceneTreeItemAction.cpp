#include "stdafx.h"
#include "SceneTreeItemAction.h"

#include "model.h"

//==============================================================================
// Double click actions
//==============================================================================

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex ) { }

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex )
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

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const char *skinName )
{
    Model_ApplyOldSkin (model, skinName);
}


//==============================================================================
// Single click actions
//==============================================================================

void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
{
    Model_SetSurfaceHighlight (model, surfaceIndex);
}

void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex ) { }
void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const char *skinName ) { }