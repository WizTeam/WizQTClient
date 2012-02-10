#include "wizcategorymodel.h"
#include "wizcategoryinfo.h"

#include <QIcon>

#include <private/qabstractitemmodel_p.h>

#include <qdirmodel>


class CWizCategoryModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(CWizCategoryModel)

public:
    struct CWizCategoryNode
    {
        CWizCategoryNode() : parent(0), populated(false), stat(false) {}
        ~CWizCategoryNode() { children.clear(); }
        CWizCategoryNode *parent;
        QIcon icon; // cache the icon
        mutable QVector<CWizCategoryNode> children;
        mutable bool populated; // have we read the children
        mutable bool stat;
    };

    CWizCategoryModelPrivate()
        : resolveSymlinks(true),
          readOnly(true),
          lazyChildCount(false),
          allowAppendChild(true),
          //iconProvider(&defaultProvider),
          shouldStat(true) // ### This is set to false by QFileDialog
    { }

    void init();
    CWizCategoryNode *node(int row, CWizCategoryNode *parent) const;
    QVector<CWizCategoryNode> children(CWizCategoryNode *parent, bool stat) const;

    void _q_refresh();

    void savePersistentIndexes();
    void restorePersistentIndexes();

    CWizCategoryInfoList entryInfoList(const QString &path) const;
    QStringList entryList(const QString &path) const;

    QString name(const QModelIndex &index) const;
    QString size(const QModelIndex &index) const;
    QString type(const QModelIndex &index) const;
    QString time(const QModelIndex &index) const;

    void appendChild(CWizCategoryModelPrivate::CWizCategoryNode *parent, const QString &path) const;
    //static CWizCategoryInfo resolvedInfo(CWizCategoryInfo info);

    inline CWizCategoryNode *node(const QModelIndex &index) const;
    inline void populate(CWizCategoryNode *parent) const;
    inline void clear(CWizCategoryNode *parent) const;

    void invalidate();

    mutable CWizCategoryNode root;
    bool resolveSymlinks;
    bool readOnly;
    bool lazyChildCount;
    bool allowAppendChild;

    CWizCategoryInfo::SortFlags sort;
    QStringList nameFilters;

    //QFileIconProvider *iconProvider;
    //QFileIconProvider defaultProvider;

    struct SavedPersistent {
        QString path;
        int column;
        QPersistentModelIndexData *data;
        QPersistentModelIndex index;
    };
    QList<SavedPersistent> savedPersistent;
    QPersistentModelIndex toBeRefreshed;

    bool shouldStat; // use the "carefull not to stat directories" mode
};

void qt_setDirModelShouldNotStat(CWizCategoryModelPrivate *modelPrivate)
{
    modelPrivate->shouldStat = false;
}

CWizCategoryModelPrivate::CWizCategoryNode *CWizCategoryModelPrivate::node(const QModelIndex &index) const
{
    CWizCategoryModelPrivate::CWizCategoryNode *n =
        static_cast<CWizCategoryModelPrivate::CWizCategoryNode*>(index.internalPointer());
    Q_ASSERT(n);
    return n;
}

void CWizCategoryModelPrivate::populate(CWizCategoryNode *parent) const
{
    Q_ASSERT(parent);
    parent->children = children(parent, parent->stat);
    parent->populated = true;
}

void CWizCategoryModelPrivate::clear(CWizCategoryNode *parent) const
{
     Q_ASSERT(parent);
     parent->children.clear();
     parent->populated = false;
}

void CWizCategoryModelPrivate::invalidate()
{
    QStack<const CWizCategoryNode*> nodes;
    nodes.push(&root);
    while (!nodes.empty()) {
        const CWizCategoryNode *current = nodes.pop();
        current->stat = false;
        const QVector<CWizCategoryNode> children = current->children;
        for (int i = 0; i < children.count(); ++i)
            nodes.push(&children.at(i));
    }
}

