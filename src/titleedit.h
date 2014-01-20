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

protected:
    QSize sizeHint() const;
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* e);

private:
    QCompleter* c;
    QChar m_separator;

    void updateCompleterPopupItems(const QString& completionPrefix);
    QString textUnderCursor();
    QChar charBeforeCursor();

    CWizDocumentView* noteView();

private Q_SLOTS:
    void onInsertCompletion(const QModelIndex &index);
    void onTitleEditingFinished();
    void onTitleReturnPressed();
};

} // namespace Internal
} // namespace Core

#endif // CORE_TITLEEDIT_H
