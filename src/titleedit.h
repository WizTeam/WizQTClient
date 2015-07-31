#ifndef CORE_TITLEEDIT_H
#define CORE_TITLEEDIT_H

#include <QLineEdit>

class QCompleter;
class QModelIndex;
class QInputMethodEvent;

namespace Core {
class CWizDocumentView;

namespace Internal {

class TitleEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit TitleEdit(QWidget *parent);
    void resetTitle(const QString& strTitle);
    void setReadOnly(bool b);

    void setCompleter(QCompleter* completer);
    QCompleter* completer() const { return c; }

public slots:
    void onTitleEditingFinished();
    void setText(const QString &text);

signals:
    void titleEdited(QString strTitle);

protected:
    QSize sizeHint() const;
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void contextMenuEvent(QContextMenuEvent* e);

private:
    QCompleter* c;
    QChar m_separator;

    void updateCompleterPopupItems(const QString& completionPrefix);
    QString textUnderCursor();
    QChar charBeforeCursor();

    CWizDocumentView* noteView();

private Q_SLOTS:
    void onInsertCompletion(const QModelIndex &index);    
    void onTitleReturnPressed();
    void onTextEdit(const QString & text);
    void onTextChanged(const QString & text);
};

} // namespace Internal
} // namespace Core

#endif // CORE_TITLEEDIT_H
