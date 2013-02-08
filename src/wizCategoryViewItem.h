#ifndef WIZCATEGORYVIEWITEM_H
#define WIZCATEGORYVIEWITEM_H

#include <QTreeWidget>

#include "wizdef.h"
#include "share/wizDatabase.h"

class CWizCategoryBaseView;

class CWizCategoryViewItemBase : public QTreeWidgetItem
{
public:
    CWizCategoryViewItemBase(CWizExplorerApp& app, const QString& strName = "", const QString& strKbGUID = "");
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) = 0;
    virtual void getDocuments(CWizDatabase& dbMgr, CWizDocumentDataArray& arrayDocument) = 0;
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data) { Q_UNUSED(db); Q_UNUSED(data); return false; }

    virtual QVariant data(int column, int role) const;
    virtual int getItemHeight(int hintHeight) const;
    virtual bool operator<(const QTreeWidgetItem &other) const;

    const QString& kbGUID() const { return m_strKbGUID; }

protected:
    CWizExplorerApp& m_app;
    QString m_strName;
    QString m_strKbGUID;
};

class CWizCategoryViewSeparatorItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewSeparatorItem(CWizExplorerApp& app);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument) { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
    virtual int getItemHeight(int hintHeight) const;
};

class CWizCategoryViewAllFoldersItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewAllFoldersItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewFolderItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewFolderItem(CWizExplorerApp& app, const QString& strLocation, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);

    virtual QTreeWidgetItem* clone() const;

    QString location() const { return m_strName; }
    QString name() const { return CWizDatabase::GetLocationName(m_strName); }
};

class CWizCategoryViewAllTagsItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewAllTagsItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewTagItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewTagItem(CWizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);

    virtual QTreeWidgetItem *clone() const;

    void reload(CWizDatabase& db);
    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};

class CWizCategoryViewGroupRootItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewGroupRootItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
    void reload(CWizDatabase& db);
};

class CWizCategoryViewGroupItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewGroupItem(CWizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
    void reload(CWizDatabase& db);

    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};

class CWizCategoryViewTrashItem : public CWizCategoryViewFolderItem
{
public:
    CWizCategoryViewTrashItem(CWizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);
};

class CWizCategoryViewSearchItem : public CWizCategoryViewItemBase
{
public:
    CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords);
    virtual void showContextMenu(CWizCategoryBaseView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(CWizDatabase& db, CWizDocumentDataArray& arrayDocument) { Q_UNUSED(db); Q_UNUSED(arrayDocument); }
    virtual bool accept(CWizDatabase& db, const WIZDOCUMENTDATA& data);

    void setKeywords(const QString& keywords);
};

#endif // WIZCATEGORYVIEWITEM_H
