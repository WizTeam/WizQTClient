#ifndef WIZTAGLISTWIDGET_H
#define WIZTAGLISTWIDGET_H

#include <QPointer>
#include <deque>

#include "share/WizPopupWidget.h"
#include "share/WizObject.h"

class QLineEdit;
class QListWidget;
class QListWidgetItem;

class WizDatabaseManager;

typedef std::deque<WIZDOCUMENTDATAEX> CWizDocumentDataArray;

class WizTagListWidget : public WizPopupWidget
{
    Q_OBJECT
public:
    WizTagListWidget(QWidget* parent);

    void setDocument(const WIZDOCUMENTDATAEX& doc);
    void setDocuments(const CWizDocumentDataArray& arrayDocument);

    virtual void showEvent(QShowEvent* event);

private:
    WizDatabaseManager& m_dbMgr;
    CWizDocumentDataArray m_arrayDocuments;
    bool m_bUpdating; // avoid itemChanged signal emit programmatically

//    QPointer<QLineEdit> m_tagsEdit;
    QPointer<QListWidget> m_list;

    void reloadTags();

    //void updateTagsText();

public Q_SLOTS:
    void on_list_itemChanged(QListWidgetItem* pItem);
    void on_tagsEdit_returnPressed();
};

#endif // WIZTAGLISTWIDGET_H
