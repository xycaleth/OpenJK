#ifndef SCENETREEMODEL_H
#define SCENETREEMODEL_H

#include <QtCore/QAbstractItemModel>

class SceneTreeItem;
class SceneTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    SceneTreeModel ( QObject *parent = 0 );
    ~SceneTreeModel();

    // Implementation for pure virtual functions.
    QVariant        data ( const QModelIndex& index, int role ) const;
    Qt::ItemFlags   flags ( const QModelIndex& index ) const;
    QVariant        headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex     index ( int row, int column, const QModelIndex& parent = QModelIndex() ) const;
    QModelIndex     parent ( const QModelIndex& index ) const;
    int             rowCount ( const QModelIndex& parent = QModelIndex() ) const;
    int             columnCount ( const QModelIndex& parent = QModelIndex() ) const;

    // Model mutator functions.

    // SceneTreeModel takes ownership of 'root'.
    void            setRoot ( SceneTreeItem *root );

private:
    SceneTreeItem *root;
};

#endif