/*!
    \class CWizCategoryModel
    \obsolete
    \brief The CWizCategoryModel class provides a data model for the local filesystem.

    \ingroup model-view

    The usage of CWizCategoryModel is not recommended anymore. The
    QFileSystemModel class is a more performant alternative.

    This class provides access to the local filesystem, providing functions
    for renaming and removing files and directories, and for creating new
    directories. In the simplest case, it can be used with a suitable display
    widget as part of a browser or filer.

    CWizCategoryModel keeps a cache with file information. The cache needs to be
    updated with refresh().

    A directory model that displays the contents of a default directory
    is usually constructed with a parent object:

    \snippet doc/src/snippets/shareddirmodel/main.cpp 2

    A tree view can be used to display the contents of the model

    \snippet doc/src/snippets/shareddirmodel/main.cpp 4

    and the contents of a particular directory can be displayed by
    setting the tree view's root index:

    \snippet doc/src/snippets/shareddirmodel/main.cpp 7

    The view's root index can be used to control how much of a
    hierarchical model is displayed. CWizCategoryModel provides a convenience
    function that returns a suitable model index for a path to a
    directory within the model.

    CWizCategoryModel can be accessed using the standard interface provided by
    QAbstractItemModel, but it also provides some convenience functions
    that are specific to a directory model. The categoryinfo() and isDir()
    functions provide information about the underlying files and directories
    related to items in the model.

    Directories can be created and removed using mkdir(), rmdir(), and the
    model will be automatically updated to take the changes into account.

    \note CWizCategoryModel requires an instance of a GUI application.

    \sa nameFilters(), setFilter(), filter(), QListView, QTreeView, QFileSystemModel,
    {Dir View Example}, {Model Classes}
*/

/*!
    Constructs a new directory model with the given \a parent.
    Only those files matching the \a nameFilters and the
    \a filters are included in the model. The sort order is given by the
    \a sort flags.
*/
/*!
  Constructs a directory model with the given \a parent.
*/

CWizCategoryModel::CWizCategoryModel(QObject *parent)
    : QAbstractItemModel(*new CWizCategoryModelPrivate, parent)
{
    Q_D(CWizCategoryModel);
    d->init();
}

/*!
    \internal
*/
CWizCategoryModel::CWizCategoryModel(CWizCategoryModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{
    Q_D(CWizCategoryModel);
    d->init();
}

/*!
  Destroys this directory model.
*/

CWizCategoryModel::~CWizCategoryModel()
{

}

/*!
  Returns the model item index for the item in the \a parent with the
  given \a row and \a column.

*/

QModelIndex CWizCategoryModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const CWizCategoryModel);
    // note that rowCount does lazy population
    if (column < 0 || column >= columnCount(parent) || row < 0 || parent.column() > 0)
        return QModelIndex();
    // make sure the list of children is up to date
    CWizCategoryModelPrivate::CWizCategoryNode *p = (d->indexValid(parent) ? d->node(parent) : &d->root);
    Q_ASSERT(p);
    if (!p->populated)
        d->populate(p); // populate without stat'ing
    if (row >= p->children.count())
        return QModelIndex();
    // now get the internal pointer for the index
    CWizCategoryModelPrivate::CWizCategoryNode *n = d->node(row, d->indexValid(parent) ? p : 0);
    Q_ASSERT(n);

    return createIndex(row, column, n);
}

/*!
  Return the parent of the given \a child model item.
*/

QModelIndex CWizCategoryModel::parent(const QModelIndex &child) const
{
    Q_D(const CWizCategoryModel);

    if (!d->indexValid(child))
    return QModelIndex();
    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(child);
    CWizCategoryModelPrivate::CWizCategoryNode *par = (node ? node->parent : 0);
    if (par == 0) // parent is the root node
    return QModelIndex();

    // get the parent's row
    const QVector<CWizCategoryModelPrivate::CWizCategoryNode> children =
        par->parent ? par->parent->children : d->root.children;
    Q_ASSERT(children.count() > 0);
    int row = (par - &(children.at(0)));
    Q_ASSERT(row >= 0);

    return createIndex(row, 0, par);
}

/*!
  Returns the number of rows in the \a parent model item.

*/

int CWizCategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.)

    Q_D(const CWizCategoryModel);
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid()) {
        if (!d->root.populated) // lazy population
            d->populate(&d->root);
        return d->root.children.count();
    }
    if (parent.model() != this)
        return 0;
    CWizCategoryModelPrivate::CWizCategoryNode *p = d->node(parent);
    if (p->info.isLocation() && !p->populated) // lazy population
        d->populate(p);
    return p->children.count();
}

/*!
  Returns the number of columns in the \a parent model item.

*/

