#include "SceneTreeItem.h"

#include <QAction>
#include <QMenu>
#include "GUIHelpers.h"
#include "files.h"

namespace
{
	bool PickedAction ( QAction *result, QAction *compare )
	{

		if ( compare == NULL )
		{
			return false;
		}

		return result == compare;
	}

	void OnRightClickAnimSequence ( ModelHandle_t model, const Sequence_t *sequence, int sequenceIndex )
	{
		QMenu menu;
		QAction *lockSequence = NULL;
		QAction *addToMultiLock = NULL;
		QAction *removeFromMultiLock = NULL;
		QAction *lockSecondarySequence = NULL;
		QAction *addToSecondaryMultiLock = NULL;
		QAction *removeFromSecondaryMultiLock = NULL;

		bool isPrimaryLocked = Model_Sequence_IsLocked (model, sequenceIndex, true);
		bool isSecondaryLocked = Model_Sequence_IsLocked (model, sequenceIndex, false);
		bool isInPrimarySequence = Model_MultiSeq_AlreadyContains (model, sequenceIndex, true);
		bool isInSecondarySequence = Model_MultiSeq_AlreadyContains (model, sequenceIndex, false);

		lockSequence = menu.addAction (QObject::tr ("Lock Sequence"));
		addToMultiLock = menu.addAction (QObject::tr ("Add to Multi-Lock Sequences"));
		removeFromMultiLock = menu.addAction (QObject::tr ("Remove from Multi-Lock Sequences"));

		menu.addSeparator();

		lockSecondarySequence = menu.addAction (QObject::tr ("Lock as Secondary Sequence"));
		addToSecondaryMultiLock = menu.addAction (QObject::tr ("Add to Secondary Multi-Lock Sequences"));
		removeFromSecondaryMultiLock = menu.addAction (QObject::tr ("Remove from Secondary Multi-Lock Sequences"));

		lockSequence->setCheckable (true);
		lockSequence->setChecked (isPrimaryLocked);
		removeFromMultiLock->setDisabled (!isInPrimarySequence);

		if ( !Model_MultiSeq_IsActive (model, true) )
		{
			lockSecondarySequence->setDisabled (true);
			addToSecondaryMultiLock->setDisabled (true);
			removeFromSecondaryMultiLock->setDisabled (true);
		}
		else
		{
			lockSecondarySequence->setCheckable (true);
			lockSecondarySequence->setChecked (isSecondaryLocked);
			removeFromSecondaryMultiLock->setDisabled (!isInSecondarySequence);
		}

		QAction *result = menu.exec(QCursor::pos());
		if ( PickedAction (result, lockSequence) )
		{
			if ( isPrimaryLocked )
			{
				Model_Sequence_UnLock (model, true);
			}
			else
			{
				Model_Sequence_Lock (model, sequenceIndex, true);
			}
		}
		else if ( PickedAction (result, addToMultiLock))
		{
			Model_MultiSeq_Add (model, sequenceIndex, true);
		}
		else if ( PickedAction (result, removeFromMultiLock) )
		{
			Model_MultiSeq_Delete (model, sequenceIndex, true);
		}
		else if ( PickedAction (result, lockSecondarySequence) )
		{
			if ( isSecondaryLocked )
			{
				Model_Sequence_UnLock (model, false);
			}
			else
			{
				Model_Sequence_Lock (model, sequenceIndex, false);
			}
		}
		else if ( PickedAction (result, addToSecondaryMultiLock) )
		{
			Model_MultiSeq_Add (model, sequenceIndex, false);
		}
		else if ( PickedAction (result, removeFromSecondaryMultiLock) )
		{
			Model_MultiSeq_Delete (model, sequenceIndex, false);
		}
	}

	void OnRightClickSkinName ( ModelHandle_t model, const char *skinName, int skinIndex )
	{
		QMenu menu;
		QAction *useAction = menu.addAction (QObject::tr ("Use"));
		QAction *validateAction = menu.addAction (QObject::tr ("Validate"));

		QAction *action = menu.exec(QCursor::pos());
		if ( PickedAction (action, useAction) )
		{
			Model_ApplyOldSkin (model, skinName);
		}
		else if ( PickedAction (action, validateAction) )
		{
			Model_ValidateSkin (model, skinIndex);
		}
	}

