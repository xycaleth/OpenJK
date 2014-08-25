#include "stdafx.h"
#include "SceneTreeModel.h"

#include <QBrush>
#include "r_model.h"
#include "SceneTreeItem.h"

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

	switch ( role )
	{
		case Qt::DisplayRole:
		{
			return static_cast<SceneTreeItem *>(index.internalPointer())->Data();
		}

		case Qt::ForegroundRole:
		{
			SceneTreeItem *item = static_cast<SceneTreeItem *>(index.internalPointer());
			return QVariant(QBrush(item->IsOff() ? Qt::gray : Qt::black));
		}

		default:
		{
			return QVariant();
		}
	}
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

void SceneTreeModel::clear()
{
	if ( this->root != NULL )
    {
        beginRemoveRows (QModelIndex(), 0, 0);
            delete this->root;
            this->root = NULL;
        endRemoveRows();
    }
}

void SceneTreeModel::setRoot ( SceneTreeItem *root )
{
    clear();

    beginInsertRows (QModelIndex(), 0, 0);
        this->root = root;
    endInsertRows();
}