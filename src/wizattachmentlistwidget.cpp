#include "wizattachmentlistwidget.h"

#include <QBoxLayout>

CWizAttachmentListView::CWizAttachmentListView(CWizDatabase& db, QWidget* parent)
    : QListWidget(parent)
    , m_db(db)
{
    setFrameStyle(QFrame::NoFrame);
    //
}

void CWizAttachmentListView::resetAttachments()
{
    clear();
    //
    CWizDocumentAttachmentDataArray arrayAttachment;
    m_db.GetDocumentAttachments(m_document.strGUID, arrayAttachment);
    //
    for (CWizDocumentAttachmentDataArray::const_iterator it = arrayAttachment.begin();
    it != arrayAttachment.end();
    it++)
    {
        addItem(it->strName);
    }
}


void CWizAttachmentListView::setDocument(const WIZDOCUMENTDATA& document)
{
    m_document = document;
}


CWizAttachmentListWidget::CWizAttachmentListWidget(CWizDatabase& db, QWidget* parent)
    : CWizPopupWidget(parent)
    , m_list(new CWizAttachmentListView(db, this))
{
    QLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom, this);
    setLayout(layout);
    layout->setMargin(0);
    //
    layout->addWidget(m_list);
}

void CWizAttachmentListWidget::setDocument(const WIZDOCUMENTDATA& document)
{
    m_list->setDocument(document);;
}