	void OnRightClickBone ( ModelHandle_t model, const mdxaSkel_t *bone, int boneIndex )
	{
		QMenu menu;
		QAction *viewDetails = menu.addAction (QObject::tr ("Details"));
		QAction *boltModel = menu.addAction (QObject::tr ("Attach Model"));
		QAction *removeModel = menu.addAction (QObject::tr ("Detach All Models"));

		bool isSecondaryAnimRoot = Model_GetSecondaryAnimStart (model) == boneIndex;

		menu.addSeparator();

		QAction *setRoot = menu.addAction (QObject::tr ("Set as Secondary Animation Root"));

		setRoot->setDisabled (boneIndex == 0);
		removeModel->setDisabled (Model_CountItemsBoltedHere (model, boneIndex, true) == 0);

		setRoot->setCheckable (true);
		setRoot->setChecked (isSecondaryAnimRoot);

		QAction *result = menu.exec(QCursor::pos());
		if ( PickedAction (result, viewDetails) )
		{
			InfoBox (Model_GLMSurfaceInfo (model, boneIndex, true));
		}
		else if ( PickedAction (result, boltModel) )
		{
			const char *directory = Filename_PathOnly(Model_GetFullPrimaryFilename());
			std::string modelName (OpenGLMDialog (NULL, directory));

			if ( modelName.empty() )
			{
				return;
			}

			Model_LoadBoltOn (modelName.c_str(), model, boneIndex, true, false);
		}
		else if ( PickedAction (result, removeModel) )
		{
			Model_DeleteBoltOn (model, boneIndex, true, -1);
		}
		else if ( PickedAction (result, setRoot) )
		{
			if ( isSecondaryAnimRoot )
			{
				Model_SetSecondaryAnimStart (model, -1);
			}
			else
			{
				Model_SetSecondaryAnimStart (model, boneIndex);
			}
		}
	}

	void OnRightClickSurface ( ModelHandle_t model, const mdxmSurfHierarchy_t *surface, int surfaceIndex )
	{
		if ( surface->flags & G2SURFACEFLAG_ISBOLT )
		{
			// Tag
			QMenu menu;
			QAction *viewDetailsAction;
			QAction *boltModelAction;
			QAction *unboltModelAction;

			viewDetailsAction = menu.addAction (QObject::tr ("Details"));
			boltModelAction = menu.addAction (QObject::tr ("Attach Model"));
			unboltModelAction = menu.addAction (QObject::tr ("Detach All Models"));

			unboltModelAction->setDisabled (Model_CountItemsBoltedHere (model, surfaceIndex, false) == 0);

			QAction *resultAction = menu.exec(QCursor::pos());

			if ( PickedAction (resultAction, viewDetailsAction) )
			{
				InfoBox (Model_GLMSurfaceInfo (model, surfaceIndex, true));
			}
			else if ( PickedAction (resultAction, boltModelAction) )
			{
				const char *directory = Filename_PathOnly(Model_GetFullPrimaryFilename());
				std::string modelName (OpenGLMDialog (NULL, directory));

				if ( modelName.empty() )
				{
					return;
				}

				Model_LoadBoltOn (modelName.c_str(), model, surfaceIndex, false, false);
			}
			else if ( PickedAction (resultAction, unboltModelAction) )
			{
				Model_DeleteBoltOn (model, surfaceIndex, false, -1);
			}
		}
		else
		{
			// Surface
			QMenu menu;
			QAction *viewDetailsAction;
			QAction *setRootSurface;
			QAction *showSurfaceAction;
			QAction *hideSurfaceAction;
			QAction *hideSurfaceAndChildAction;
			SurfaceOnOff_t surfaceVisibility;
			bool isRoot;

			viewDetailsAction = menu.addAction (QObject::tr ("Details"));
			setRootSurface = menu.addAction (QObject::tr ("Set as Root Surface"));
			menu.addSeparator();
			showSurfaceAction = menu.addAction (QObject::tr ("Show"));
			hideSurfaceAction = menu.addAction (QObject::tr ("Hide"));
			hideSurfaceAndChildAction = menu.addAction (QObject::tr ("Hide (+ Descendants)"));

			isRoot = Model_GetG2SurfaceRootOverride (model) == surfaceIndex;
			setRootSurface->setCheckable (true);
			setRootSurface->setChecked (isRoot);

			surfaceVisibility = Model_GLMSurface_GetStatus (model, surfaceIndex);

			showSurfaceAction->setDisabled (surfaceVisibility == SURF_ON);
			hideSurfaceAction->setDisabled (surfaceVisibility == SURF_OFF);
			hideSurfaceAndChildAction->setDisabled (surfaceVisibility == SURF_NO_DESCENDANTS);

			QAction *resultAction = menu.exec(QCursor::pos());
			if ( PickedAction (resultAction, viewDetailsAction) )
			{
				InfoBox (Model_GLMSurfaceInfo (model, surfaceIndex, true));
			}
			else if ( PickedAction (resultAction, setRootSurface) )
			{
				if ( isRoot )
				{
					Model_SetG2SurfaceRootOverride (model, -1);
				}
				else
				{
					Model_SetG2SurfaceRootOverride (model, surfaceIndex);
				}
			}
			else if ( PickedAction (resultAction, showSurfaceAction) )
			{
				Model_GLMSurface_SetStatus (model, surfaceIndex, SURF_ON);
			}
			else if ( PickedAction (resultAction, hideSurfaceAction) )
			{
				Model_GLMSurface_SetStatus (model, surfaceIndex, SURF_OFF);
			}
			else if ( PickedAction (resultAction, hideSurfaceAndChildAction) )
			{
				Model_GLMSurface_SetStatus (model, surfaceIndex, SURF_NO_DESCENDANTS);
			}
		}
	}
	