int CWizCategoryModel::columnCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    return 1;
}

/*!
  Returns the data for the model item \a index with the given \a role.
*/
QVariant CWizCategoryModel::data(const QModelIndex &index, int role) const
{
    Q_D(const CWizCategoryModel);
    if (!d->indexValid(index))
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return d->name(index);
        case 1: return d->size(index);
        case 2: return d->type(index);
        case 3: return d->time(index);
        default:
            qWarning("data: invalid display value column %d", index.column());
            return QVariant();
        }
    }

    if (index.column() == 0) {
        if (role == FileIconRole)
            return fileIcon(index);
        if (role == FilePathRole)
            return filePath(index);
        if (role == FileNameRole)
            return fileName(index);
    }

    if (index.column() == 1 && Qt::TextAlignmentRole == role) {
        return Qt::AlignRight;
    }
    return QVariant();
}

/*!
  Sets the data for the model item \a index with the given \a role to
  the data referenced by the \a value. Returns true if successful;
  otherwise returns false.

  \sa Qt::ItemDataRole
*/

bool CWizCategoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(CWizCategoryModel);
    if (!d->indexValid(index) || index.column() != 0
        || (flags(index) & Qt::ItemIsEditable) == 0 || role != Qt::EditRole)
        return false;

    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(index);
    //
    if (node->info.isLocation())
    {

    }
    else
    {

    }
    /*
    //QDir dir = node->info.dir();
    QString name = value.toString();
    if (dir.rename(node->info.fileName(), name)) {
        node->info = CWizCategoryInfo(dir, name);
        QModelIndex sibling = index.sibling(index.row(), 3);
        emit dataChanged(index, sibling);

        d->toBeRefreshed = index.parent();
        QMetaObject::invokeMethod(this, "_q_refresh", Qt::QueuedConnection);

        return true;
    }
    */

    return false;
}

/*!
  Returns the data stored under the given \a role for the specified \a section
  of the header with the given \a orientation.
*/

QVariant CWizCategoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role != Qt::DisplayRole)
            return QVariant();
    switch (section) {
        case 0: return tr("Name");
        case 1: return tr("Size");
        case 2: return
#ifdef Q_OS_MAC
                       tr("Kind", "Match OS X Finder");
#else
                       tr("Type", "All other platforms");
#endif
        // Windows   - Type
        // OS X      - Kind
        // Konqueror - File Type
        // Nautilus  - Type
        case 3: return tr("Date Modified");
        default: return QVariant();
        }
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
  Returns true if the \a parent model item has children; otherwise
  returns false.
*/

bool CWizCategoryModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const CWizCategoryModel);
    if (parent.column() > 0)
        return false;

    if (!parent.isValid()) // the invalid index is the "My Computer" item
        return true; // the drives
    CWizCategoryModelPrivate::CWizCategoryNode *p = d->node(parent);
    Q_ASSERT(p);

    if (d->lazyChildCount) // optimization that only checks for children if the node has been populated
    {
        return p->info.hasChildren();
    }
    //
    return p->info.hasChildren();
}

/*!
  Returns the item flags for the given \a index in the model.

  \sa Qt::ItemFlags
*/
Qt::ItemFlags CWizCategoryModel::flags(const QModelIndex &index) const
{
    Q_D(const CWizCategoryModel);
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!d->indexValid(index))
        return flags;
    flags |= Qt::ItemIsDragEnabled;
    if (d->readOnly)
        return flags;
    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(index);
    if ((index.column() == 0) && node->info.isEditable()) {
        flags |= Qt::ItemIsEditable;
        if (categoryinfo(index).isDropable()) // is directory and is editable
            flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

/*!
  Sort the model items in the \a column using the \a order given.
  The order is a value defined in \l Qt::SortOrder.
*/

void CWizCategoryModel::sort(int column, Qt::SortOrder order)
{
    CWizCategoryInfo::SortFlags sort = CWizCategoryInfo::DirsFirst;
    sort |= CWizCategoryInfo::IgnoreCase;
    //
    if (order == Qt::DescendingOrder)
        sort |= CWizCategoryInfo::Reversed;

    switch (column) {
    case 0:
        sort |= CWizCategoryInfo::Name;
        break;
    case 1:
        sort |= CWizCategoryInfo::Size;
        break;
    case 2:
        sort |= CWizCategoryInfo::Type;
        break;
    case 3:
        sort |= CWizCategoryInfo::Time;
        break;
    default:
        break;
    }

    setSorting(sort);
}

/*!
    Returns a list of MIME types that can be used to describe a list of items
    in the model.
*/

QStringList CWizCategoryModel::mimeTypes() const
{
    return QStringList(QLatin1String("text/uri-list"));
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/

QMimeData *CWizCategoryModel::mimeData(const QModelIndexList &indexes) const
{
    return NULL;
    //QMimeData *data = new QMimeData();
    //return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row and
    \a column and by the \a parent index.

    \sa supportedDropActions()
*/

bool CWizCategoryModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                             int /* row */, int /* column */, const QModelIndex &parent)
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(parent);
    //
    return false;
}

/*!
  Returns the drop actions supported by this model.

  \sa Qt::DropActions
*/

Qt::DropActions CWizCategoryModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction; // FIXME: LinkAction is not supported yet
}

