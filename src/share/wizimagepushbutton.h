#ifndef WIZIMAGEPUSHBUTTON_H
#define WIZIMAGEPUSHBUTTON_H

#include <QPushButton>

class CWizImagePushButton : public QPushButton
{
public:
    CWizImagePushButton(const QIcon& icon, const QString &text, QWidget* parent);
private:
    bool m_redFlag;
public:
    virtual QSize sizeHint() const;
public:
    void setRedFlag(bool b) { m_redFlag = b; repaint(); }
    bool redFlag() const { return m_redFlag; }
};

#endif // WIZIMAGEPUSHBUTTON_H