	void OnRightClickAnimSequenceRoot ( ModelHandle_t model )
	{

	}

	void OnRightClickSkinNameRoot ( ModelHandle_t model )
	{
		QMenu menu;
		QAction *validateAllAction = menu.addAction (QObject::tr ("Validate All"));

		QAction *action = menu.exec(QCursor::pos());
		if ( PickedAction (action, validateAllAction) )
		{
			Model_ValidateSkin (model, -1);
		}
	}

	void OnRightClickBoneRoot ( ModelHandle_t model )
	{
	}

	void OnRightClickSurfaceRoot ( ModelHandle_t model )
	{
	}

	void OnRightClickTagRoot ( ModelHandle_t model )
	{
	}
}

SceneTreeItem::SceneTreeItem ( ModelResourceType resourceType, const void *resource, int resourceIndex, const QString& data, ModelHandle_t model, SceneTreeItem *parent )
    : data (data)
    , parent (parent)
    , model (model)
	, resourceType(resourceType)
	, resourceIndex(resourceIndex)
{
	switch ( resourceType )
	{
	case MODELRESOURCE_SKIN:
		{
			if ( resource != NULL )
			{
				const char *skinName = static_cast<const char*>(resource);
				size_t skinNameLength = strlen(skinName);
				char *skinNameDup = new char[skinNameLength + 1];
				strcpy(skinNameDup, skinName);

				this->resource.skinName = skinNameDup;

				break;
			}

			// fall through
		}

	default:
		this->resource.ptr = resource;
		break;
	}
}

SceneTreeItem::~SceneTreeItem()
{
    for ( int i = 0; i < children.size(); i++ )
    {
        delete children[i];
    }

	switch ( resourceType )
	{
	case MODELRESOURCE_SKIN:
		delete [] this->resource.skinName;
		break;

	default:
		break;
	}
}

ModelHandle_t SceneTreeItem::GetModel() const
{
    return model;
}

void SceneTreeItem::AddChild ( SceneTreeItem *child )
{
    children.append (child);
}

SceneTreeItem *SceneTreeItem::Child ( int row ) const
{
    return children.value (row);
}

int SceneTreeItem::ChildCount() const
{
    return children.size();
}