/*!
  Sets the \a provider of file icons for the directory model.

*/

/*
void CWizCategoryModel::setIconProvider(QFileIconProvider *provider)
{
    Q_D(CWizCategoryModel);
    d->iconProvider = provider;
}
*/

/*!
  Returns the file icon provider for this directory model.
*/

/*
QFileIconProvider *CWizCategoryModel::iconProvider() const
{
    Q_D(const CWizCategoryModel);
    return d->iconProvider;
}

*/

/*!
  Sets the name \a filters for the directory model.
*/

void CWizCategoryModel::setNameFilters(const QStringList &filters)
{
    Q_D(CWizCategoryModel);
    d->nameFilters = filters;
    emit layoutAboutToBeChanged();
    if (d->shouldStat)
       refresh(QModelIndex());
    else
        d->invalidate();
    emit layoutChanged();
}

/*!
  Returns a list of filters applied to the names in the model.
*/

QStringList CWizCategoryModel::nameFilters() const
{
    Q_D(const CWizCategoryModel);
    return d->nameFilters;
}


/*!
  Sets the directory model's sorting order to that specified by \a sort.

  \sa WizCategorySortFlag
*/

void CWizCategoryModel::setSorting(CWizCategoryInfo::SortFlags sort)
{
    Q_D(CWizCategoryModel);
    d->sort = sort;
    emit layoutAboutToBeChanged();
    if (d->shouldStat)
        refresh(QModelIndex());
    else
        d->invalidate();
    emit layoutChanged();
}

/*!
  Returns the sorting method used for the directory model.

  \sa WizCategorySortFlag */

CWizCategoryInfo::SortFlags CWizCategoryModel::sorting() const
{
    Q_D(const CWizCategoryModel);
    return d->sort;
}

/*!
    \property CWizCategoryModel::resolveSymlinks
    \brief Whether the directory model should resolve symbolic links

    This is only relevant on operating systems that support symbolic
    links.
*/
void CWizCategoryModel::setResolveSymlinks(bool enable)
{
    Q_D(CWizCategoryModel);
    d->resolveSymlinks = enable;
}

bool CWizCategoryModel::resolveSymlinks() const
{
    Q_D(const CWizCategoryModel);
    return d->resolveSymlinks;
}

/*!
  \property CWizCategoryModel::readOnly
  \brief Whether the directory model allows writing to the file system

  If this property is set to false, the directory model will allow renaming, copying
  and deleting of files and directories.

  This property is true by default
*/

void CWizCategoryModel::setReadOnly(bool enable)
{
    Q_D(CWizCategoryModel);
    d->readOnly = enable;
}

bool CWizCategoryModel::isReadOnly() const
{
    Q_D(const CWizCategoryModel);
    return d->readOnly;
}

/*!
  \property CWizCategoryModel::lazyChildCount
  \brief Whether the directory model optimizes the hasChildren function
  to only check if the item is a directory.

  If this property is set to false, the directory model will make sure that a directory
  actually containes any files before reporting that it has children.
  Otherwise the directory model will report that an item has children if the item
  is a directory.

  This property is false by default
*/

void CWizCategoryModel::setLazyChildCount(bool enable)
{
    Q_D(CWizCategoryModel);
    d->lazyChildCount = enable;
}

bool CWizCategoryModel::lazyChildCount() const
{
    Q_D(const CWizCategoryModel);
    return d->lazyChildCount;
}

