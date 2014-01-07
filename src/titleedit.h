#ifndef CORE_TITLEEDIT_H
#define CORE_TITLEEDIT_H

#include <QLineEdit>

class QCompleter;
class QInputMethodEvent;

namespace Core {
class CWizDocumentView;

namespace Internal {

class TitleEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit TitleEdit(QWidget *parent);
    void setReadOnly(bool b);

    void setCompleter(QCompleter* completer);
    QCompleter* completer() const { return c; }

protected:
    QSize sizeHint() const;
    virtual void inputMethodEvent(QInputMethodEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);

private:
    QCompleter* c;

    void updateCompleterPopupItems(const QString& completionPrefix);
    QString textUnderCursor();

    CWizDocumentView* noteView();

private Q_SLOTS:
    void onInsertCompletion(const QString& completion);
    void onTitleEditingFinished();
    void onTitleReturnPressed();
};

} // namespace Internal
} // namespace Core

#endif // CORE_TITLEEDIT_H
