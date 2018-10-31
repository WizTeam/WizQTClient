#include "WizTitleEdit.h"

#include <QCompleter>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QDebug>

#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "WizDocumentView.h"
#include "WizMainWindow.h"
#include "WizDocumentWebView.h"
#include "WizDocumentWebEngine.h"
#include "share/WizQtHelper.h"

WizTitleEdit::WizTitleEdit(QWidget *parent)
    : QLineEdit(parent)
    , c(NULL)
    , m_separator('@')
{    
    setContentsMargins(0, 0, 0, 0);
    setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setFrame(false);

    connect(this, SIGNAL(returnPressed()), SLOT(onTitleReturnPressed()));
    connect(this, SIGNAL(editingFinished()), SLOT(onTitleEditingFinished()));
    connect(this, SIGNAL(textEdited(QString)), SLOT(onTextEdit(QString)));
    connect(this, SIGNAL(textChanged(QString)), SLOT(onTextChanged(QString)));
    QFont f = font();
    f.setPixelSize(WizSmartScaleUI(14));
    setFont(f);
}

QSize WizTitleEdit::sizeHint() const
{
    return QSize(fontMetrics().width(text()), fontMetrics().height() + 10);
}

void WizTitleEdit::keyPressEvent(QKeyEvent* e)
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

    // NOTE:目前不在keypress的时候处理提示的问题，原因是中文输入法不会触发keypress事件，
    //改为在textedit事件中处理
//    if (!c)
//        return;

//    QString completionPrefix = textUnderCursor();
//    bool isSeparator = (!completionPrefix.isEmpty() || charBeforeCursor() == m_separator) ? true : false;

//    if (!isSeparator) {
//        c->popup()->hide();
//        return;
//    }

//    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
//    if (!c || (ctrlOrShift && e->text().isEmpty()))
//        return;

//    static QString eow("~!#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
//    bool hasModifier = isSeparator && !ctrlOrShift;

//    if (!isSeparator && (hasModifier || e->text().isEmpty()
//                      || eow.contains(e->text().right(1)))) {
//        c->popup()->hide();
//        return;
//    }

//    if (completionPrefix != c->completionPrefix()) {
//        updateCompleterPopupItems(completionPrefix);
//    }

//    QRect cr = cursorRect();
//    cr.setWidth(c->popup()->sizeHintForColumn(0)
//                + c->popup()->verticalScrollBar()->sizeHint().width() + 20); // bigger
//    c->complete(cr); // popup it up!
}




void WizTitleEdit::contextMenuEvent(QContextMenuEvent* e)
{
    QLineEdit::contextMenuEvent(e);
}

void WizTitleEdit::updateCompleterPopupItems(const QString& completionPrefix)
{
    c->setCompletionPrefix(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
}

QString WizTitleEdit::textUnderCursor()
{
    QString strText;
    int i = cursorPosition() - 1;
    while (i >= 0 && text().at(i) != m_separator) {
        strText = text().at(i) + strText;
        i--;
    }

    if (-1 == i) {
        return QString();
    }

    return strText;
}

QChar WizTitleEdit::charBeforeCursor()
{
    int i = cursorPosition() - 1;
    if (i >= 0)
        return text().at(i);

    return QChar();
}

WizDocumentView* WizTitleEdit::noteView()
{
    QWidget* pParent = parentWidget();
    while (pParent) {
        WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent);
        if (view) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void WizTitleEdit::resetTitle(const QString& strTitle)
{
    if (strTitle.isEmpty())
        return;

    setText(strTitle);
    onTitleEditingFinished();
}

void WizTitleEdit::setReadOnly(bool b)
{
    QLineEdit::setReadOnly(b);

    // Qt-bug: Must always set this flag after setReadOnly
    setAttribute(Qt::WA_MacShowFocusRect, false);

    // Focre to update document title right now
//    if (b)
//        onTitleEditingFinished();
}

void WizTitleEdit::setCompleter(QCompleter* completer)
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

void WizTitleEdit::onInsertCompletion(const QModelIndex& index)
{
    if (c->widget() != this)
        return;

    QString strOrigin = text();
    QString strExtra = c->completionModel()->data(index, Qt::DisplayRole).toString();
    int nDel = textUnderCursor().size();
    QString strNew = strOrigin.left(strOrigin.size() - nDel) + strExtra + " ";
    setText(strNew);
}

void WizTitleEdit::onTitleEditingFinished()
{
    setCursorPosition(0);
    //
    WIZDOCUMENTDATA data;
    WizDatabase& db = WizDatabaseManager::instance()->db(noteView()->note().strKbGUID);
    if (db.documentFromGuid(noteView()->note().strGUID, data)) {
        if (!db.canEditDocument(data))
            return;

        QString strNewTitle = text().left(255);
        if (strNewTitle.isEmpty() && !placeholderText().isEmpty()) {
            strNewTitle = placeholderText().left(255);
        }
        strNewTitle.replace("\n", " ");
        strNewTitle.replace("\r", " ");
        strNewTitle = strNewTitle.trimmed();
        if (strNewTitle != data.strTitle) {
            data.strTitle = strNewTitle;
            data.tDataModified = WizGetCurrentTime();
            db.modifyDocumentInfo(data);

            emit titleEdited(strNewTitle);
        }
    }
}

void WizTitleEdit::setText(const QString& text)
{
    QLineEdit::setText(text);
    setCursorPosition(0);
    if (isDarkMode()) {
        setStyleSheet("color:#a6a6a6;background-color:#272727");
    } else {
        setStyleSheet("color:#535353;");
    }
}

void WizTitleEdit::onTitleReturnPressed()
{
    noteView()->setEditorFocus();
    noteView()->web()->setFocus(Qt::MouseFocusReason);
    noteView()->web()->editorFocus();
}

void WizTitleEdit::onTextEdit(const QString& text)
{
    if (!c)
        return;

    QString completionPrefix = textUnderCursor();
    bool isSeparator = (!completionPrefix.isEmpty() || charBeforeCursor() == m_separator) ? true : false;

    if (!isSeparator) {
        c->popup()->hide();
        return;
    }


    static QString eow("~!#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word

    if (!isSeparator && (text.isEmpty()
                      || eow.contains(text.right(1)))) {
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

void WizTitleEdit::onTextChanged(const QString& text)
{    
}
