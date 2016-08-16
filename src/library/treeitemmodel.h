#ifndef TREE_ITEM_MODEL_H
#define TREE_ITEM_MODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QUrl>

class TreeItem;
class LibraryFeature;

class TreeItemModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    
    enum Role {
        RoleDataPath = Qt::UserRole,
        RoleBold,
        RoleDivider,
        RoleQuery,
        RoleBreadCrumb,
        RoleSettings
    };

    TreeItemModel(QObject* parent = nullptr);
    virtual ~TreeItemModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &index) const;
    // Tell the compiler we don't mean to shadow insertRows.
    using QAbstractItemModel::insertRows;
    virtual bool insertRows(QList<TreeItem*> &data, int position, int rows, const QModelIndex &parent = QModelIndex());
    virtual bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    virtual bool setData(const QModelIndex &a_rIndex, const QVariant &a_rValue,
                         int a_iRole = Qt::EditRole);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    void setRootItem(TreeItem *item);
    // Return the underlying TreeItem.
    // If the index is invalid, the root item is returned.
    TreeItem* getItem(const QModelIndex &index) const;
    
    bool dropAccept(const QModelIndex& index, QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);
    
    static QString getBreadCrumbString(TreeItem* pTree);
    static QSize getDefaultIconSize();
    
  public slots:
    virtual void reloadTree();
    void triggerRepaint();

  protected:
    LibraryFeature* getFeatureFromIndex(const QModelIndex& index) const;
    
    TreeItem* m_pRootItem;
};

#endif
