#include "stdafx.h"
#include "SceneTreeItemAction.h"

#include <QtWidgets/QMenu>
#include "model.h"

//==============================================================================
// Double click actions
//==============================================================================

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

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex ) { }
void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex ) { }


//==============================================================================
// Right click actions
//==============================================================================

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex )
{
    QMenu menu;
    menu.addAction ("Lock Sequence");
    menu.addAction ("Start Multi-Locking with this Sequence");
    menu.addSeparator();
    menu.addAction ("Lock as Secondary Sequence");
    menu.addAction ("Start Secondary Multi-Locking with this Sequence");

    menu.exec(QCursor::pos());
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const char *skinName )
{
    QMenu menu;
    menu.addAction ("Validate");

    menu.exec(QCursor::pos());
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
{
    if ( surface->flags & G2SURFACEFLAG_ISBOLT )
    {
        // Tag
        QMenu menu;
        menu.addAction ("View Details");
        menu.addAction ("Bolt Model to this Tag");

        menu.exec(QCursor::pos());
    }
    else
    {
        // Surface
        QMenu menu;
        menu.addAction ("View Details");
        menu.addAction ("Set as Root Surface");
        menu.addSeparator();
        menu.addAction ("Hide");
        menu.addAction ("Hide (+ Descendants)");

        menu.exec(QCursor::pos());
    }
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex )
{
    QMenu menu;
    menu.addAction ("View Details");
    menu.addAction ("Bolt Model to this Bone");
    if ( boneIndex > 0 )
    {
        menu.addAction ("Set as Secondary Animation Root");
    }

    menu.exec(QCursor::pos());
}


//==============================================================================
// Single click actions
//==============================================================================

void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
{   
    Model_SetSurfaceHighlight (model, surfaceIndex);
}

void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex )
{
    Model_SetBoneHighlight (model, boneIndex);
}

void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex ) { }
void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const char *skinName ) { }