int SceneTreeItem::ChildCountRecursive() const
{
    int count = 0;
    for ( int i = 0; i < ChildCount(); i++ )
    {
        count += Child(i)->ChildCountRecursive() + 1;
    }

    return count;
}

QVariant SceneTreeItem::Data() const
{
    return data;
}

void SceneTreeItem::Data ( const QString& data )
{
    this->data = data;
}

int SceneTreeItem::Row() const
{
    if ( parent != NULL )
    {
        return parent->children.indexOf (const_cast<SceneTreeItem *>(this));
    }

    return 0;
}

SceneTreeItem *SceneTreeItem::Parent() const
{
    return parent;
}

void SceneTreeItem::LeftClick()
{
	switch ( resourceType )
	{
	case MODELRESOURCE_SURFACE:
		if ( resourceIndex == -1 )
		{
			Model_SetSurfaceHighlight( model, iITEMHIGHLIGHT_ALL );
		}
		else
		{
			Model_SetSurfaceHighlight( model, resourceIndex );
		}
		break;

	case MODELRESOURCE_TAG:
		if ( resourceIndex == -1 )
		{
			Model_SetSurfaceHighlight( model, iITEMHIGHLIGHT_ALL_TAGSURFACES );
		}
		else
		{
			Model_SetSurfaceHighlight( model, resourceIndex );
		}
		break;

	case MODELRESOURCE_BONE:
		if ( resourceIndex == -1 )
		{
			Model_SetBoneHighlight( model, iITEMHIGHLIGHT_ALL );
		}
		else
		{
			Model_SetBoneHighlight( model, resourceIndex );
		}
		break;

	default:
		break;
	}
}

void SceneTreeItem::RightClick()
{
	if ( resourceIndex == -1 )
	{
		switch ( resourceType )
		{
		case MODELRESOURCE_ANIMSEQUENCE:
			OnRightClickAnimSequenceRoot( model );
			break;

		case MODELRESOURCE_SKIN:
			OnRightClickSkinNameRoot( model );
			break;

		case MODELRESOURCE_BONE:
			OnRightClickBoneRoot( model );
			break;

		case MODELRESOURCE_TAG:
			OnRightClickTagRoot( model );
			break;

		case MODELRESOURCE_SURFACE:
			OnRightClickSurfaceRoot( model );
			break;

		default:
			break;
		}

		return;
	}

	switch ( resourceType )
	{
	case MODELRESOURCE_ANIMSEQUENCE:
		OnRightClickAnimSequence( model, resource.sequence, resourceIndex );
		break;

	case MODELRESOURCE_SKIN:
		OnRightClickSkinName( model, resource.skinName, resourceIndex );
		break;

	case MODELRESOURCE_BONE:
		OnRightClickBone( model, resource.bone, resourceIndex );
		break;

	case MODELRESOURCE_TAG:
	case MODELRESOURCE_SURFACE:
		OnRightClickSurface( model, resource.surface, resourceIndex );
		break;

	default:
		break;
	}
}

void SceneTreeItem::DoubleClick()
{
	if ( resourceIndex == -1 )
	{
		return;
	}

	switch ( resourceType )
	{
	case MODELRESOURCE_ANIMSEQUENCE:
		{
			if (Model_MultiSeq_IsActive (model, true))
			{
				Model_MultiSeq_Add (model, resourceIndex, true);
			}
			else
			{
				Model_Sequence_Lock (model, resourceIndex, true);
				ModelList_Rewind();
			}
		}
		break;

	case MODELRESOURCE_SKIN:
		Model_ApplyOldSkin (model, resource.skinName);
		break;

	default:
		break;
	}
}

bool SceneTreeItem::IsOff() const
{
	if ( resourceIndex == -1 )
	{
		return false;
	}

	switch ( resourceType )
	{
	case MODELRESOURCE_SURFACE:
		{
			SurfaceOnOff_t surfaceStatus = GLMModel_Surface_GetStatus( model, resourceIndex );
			return surfaceStatus == SURF_OFF || surfaceStatus == SURF_NO_DESCENDANTS;
		}

	default:
		return false;
	}
}
