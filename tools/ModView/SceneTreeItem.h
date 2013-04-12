#ifndef SCENETREEITEM_H
#define SCENETREEITEM_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>

#include "model.h"
#include "sequence.h"

class ISceneTreeItemVisitor;
class SceneTreeItem
{
public:
    SceneTreeItem ( const QString& data, ModelHandle_t model, SceneTreeItem *parent = 0 );
    virtual ~SceneTreeItem();

    // This SceneTreeItem object takes ownership of 'child'.
    void            AddChild ( SceneTreeItem *child );
    SceneTreeItem  *Child ( int row ) const;
    int             ChildCount() const;
    int             ChildCountRecursive() const;

    QVariant        Data() const;
    void            Data ( const QString& data );
    int             Row() const;
    SceneTreeItem  *Parent() const;

    // Not really sure if it's right to call this
    // the visitor pattern...
    virtual void    Accept ( ISceneTreeItemVisitor *visitor ) {};

protected:
    ModelHandle_t   GetModel() const;

private:
    QList<SceneTreeItem *> children;
    QString data;
    SceneTreeItem *parent;

    ModelHandle_t model;
};

class SequenceSceneTreeItem : public SceneTreeItem
{
public:
    SequenceSceneTreeItem ( const Sequence_t *sequence, int sequenceIndex, ModelHandle_t model, SceneTreeItem *parent = 0 );

    void Accept ( ISceneTreeItemVisitor *visitor );

private:
    const Sequence_t *sequence;
    int sequenceIndex;
};

class SurfaceSceneTreeItem : public SceneTreeItem
{
public:
    SurfaceSceneTreeItem ( const mdxmSurfHierarchy_t *surface, int surfaceIndex, ModelHandle_t model, SceneTreeItem *parent = 0 );

    void Accept ( ISceneTreeItemVisitor *visitor );

private:
    const mdxmSurfHierarchy_t *surface;
    int surfaceIndex;
};

#endif