/*!
  CWizCategoryModel caches file information. This function updates the
  cache. The \a parent parameter is the directory from which the
  model is updated; the default value will update the model from
  root directory of the file system (the entire model).
*/

void CWizCategoryModel::refresh(const QModelIndex &parent)
{
    Q_D(CWizCategoryModel);

    CWizCategoryModelPrivate::CWizCategoryNode *n = d->indexValid(parent) ? d->node(parent) : &(d->root);

    int rows = n->children.count();
    if (rows == 0) {
        emit layoutAboutToBeChanged();
        n->stat = true; // make sure that next time we read all the info
        n->populated = false;
        emit layoutChanged();
        return;
    }

    emit layoutAboutToBeChanged();
    d->savePersistentIndexes();
    d->rowsAboutToBeRemoved(parent, 0, rows - 1);
    n->stat = true; // make sure that next time we read all the info
    d->clear(n);
    d->rowsRemoved(parent, 0, rows - 1);
    d->restorePersistentIndexes();
    emit layoutChanged();
}

/*!
    \overload

    Returns the model item index for the given \a path.
*/

QModelIndex CWizCategoryModel::index(const QString &path, int column) const
{
    /*
    Q_D(const CWizCategoryModel);

    if (path.isEmpty())
        return QModelIndex();

    QString absolutePath = QDir(path).absolutePath();
#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    absolutePath = absolutePath.toLower();
#endif
#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    // On Windows, "filename......." and "filename" are equivalent
    if (absolutePath.endsWith(QLatin1Char('.'))) {
        int i;
        for (i = absolutePath.count() - 1; i >= 0; --i) {
            if (absolutePath.at(i) != QLatin1Char('.'))
                break;
        }
        absolutePath = absolutePath.left(i+1);
    }
#endif

    QStringList pathElements = absolutePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
    if ((pathElements.isEmpty() || !CWizCategoryInfo(path).exists())
#if !defined(Q_OS_WIN) || defined(Q_OS_WINCE)
        && path != QLatin1String("/")
#endif
        )
        return QModelIndex();

    QModelIndex idx; // start with "My Computer"
    if (!d->root.populated) // make sure the root is populated
        d->populate(&d->root);

#if defined(Q_OS_WIN) && !defined(Q_OS_WINCE)
    if (absolutePath.startsWith(QLatin1String("//"))) { // UNC path
        QString host = pathElements.first();
        int r = 0;
        for (; r < d->root.children.count(); ++r)
            if (d->root.children.at(r).info.fileName() == host)
                break;
        bool childAppended = false;
        if (r >= d->root.children.count() && d->allowAppendChild) {
            d->appendChild(&d->root, QLatin1String("//") + host);
            childAppended = true;
        }
        idx = index(r, 0, QModelIndex());
        pathElements.pop_front();
        if (childAppended)
            emit const_cast<CWizCategoryModel*>(this)->layoutChanged();
    } else
#endif
#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
    if (pathElements.at(0).endsWith(QLatin1Char(':'))) {
        pathElements[0] += QLatin1Char('/');
    }
#else
    // add the "/" item, since it is a valid path element on unix
    pathElements.prepend(QLatin1String("/"));
#endif

    for (int i = 0; i < pathElements.count(); ++i) {
        Q_ASSERT(!pathElements.at(i).isEmpty());
        QString element = pathElements.at(i);
        CWizCategoryModelPrivate::CWizCategoryNode *parent = (idx.isValid() ? d->node(idx) : &d->root);

        Q_ASSERT(parent);
        if (!parent->populated)
            d->populate(parent);

        // search for the element in the child nodes first
        int row = -1;
        for (int j = parent->children.count() - 1; j >= 0; --j) {
            const CWizCategoryInfo& fi = parent->children.at(j).info;
            QString childFileName;
            childFileName = idx.isValid() ? fi.fileName() : fi.absoluteFilePath();
#if (defined(Q_OS_WIN) && !defined(Q_OS_WINCE)) || defined(Q_OS_SYMBIAN)
            childFileName = childFileName.toLower();
#endif
            if (childFileName == element) {
                if (i == pathElements.count() - 1)
                    parent->children[j].stat = true;
                row = j;
                break;
            }
        }

        // we couldn't find the path element, we create a new node since we _know_ that the path is valid
        if (row == -1) {
#if defined(Q_OS_WINCE)
            QString newPath;
            if (parent->info.isRoot())
                newPath = parent->info.absoluteFilePath() + element;
            else
                newPath = parent->info.absoluteFilePath() + QLatin1Char('/') + element;
#else
            QString newPath = parent->info.absoluteFilePath() + QLatin1Char('/') + element;
#endif
            if (!d->allowAppendChild || !CWizCategoryInfo(newPath).isDir())
                return QModelIndex();
            d->appendChild(parent, newPath);
            row = parent->children.count() - 1;
            if (i == pathElements.count() - 1) // always stat children of  the last element
                parent->children[row].stat = true;
            emit const_cast<CWizCategoryModel*>(this)->layoutChanged();
        }

        Q_ASSERT(row >= 0);
        idx = createIndex(row, 0, static_cast<void*>(&parent->children[row]));
        Q_ASSERT(idx.isValid());
    }

    if (column != 0)
        return idx.sibling(idx.row(), column);
    return idx;
    */
    return QModelIndex();
}

