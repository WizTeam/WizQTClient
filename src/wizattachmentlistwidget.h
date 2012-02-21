#ifndef WIZATTACHMENTLISTWIDGET_H
#define WIZATTACHMENTLISTWIDGET_H

#include "share/wizpopupwidget.h"
#include <QListWidget>
#include "share/wizdatabase.h"

class CWizAttachmentListView : public QListWidget
{
    Q_OBJECT
public:
    CWizAttachmentListView(CWizDatabase& db, QWidget* parent);
private:
    CWizDatabase& m_db;
    WIZDOCUMENTDATA m_document;
private:
    void resetAttachments();
public:
    void setDocument(const WIZDOCUMENTDATA& document);
};


class CWizAttachmentListWidget : public CWizPopupWidget
{
public:
    CWizAttachmentListWidget(CWizDatabase& db, QWidget* parent);
private:
    CWizAttachmentListView* m_list;
public:
    void setDocument(const WIZDOCUMENTDATA& document);
};

#endif // WIZATTACHMENTLISTWIDGET_H
