#ifndef CORE_TITLEEDIT_H
#define CORE_TITLEEDIT_H

#include <QLineEdit>

class QInputMethodEvent;

namespace Core {
namespace Internal {
class CWizDocumentView;

class TitleEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit TitleEdit(QWidget *parent);
    void setReadOnly(bool b);

protected:
    void inputMethodEvent(QInputMethodEvent* event);
    QSize sizeHint() const;

private:
    CWizDocumentView* noteView();

private Q_SLOTS:
    void onTitleEditingFinished();
    void onTitleReturnPressed();
};

} // namespace Internal
} // namespace Core

#endif // CORE_TITLEEDIT_H
