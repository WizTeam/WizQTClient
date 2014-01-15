#include "notifybar.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace Core::Internal;

NotifyBar::NotifyBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("* {font-size:12px; color: #FFFFFF;} *:active {background: url(:/notify_bg.png);} *:!active {background: url(:/notify_bg_inactive.png);}");

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(8, 5, 8, 5);
    layout->setSpacing(15);
    setLayout(layout);

    m_labelNotify = new QLabel(this);
    m_labelNotify->setAttribute(Qt::WA_NoSystemBackground, true);
    layout->addWidget(m_labelNotify);
    layout->addStretch();
}

void NotifyBar::showNotify(int type)
{
    switch (type) {
    case NotifyBar::Locked:
        m_labelNotify->setText(QObject::tr("The note is locked and read only, press unlock button if you need edit."));
        show();
        break;
    case NotifyBar::Deleted:
        m_labelNotify->setText(QObject::tr("This note is deleted, You can edit after move to other folders."));
        show();
        break;
    case NotifyBar::PermissionLack:
        m_labelNotify->setText(QObject::tr("Your permission is not enough to edit this note."));
        show();
        break;
    default:
        hide();
    }
}
