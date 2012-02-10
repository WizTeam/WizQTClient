#ifndef WIZCATEGORYMODEL_H
#define WIZCATEGORYMODEL_H

#include <QAbstractItemModel>

#include "share/wizdatabase.h"

#include "wizcategoryinfo.h"


class CWizCategoryModelPrivate;


class CWizCategoryModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool resolveSymlinks READ resolveSymlinks WRITE setResolveSymlinks)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)
    Q_PROPERTY(bool lazyChildCount READ lazyChildCount WRITE setLazyChildCount)

public:
    enum Roles {
        FileIconRole = Qt::DecorationRole,
        FilePathRole = Qt::UserRole + 1,
        FileNameRole
    };

    enum SortFlag { Name        = 0x00,
                    Time        = 0x01,
                    Size        = 0x02,
                    Unsorted    = 0x03,
                    SortByMask  = 0x03,

                    DirsFirst   = 0x04,
                    Reversed    = 0x08,
                    IgnoreCase  = 0x10,
                    DirsLast    = 0x20,
                    LocaleAware = 0x40,
                    Type        = 0x80,
                    NoSort = -1
    };

    Q_DECLARE_FLAGS(SortFlags, SortFlag)


    explicit CWizCategoryModel(QObject *parent = 0);
    ~CWizCategoryModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool hasChildren(const QModelIndex &index = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    Qt::DropActions supportedDropActions() const;

    // CWizCategoryModel specific API

    //void setIconProvider(QFileIconProvider *provider);
    //QFileIconProvider *iconProvider() const;

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;

    void setSorting(SortFlags sort);
    SortFlags sorting() const;

    void setResolveSymlinks(bool enable);
    bool resolveSymlinks() const;

    void setReadOnly(bool enable);
    bool isReadOnly() const;

    void setLazyChildCount(bool enable);
    bool lazyChildCount() const;

    QModelIndex index(const QString &path, int column = 0) const;

    bool isLocation(const QModelIndex &index) const;
    QModelIndex mkdir(const QModelIndex &parent, const QString &name);
    bool rmdir(const QModelIndex &index);
    bool remove(const QModelIndex &index);

    QString filePath(const QModelIndex &index) const;
    QString fileName(const QModelIndex &index) const;
    QIcon fileIcon(const QModelIndex &index) const;

#ifdef Q_NO_USING_KEYWORD
    inline QObject *parent() const { return QObject::parent(); }
#else
    using QObject::parent;
#endif

public Q_SLOTS:
    void refresh(const QModelIndex &parent = QModelIndex());

protected:
    CWizCategoryModel(CWizCategoryModelPrivate &, QObject *parent = 0);
    friend class QFileDialogPrivate;

private:
    Q_DECLARE_PRIVATE(CWizCategoryModel)
    Q_DISABLE_COPY(CWizCategoryModel)
    //Q_PRIVATE_SLOT(d_func(), void _q_refresh())
};




#endif // WIZCATEGORYMODEL_H
