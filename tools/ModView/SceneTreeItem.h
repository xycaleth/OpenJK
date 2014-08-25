#ifndef SCENETREEITEM_H
#define SCENETREEITEM_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include <string>
#include "model.h"
#include "sequence.h"

enum ModelResourceType
{
	MODELRESOURCE_NULL, // root of the tree view
	MODELRESOURCE_ANIMSEQUENCE,
	MODELRESOURCE_SURFACE,
	MODELRESOURCE_TAG,
	MODELRESOURCE_SKIN,
	MODELRESOURCE_BONE
};

class ISceneTreeItemVisitor;
class SceneTreeItem
{
public:
    SceneTreeItem (
		ModelResourceType resourceType, const void *resource, int resourceIndex,
		const QString& data, ModelHandle_t model, SceneTreeItem *parent = 0 );
    ~SceneTreeItem();

    // This SceneTreeItem object takes ownership of 'child'.
    void            AddChild ( SceneTreeItem *child );
    SceneTreeItem  *Child ( int row ) const;
    int             ChildCount() const;
    int             ChildCountRecursive() const;

    QVariant        Data() const;
    void            Data ( const QString& data );
    int             Row() const;
    SceneTreeItem  *Parent() const;

	void			LeftClick();
	void			RightClick();
	void			DoubleClick();

	bool			IsOff() const;

    ModelHandle_t   GetModel() const;

private:
    QList<SceneTreeItem *> children;
    QString data;
    SceneTreeItem *parent;

    ModelHandle_t model;

	ModelResourceType resourceType;
	int resourceIndex;
	union
	{
		const void *ptr;

		const Sequence_t *sequence;
		const mdxmSurfHierarchy_t *surface;
		const char *skinName;
		const mdxaSkel_t *bone;
	} resource;
};

#endif