/*!
  Returns true if the model item \a index represents a directory;
  otherwise returns false.
*/

bool CWizCategoryModel::isLocation(const QModelIndex &index) const
{
    Q_D(const CWizCategoryModel);
    Q_ASSERT(d->indexValid(index));
    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(index);
    return node->info.isLocation();
}

/*!
  Create a directory with the \a name in the \a parent model item.
*/

QModelIndex CWizCategoryModel::mkdir(const QModelIndex &parent, const QString &name)
{
    return QModelIndex();
    /*
    Q_D(CWizCategoryModel);
    if (!d->indexValid(parent) || isReadOnly())
        return QModelIndex();

    CWizCategoryModelPrivate::CWizCategoryNode *p = d->node(parent);
    QString path = p->info.absoluteFilePath();
    // For the indexOf() method to work, the new directory has to be a direct child of
    // the parent directory.

    QDir newDir(name);
    QDir dir(path);
    if (newDir.isRelative())
        newDir = QDir(path + QLatin1Char('/') + name);
    QString childName = newDir.dirName(); // Get the singular name of the directory
    newDir.cdUp();

    if (newDir.absolutePath() != dir.absolutePath() || !dir.mkdir(name))
        return QModelIndex(); // nothing happened

    refresh(parent);

    QStringList entryList = d->entryList(path);
    int r = entryList.indexOf(childName);
    QModelIndex i = index(r, 0, parent); // return an invalid index

    return i;
    */
}

/*!
  Removes the directory corresponding to the model item \a index in the
  directory model and \bold{deletes the corresponding directory from the
  file system}, returning true if successful. If the directory cannot be
  removed, false is returned.

  \warning This function deletes directories from the file system; it does
  \bold{not} move them to a location where they can be recovered.

  \sa remove()
*/

bool CWizCategoryModel::rmdir(const QModelIndex &index)
{
    /*
    Q_D(CWizCategoryModel);
    if (!d->indexValid(index) || isReadOnly())
        return false;

    CWizCategoryModelPrivate::CWizCategoryNode *n = d_func()->node(index);
    if (!n->info.isDir()) {
        qWarning("rmdir: the node is not a directory");
        return false;
    }

    QModelIndex par = parent(index);
    CWizCategoryModelPrivate::CWizCategoryNode *p = d_func()->node(par);
    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.rmdir(path))
        return false;

    refresh(par);

    return true;
    */
    return false;
}

/*!
  Removes the model item \a index from the directory model and \bold{deletes the
  corresponding file from the file system}, returning true if successful. If the
  item cannot be removed, false is returned.

  \warning This function deletes files from the file system; it does \bold{not}
  move them to a location where they can be recovered.

  \sa rmdir()
*/

bool CWizCategoryModel::remove(const QModelIndex &index)
{
    /*
    Q_D(CWizCategoryModel);
    if (!d->indexValid(index) || isReadOnly())
        return false;

    CWizCategoryModelPrivate::CWizCategoryNode *n = d_func()->node(index);
    if (n->info.isDir())
        return false;

    QModelIndex par = parent(index);
    CWizCategoryModelPrivate::CWizCategoryNode *p = d_func()->node(par);
    QDir dir = p->info.dir(); // parent dir
    QString path = n->info.absoluteFilePath();
    if (!dir.remove(path))
        return false;

    refresh(par);

    return true;
    */
    return false;
}

