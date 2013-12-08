#include "titleedit.h"

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "wizDocumentView.h"

using namespace Core;
using namespace Core::Internal;

TitleEdit::TitleEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setStyleSheet("font-size: 12px;");
    setContentsMargins(5, 0, 0, 0);
    setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFrame(false);

    connect(this, SIGNAL(returnPressed()), SLOT(onTitleReturnPressed()));
    connect(this, SIGNAL(editingFinished()), SLOT(onTitleEditingFinished()));
}


void TitleEdit::inputMethodEvent(QInputMethodEvent* event)
{
    // FIXME: This should be a QT bug
    // when use input method, input as normal is fine, but input as selection text
    // will lose blink cursor, we should reset cursor position first!!!
    if (hasSelectedText()) {
        del();
    }

    QLineEdit::inputMethodEvent(event);
}

QSize TitleEdit::sizeHint() const
{
    return QSize(fontMetrics().width(text()), fontMetrics().height() + 10);
}

CWizDocumentView* TitleEdit::noteView()
{
    QWidget* pParent = parentWidget();
    while (pParent) {
        CWizDocumentView* view = dynamic_cast<CWizDocumentView*>(pParent);
        if (view) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void TitleEdit::setReadOnly(bool b)
{
    QLineEdit::setReadOnly(b);

    // Qt-bug: Must always set this flag after setReadOnly
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

void TitleEdit::onTitleEditingFinished()
{
    WIZDOCUMENTDATA data;
    CWizDatabase& db = CWizDatabaseManager::instance()->db(noteView()->note().strKbGUID);
    if (db.DocumentFromGUID(noteView()->note().strGUID, data)) {
        QString strNewTitle = text().left(255);
        if (strNewTitle != data.strTitle) {
            data.strTitle = strNewTitle;
            db.ModifyDocumentInfo(data);
        }
    }
}

void TitleEdit::onTitleReturnPressed()
{
    noteView()->setEditorFocus();
    //m_web->setFocus(Qt::MouseFocusReason);
    //m_web->editorFocus();
}
