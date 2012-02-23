#ifndef WIZIMAGEPUSHBUTTON_H
#define WIZIMAGEPUSHBUTTON_H

#include <QPushButton>

class CWizImagePushButton : public QPushButton
{
public:
    CWizImagePushButton(const QIcon& icon, const QString &text, QWidget* parent);
public:
    virtual QSize sizeHint() const;
};

#endif // WIZIMAGEPUSHBUTTON_H