/*!
  Returns the path of the item stored in the model under the
  \a index given.

*/

QString CWizCategoryModel::filePath(const QModelIndex &index) const
{
    /*
    Q_D(const CWizCategoryModel);
    if (d->indexValid(index)) {
        CWizCategoryInfo fi = categoryinfo(index);
        if (d->resolveSymlinks && fi.isSymLink())
            fi = d->resolvedInfo(fi);
        return QDir::cleanPath(fi.absoluteFilePath());
    }
    return QString(); // root path
    */
    return QString();
}

/*!
  Returns the name of the item stored in the model under the
  \a index given.

*/

QString CWizCategoryModel::fileName(const QModelIndex &index) const
{
    /*
    Q_D(const CWizCategoryModel);
    if (!d->indexValid(index))
        return QString();
    CWizCategoryInfo info = categoryinfo(index);
    if (info.isRoot())
        return info.absoluteFilePath();
    if (d->resolveSymlinks && info.isSymLink())
        info = d->resolvedInfo(info);
    return info.fileName();
    */
    return QString();
}

/*!
  Returns the icons for the item stored in the model under the given
  \a index.
*/

QIcon CWizCategoryModel::fileIcon(const QModelIndex &index) const
{
    /*
    Q_D(const CWizCategoryModel);
    if (!d->indexValid(index))
        return d->iconProvider->icon(QFileIconProvider::Computer);
    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(index);
    if (node->icon.isNull())
        node->icon = d->iconProvider->icon(node->info);
    return node->icon;
    */
    return QIcon();
}

/*!
  Returns the file information for the specified model \a index.

  \bold{Note:} If the model index represents a symbolic link in the
  underlying filing system, the file information returned will contain
  information about the symbolic link itself, regardless of whether
  resolveSymlinks is enabled or not.

  \sa CWizCategoryInfo::symLinkTarget()
*/

CWizCategoryInfo CWizCategoryModel::categoryinfo(const QModelIndex &index) const
{
    Q_D(const CWizCategoryModel);
    Q_ASSERT(d->indexValid(index));

    CWizCategoryModelPrivate::CWizCategoryNode *node = d->node(index);
    return node->info;
}

/*!
  \fn QObject *CWizCategoryModel::parent() const
  \internal
*/

/*
  The root node is never seen outside the model.
*/

void CWizCategoryModelPrivate::init()
{
    Q_Q(CWizCategoryModel);
    sort = CWizCategoryInfo::Name;
    nameFilters << QLatin1String("*");
    root.parent = 0;
    root.info = CWizCategoryInfo();
    clear(&root);
    QHash<int, QByteArray> roles = q->roleNames();
    roles.insertMulti(CWizCategoryModel::FileIconRole, "fileIcon"); // == Qt::decoration
    roles.insert(CWizCategoryModel::FilePathRole, "filePath");
    roles.insert(CWizCategoryModel::FileNameRole, "fileName");
    q->setRoleNames(roles);
}

CWizCategoryModelPrivate::CWizCategoryNode *CWizCategoryModelPrivate::node(int row, CWizCategoryNode *parent) const
{
    if (row < 0)
    return 0;

    bool isDir = !parent || parent->info.isLocation();
    CWizCategoryNode *p = (parent ? parent : &root);
    if (isDir && !p->populated)
        populate(p); // will also resolve symlinks

    if (row >= p->children.count()) {
        qWarning("node: the row does not exist");
        return 0;
    }

    return const_cast<CWizCategoryNode*>(&p->children.at(row));
}

