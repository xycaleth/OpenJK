#include "stdafx.h"
#include "SceneTreeModel.h"

SceneTreeModel::SceneTreeModel ( QObject *parent )
    : QAbstractItemModel (parent)
{
}

QVariant SceneTreeModel::data ( const QModelIndex& index, int role ) const
{
    return QVariant();
}

Qt::ItemFlags SceneTreeModel::flags ( const QModelIndex& index ) const
{
    return 0;
}

QVariant SceneTreeModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    return QVariant();
}

QModelIndex SceneTreeModel::index ( int row, int column, const QModelIndex& parent ) const
{
    return QModelIndex();
}

QModelIndex SceneTreeModel::parent ( const QModelIndex& index ) const
{
    return QModelIndex();
}

int SceneTreeModel::rowCount ( const QModelIndex& parent ) const
{
    return 0;
}

int SceneTreeModel::columnCount ( const QModelIndex& parent ) const
{
    return 0;
}