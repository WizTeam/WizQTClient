#include "titleedit.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QDebug>

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "wizDocumentView.h"

using namespace Core;
using namespace Core::Internal;

TitleEdit::TitleEdit(QWidget *parent)
    : QLineEdit(parent)
    , c(NULL)
{
    setStyleSheet("font-size: 12px;");
    setContentsMargins(5, 0, 0, 0);
    setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFrame(false);

    connect(this, SIGNAL(returnPressed()), SLOT(onTitleReturnPressed()));
    connect(this, SIGNAL(editingFinished()), SLOT(onTitleEditingFinished()));
}

QSize TitleEdit::sizeHint() const
{
    return QSize(fontMetrics().width(text()), fontMetrics().height() + 10);
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

    //QString strCompPrefix = textUnderCursor();
    //if (strCompPrefix != c->completionPrefix()) {
    //    updateCompleterPopupItems(strCompPrefix);
    //}
}

void TitleEdit::keyPressEvent(QKeyEvent* event)
{
    if (c && c->popup()->isVisible()) {
        switch (event->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
             event->ignore();
             return;
        default:
            break;
        }
    }

    QLineEdit::keyPressEvent(event);

    QString strCompPrefix = textUnderCursor();
    if (strCompPrefix != c->completionPrefix()) {
        updateCompleterPopupItems(strCompPrefix);
    }

    if (event->text().size() && strCompPrefix.size()) {
        c->complete();
    }

    if (!strCompPrefix.size()) {
        completer()->popup()->hide();
    }
}

void TitleEdit::updateCompleterPopupItems(const QString& completionPrefix)
{
    completer()->setCompletionPrefix(completionPrefix);
    completer()->popup()->setCurrentIndex(completer()->completionModel()->index(0, 0));
}

QString TitleEdit::textUnderCursor()
{
    QString strText;
    int i = cursorPosition() - 1;
    while (i >= 0 && text().at(i) != ' ') {
        strText = text().at(i) + strText;
        i--;
    }

    qDebug() << strText;

    return strText;
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

void TitleEdit::setCompleter(QCompleter* completer)
{
    if (c)
        QObject::disconnect(c, 0, this, 0);

    c = completer;

    if (!c)
        return;

    c->setWidget(this);
    c->setCompletionMode(QCompleter::PopupCompletion);
    c->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(c, SIGNAL(activated(const QString&)), SLOT(onInsertCompletion(const QString&)));
}

void TitleEdit::onInsertCompletion(const QString& completion)
{
    if (c->widget() != this)
        return;

    int extra = completion.length() - c->completionPrefix().length();
    setText(text() + completion.right(extra) + " ");
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