QVector<CWizCategoryModelPrivate::CWizCategoryNode> CWizCategoryModelPrivate::children(CWizCategoryNode *parent, bool stat) const
{
    /*
    Q_ASSERT(parent);
    CWizCategoryInfoList infoList;
    if (parent == &root) {
        parent = 0;
        infoList = QDir::drives();
    } else if (parent->info.isDir()) {
        //resolve directory links only if requested.
        if (parent->info.isSymLink() && resolveSymlinks) {
            QString link = parent->info.symLinkTarget();
            if (link.size() > 1 && link.at(link.size() - 1) == QDir::separator())
                link.chop(1);
            if (stat)
                infoList = entryInfoList(link);
            else
                infoList = QDir(link).entryInfoList(nameFilters, QDir::AllEntries | QDir::System);
        } else {
            if (stat)
                infoList = entryInfoList(parent->info.absoluteFilePath());
            else
                infoList = QDir(parent->info.absoluteFilePath()).entryInfoList(nameFilters, QDir::AllEntries | QDir::System);
        }
    }

    QVector<CWizCategoryNode> nodes(infoList.count());
    for (int i = 0; i < infoList.count(); ++i) {
        CWizCategoryNode &node = nodes[i];
        node.parent = parent;
        node.info = infoList.at(i);
        node.populated = false;
        node.stat = shouldStat;
    }

    return nodes;
    */
    QVector<CWizCategoryNode> nodes;
    return nodes;
}

void CWizCategoryModelPrivate::_q_refresh()
{
    Q_Q(CWizCategoryModel);
    q->refresh(toBeRefreshed);
    toBeRefreshed = QModelIndex();
}

void CWizCategoryModelPrivate::savePersistentIndexes()
{
    Q_Q(CWizCategoryModel);
    savedPersistent.clear();
    foreach (QPersistentModelIndexData *data, persistent.indexes) {
        SavedPersistent saved;
        QModelIndex index = data->index;
        saved.path = q->filePath(index);
        saved.column = index.column();
        saved.data = data;
        saved.index = index;
        savedPersistent.append(saved);
    }
}

void CWizCategoryModelPrivate::restorePersistentIndexes()
{
    Q_Q(CWizCategoryModel);
    bool allow = allowAppendChild;
    allowAppendChild = false;
    for (int i = 0; i < savedPersistent.count(); ++i) {
        QPersistentModelIndexData *data = savedPersistent.at(i).data;
        QString path = savedPersistent.at(i).path;
        int column = savedPersistent.at(i).column;
        QModelIndex idx = q->index(path, column);
        if (idx != data->index || data->model == 0) {
            //data->model may be equal to 0 if the model is getting destroyed
            persistent.indexes.remove(data->index);
            data->index = idx;
            data->model = q;
            if (idx.isValid())
                persistent.indexes.insert(idx, data);
        }
    }
    savedPersistent.clear();
    allowAppendChild = allow;
}

CWizCategoryInfoList CWizCategoryModelPrivate::entryInfoList(const QString &path) const
{
    /*
    const QDir dir(path);
    return dir.entryInfoList(nameFilters, filters, sort);
    */
}

QStringList CWizCategoryModelPrivate::entryList(const QString &path) const
{
    /*
    const QDir dir(path);
    return dir.entryList(nameFilters, filters, sort);
    */
}

QString CWizCategoryModelPrivate::name(const QModelIndex &index) const
{
    const CWizCategoryNode *n = node(index);
    const CWizCategoryInfo info = n->info;
    if (info.isAllLocations())
    {
        QString name = "All folders";
        return name;
    }
    return info.name();
}

QString CWizCategoryModelPrivate::size(const QModelIndex &index) const
{
    return 0;
}

QString CWizCategoryModelPrivate::type(const QModelIndex &index) const
{
    return "";
    //return iconProvider->type(node(index)->info);
}

QString CWizCategoryModelPrivate::time(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QString();
}

void CWizCategoryModelPrivate::appendChild(CWizCategoryModelPrivate::CWizCategoryNode *parent, const QString &path) const
{
    CWizCategoryModelPrivate::CWizCategoryNode node;
    node.populated = false;
    node.stat = shouldStat;
    node.parent = (parent == &root ? 0 : parent);
    node.info = CWizCategoryInfo(path);
    node.info.setCaching(true);

    // The following append(node) may reallocate the vector, thus
    // we need to update the pointers to the childnodes parent.
    CWizCategoryModelPrivate *that = const_cast<CWizCategoryModelPrivate *>(this);
    that->savePersistentIndexes();
    parent->children.append(node);
    for (int i = 0; i < parent->children.count(); ++i) {
        CWizCategoryNode *childNode = &parent->children[i];
        for (int j = 0; j < childNode->children.count(); ++j)
            childNode->children[j].parent = childNode;
    }
    that->restorePersistentIndexes();
}

#include "moc_wizcategorymodel.cpp"
