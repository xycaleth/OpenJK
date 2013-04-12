#include "stdafx.h"
#include "SceneTreeModel.h"

#include "ISceneTreeItemVisitor.h"
#include "SceneTreeItem.h"

//=============================================================================
// Scene Tree Item class implementation
//=============================================================================

SceneTreeItem::SceneTreeItem ( const QString& data, ModelHandle_t model, SceneTreeItem *parent )
    : data (data)
    , parent (parent)
    , model (model)
{
}

SceneTreeItem::~SceneTreeItem()
{
    for ( int i = 0; i < children.size(); i++ )
    {
        delete children[i];
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


SequenceSceneTreeItem::SequenceSceneTreeItem ( const Sequence_t *sequence, int sequenceIndex, ModelHandle_t model, SceneTreeItem *parent )
    : SceneTreeItem (Sequence_CreateTreeName (sequence), model, parent)
    , sequence (sequence)
    , sequenceIndex (sequenceIndex)
{
}

void SequenceSceneTreeItem::Accept ( ISceneTreeItemVisitor *visitor )
{
    visitor->Visit (GetModel(), sequence, sequenceIndex);
}

SurfaceSceneTreeItem::SurfaceSceneTreeItem ( const mdxmSurfHierarchy_t *surface, int surfaceIndex, ModelHandle_t model, SceneTreeItem *parent )
    : SceneTreeItem (surface->name, model, parent)
    , surface (surface)
    , surfaceIndex (surfaceIndex)
{
}

void SurfaceSceneTreeItem::Accept ( ISceneTreeItemVisitor *visitor )
{
    visitor->Visit (GetModel(), surface, surfaceIndex);
}

SkinSceneTreeItem::SkinSceneTreeItem ( const std::string& skinName, ModelHandle_t model, SceneTreeItem *parent )
    : SceneTreeItem (QString::fromStdString (skinName), model, parent)
    , skinName (skinName)
{
}

void SkinSceneTreeItem::Accept ( ISceneTreeItemVisitor *visitor )
{
    visitor->Visit (GetModel(), skinName.c_str());
}

//=============================================================================
// Scene Tree Model class implementation
//=============================================================================

SceneTreeModel::SceneTreeModel ( QObject *parent )
    : QAbstractItemModel (parent)
    , root (NULL)
{
}

SceneTreeModel::~SceneTreeModel()
{
    delete root;
}

QVariant SceneTreeModel::data ( const QModelIndex& index, int role ) const
{
    if ( !index.isValid() )
    {
        return QVariant();
    }

    if ( role != Qt::DisplayRole )
    {
        return QVariant();
    }

    SceneTreeItem *item = static_cast<SceneTreeItem *>(index.internalPointer());

    return item->Data();
}

Qt::ItemFlags SceneTreeModel::flags ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return 0;
    }

    return QAbstractItemModel::flags (index);
}

QVariant SceneTreeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

QModelIndex SceneTreeModel::index ( int row, int column, const QModelIndex& parent ) const
{
    if ( !hasIndex (row, column, parent) )
    {
        return QModelIndex();
    }

    SceneTreeItem *parentNode = root;
    if ( parent.isValid() )
    {
        parentNode = static_cast<SceneTreeItem *>(parent.internalPointer());
    }

    if ( parentNode != NULL )
    {
        SceneTreeItem *childNode = parentNode->Child (row);
        if ( childNode != NULL )
        {
            return createIndex (row, column, childNode);
        }
    }

    return QModelIndex();
}

QModelIndex SceneTreeModel::parent ( const QModelIndex& index ) const
{
    if ( !index.isValid() )
    {
        return QModelIndex();
    }

    SceneTreeItem *childNode = static_cast<SceneTreeItem *>(index.internalPointer());
    SceneTreeItem *parentNode = childNode->Parent();

    if ( parentNode != NULL && parentNode != root )
    {
        return createIndex (parentNode->Row(), 0, parentNode);
    }

    return QModelIndex();
}

int SceneTreeModel::rowCount ( const QModelIndex& parent ) const
{
    if ( parent.column() > 0 )
    {
        return 0;
    }

    SceneTreeItem *parentNode = root;
    if ( parent.isValid() )
    {
        parentNode = static_cast<SceneTreeItem *>(parent.internalPointer());
    }

    if ( parentNode == NULL )
    {
        return 0;
    }

    return parentNode->ChildCount();
}

int SceneTreeModel::columnCount ( const QModelIndex& parent ) const
{
    return 1;
}

void SceneTreeModel::setRoot ( SceneTreeItem *root )
{
    if ( this->root != NULL )
    {
        beginRemoveRows (QModelIndex(), 0, 0);
            delete this->root;
            this->root = NULL;
        endRemoveRows();
    }

    beginInsertRows (QModelIndex(), 0, 0);
        this->root = root;
    endInsertRows();
}