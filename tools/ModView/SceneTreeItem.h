#ifndef SCENETREEITEM_H
#define SCENETREEITEM_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>

class SceneTreeItem
{
public:
    SceneTreeItem ( const QString& data, SceneTreeItem *parent = 0 );
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

private:
    QList<SceneTreeItem *> children;
    QString data;
    SceneTreeItem *parent;
};

#endif