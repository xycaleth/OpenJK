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

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const char *skinName, int skinIndex )
{
    Model_ApplyOldSkin (model, skinName);
}

void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex ) { }
void SceneTreeItemDblClickAction::Visit ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex ) { }


//==============================================================================
// Right click actions
//==============================================================================

bool PickedAction ( QAction *result, QAction *compare )
{
    if ( compare == NULL )
    {
        return false;
    }

    return result == compare;
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex )
{
    QMenu menu;
    QAction *lockSequence = NULL;
    QAction *unlockSequence = NULL;
    QAction *startMultiLock = NULL;
    QAction *addToMultiLock = NULL;
    QAction *removeFromMultiLock = NULL;
    QAction *lockSecondarySequence = NULL;
    QAction *unlockSecondarySequence = NULL;
    QAction *startSecondaryMultilock = NULL;
    QAction *addToSecondaryMultiLock = NULL;
    QAction *removeFromSecondaryMultiLock = NULL;

    lockSequence = menu.addAction ("Lock Sequence");
    unlockSequence = menu.addAction ("Unlock Sequence");
    addToMultiLock = menu.addAction ("Add to Multi-Lock Sequences");
    startMultiLock = menu.addAction ("Start Multi-Lock with this Sequence");
    removeFromMultiLock = menu.addAction ("Remove from Multi-Lock Sequences");

    menu.addSeparator();

    lockSecondarySequence = menu.addAction ("Lock as Secondary Sequence");
    unlockSecondarySequence = menu.addAction ("Unlock Secondary Sequence");
    addToMultiLock = menu.addAction ("Add to Secondary Multi-Lock Sequences");
    startMultiLock = menu.addAction ("Start Secondary Multi-Lock with this Sequence");
    menu.addAction ("Remove from Secondary Multi-Lock Sequences");

    QAction *result = menu.exec(QCursor::pos());
    if ( PickedAction (result, lockSequence) )
    {
        Model_Sequence_Lock (model, sequenceIndex, true);
    }
    else if ( PickedAction (result, unlockSequence) )
    {
        Model_Sequence_UnLock (model, true);
    }
    else if ( PickedAction (result, startMultiLock) ||
            PickedAction (result, addToMultiLock))
    {
        Model_MultiSeq_Add (model, sequenceIndex, true);
    }
    else if ( PickedAction (result, removeFromMultiLock) )
    {
        Model_MultiSeq_Delete (model, sequenceIndex, true);
    }
    else if ( PickedAction (result, lockSecondarySequence) )
    {
        Model_Sequence_Lock (model, sequenceIndex, false);
    }
    else if ( PickedAction (result, unlockSecondarySequence) )
    {
        Model_Sequence_UnLock (model, false);
    }
    else if ( PickedAction (result, startSecondaryMultilock) ||
            PickedAction (result, addToSecondaryMultiLock) )
    {
        Model_MultiSeq_Add (model, sequenceIndex, false);
    }
    else if ( PickedAction (result, removeFromSecondaryMultiLock) )
    {
        Model_MultiSeq_Delete (model, sequenceIndex, false);
    }
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const char *skinName, int skinIndex )
{
    QMenu menu;
    QAction *validateAction = menu.addAction ("Validate");

    QAction *action = menu.exec(QCursor::pos());
    if ( PickedAction (action, validateAction) )
    {
        Model_ValidateSkin (model, skinIndex);
    }
}

void SceneTreeItemRightClickAction::Visit ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
{
    if ( surface->flags & G2SURFACEFLAG_ISBOLT )
    {
        // Tag
        QMenu menu;
        QAction *viewDetailsAction = menu.addAction ("View Details");
        QAction *boltModelAction = menu.addAction ("Bolt Model to this Tag");
        QAction *unboltModelAction = 0;

        QAction *resultAction = menu.exec(QCursor::pos());

        if ( resultAction == viewDetailsAction )
        {
            InfoBox (Model_GLMSurfaceInfo (model, surfaceIndex, true));
        }
        else if ( boltModelAction != NULL && resultAction == boltModelAction )
        {
            
        }
        else
        {
            OutputDebugString ("WHAT\n");
        }
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
    QAction *viewDetails = menu.addAction ("View Details");
    QAction *boltModel = menu.addAction ("Bolt Model to this Bone");
    QAction *removeModel = menu.addAction ("Remove Models from this Bone");
    QAction *setRoot = menu.addAction ("Set as Secondary Animation Root");
    if ( boneIndex == 0 )
    {
        setRoot->setDisabled (true);
    }

    QAction *result = menu.exec(QCursor::pos());
    if ( PickedAction (result, viewDetails) )
    {
        InfoBox (Model_GLMSurfaceInfo (model, boneIndex, true));
    }
    else if ( PickedAction (result, boltModel) )
    {
        
    }
    else if ( PickedAction (result, removeModel) )
    {
        
    }
    else if ( PickedAction (result, setRoot) )
    {
        
    }
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
void SceneTreeItemClickAction::Visit ( ModelHandle_t model, const char *skinName, int skinIndex ) { }