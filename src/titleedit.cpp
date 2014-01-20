#include "titleedit.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDebug>

#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "wizDocumentView.h"

using namespace Core;
using namespace Core::Internal;

TitleEdit::TitleEdit(QWidget *parent)
    : QLineEdit(parent)
    , c(NULL)
    , m_separator('@')
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

void TitleEdit::keyPressEvent(QKeyEvent* e)
{
    if (c && c->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
       switch (e->key()) {
       case Qt::Key_Enter:
       case Qt::Key_Return:
       case Qt::Key_Escape:
       case Qt::Key_Tab:
       case Qt::Key_Backtab:
            e->ignore();
            return; // let the completer do default behavior
       default:
           break;
       }
    }

    QLineEdit::keyPressEvent(e);

    if (!c)
        return;

    QString completionPrefix = textUnderCursor();
    bool isSeparator = (!completionPrefix.isEmpty() || charBeforeCursor() == m_separator) ? true : false;

    if (!isSeparator) {
        c->popup()->hide();
        return;
    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!c || (ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = isSeparator && !ctrlOrShift;

    if (!isSeparator && (hasModifier || e->text().isEmpty()
                      || eow.contains(e->text().right(1)))) {
        c->popup()->hide();
        return;
    }

    if (completionPrefix != c->completionPrefix()) {
        updateCompleterPopupItems(completionPrefix);
    }

    QRect cr = cursorRect();
    cr.setWidth(c->popup()->sizeHintForColumn(0)
                + c->popup()->verticalScrollBar()->sizeHint().width() + 20); // bigger
    c->complete(cr); // popup it up!
}

void TitleEdit::updateCompleterPopupItems(const QString& completionPrefix)
{
    c->setCompletionPrefix(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

QString TitleEdit::textUnderCursor()
{
    QString strText;
    int i = cursorPosition() - 1;
    while (i >= 0 && text().at(i) != m_separator) {
        strText = text().at(i) + strText;
        i--;
    }

    if (-1 == i) {
        return NULL;
    }

    return strText;
}

QChar TitleEdit::charBeforeCursor()
{
    int i = cursorPosition() - 1;
    if (i >= 0)
        return text().at(i);

    return QChar();
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

void TitleEdit::resetTitle(const QString& strTitle)
{
    if (strTitle.isEmpty())
        return;

    setText(strTitle);
    onTitleEditingFinished();
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

    connect(c, SIGNAL(activated(const QModelIndex &)), SLOT(onInsertCompletion(const QModelIndex &)));
}

void TitleEdit::onInsertCompletion(const QModelIndex& index)
{
    if (c->widget() != this)
        return;

    QString strOrigin = text();
    QString strExtra = c->completionModel()->data(index, Qt::DisplayRole).toString();
    int nDel = textUnderCursor().size();
    setText(strOrigin.left(strOrigin.size() - nDel) + strExtra + " ");
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
