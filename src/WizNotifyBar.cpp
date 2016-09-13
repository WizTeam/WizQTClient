#include "WizNotifyBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QDebug>

#include "widgets/WizImageButton.h"
#include "utils/WizStyleHelper.h"



WizNotifyBar::WizNotifyBar(QWidget *parent)
    : QWidget(parent)
    , m_type(NoNotify)
    , m_childWgt(new QWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);    // On most platforms, the margin is 11 pixels in all directions.
    layout->setSpacing(0);
    setLayout(layout);

    m_spacer = new QLabel(this);
    m_spacer->setFixedHeight(2);
    m_spacer->setText(QString());
    layout->addWidget(m_spacer);

    //Q_OBJECT标志与stylesheet不能共存，创建一个子Widget来调整样式
    QHBoxLayout* childLayout = new QHBoxLayout();
    childLayout->setContentsMargins(12, 5, 12, 5);    // On most platforms, the margin is 11 pixels in all directions.
    childLayout->setSpacing(15);
    m_childWgt->setLayout(childLayout);
    layout->addWidget(m_childWgt);

    m_labelNotify = new QLabel(this);
    m_labelNotify->setAttribute(Qt::WA_NoSystemBackground, true);
    m_labelNotify->setAlignment(Qt::AlignVCenter);
    m_buttonCloseRed = new WizImageButton(this);
    m_buttonCloseRed->setMaximumSize(8, 8);
    m_buttonCloseRed->setIcon(Utils::WizStyleHelper::loadIcon("closeNotifyBarRed"));
    m_buttonCloseRed->setLockNormalStatus(true);
    m_buttonCloseBlue = new WizImageButton(this);
    m_buttonCloseBlue->setMaximumSize(8, 8);
    m_buttonCloseBlue->setIcon(Utils::WizStyleHelper::loadIcon("closeNotifyBarBlue"));
    m_buttonCloseBlue->setLockNormalStatus(true);
    childLayout->addWidget(m_labelNotify);
    childLayout->addStretch();
    childLayout->addWidget(m_buttonCloseRed);
    childLayout->addWidget(m_buttonCloseBlue);
    showCloseButton(false);

    connect(m_labelNotify, SIGNAL(linkActivated(QString)), SIGNAL(labelLink_clicked(QString)));
    connect(m_buttonCloseRed, SIGNAL(clicked()), SLOT(on_closeButton_Clicked()));
    connect(m_buttonCloseBlue, SIGNAL(clicked()), SLOT(on_closeButton_Clicked()));

    setMaximumHeight(0);
    m_animation = new QPropertyAnimation(this, "maximumHeight", this);

    applyStyleSheet(false);
}

void WizNotifyBar::showPermissionNotify(int type)
{
    if (m_type == type)
        return;

    hideNotify(false);
    setStyleForPermission();

    m_type = (NotifyType)type;
    switch (type) {
    case Locked:
        m_labelNotify->setText(QObject::tr("The note is locked and read only, press unlock button if you need edit."));
        showNotify();
        break;
    case Deleted:
        m_labelNotify->setText(QObject::tr("This note is deleted, You can edit after move to other folders."));
        showNotify();
        break;
    case PermissionLack:
        m_labelNotify->setText(QObject::tr("Your permission is not enough to edit this note."));
        showNotify();
        break;
    case LockForGruop:
//        setStyleForEditing();
//        m_labelNotify->setText(QObject::tr("Checking for the version of note, please wait for a second..."));
//        show();
        break;
    default:
        hideNotify(false);
        break;
    }
}

void WizNotifyBar::showMessageTips(Qt::TextFormat format, const QString& info)
{
    if (!info.isEmpty())
    {
        setStyleForEditing();
        m_labelNotify->setTextFormat(format);
        m_labelNotify->setText(info);
        showNotify();
        m_type = CustomMessage;
        m_spacer->show();
    }
    else
    {
        hideNotify(false);
    }
}

void WizNotifyBar::hideMessageTips(bool useAnimation)
{
    hideNotify(useAnimation);
}

void WizNotifyBar::on_closeButton_Clicked()
{
    hideNotify(true);
}

void WizNotifyBar::setStyleForPermission()
{        
    applyStyleSheet(true);
    showCloseButton(true);
}

void WizNotifyBar::setStyleForEditing()
{    
    applyStyleSheet(false);
    showCloseButton(false);
}

void WizNotifyBar::showNotify()
{
//    if (maximumHeight() > 0)
//        return;

    m_animation->stop();
    m_animation->setDuration(800);
    m_animation->setStartValue(maximumHeight());
    m_animation->setEndValue(Utils::WizStyleHelper::notifyBarHeight());
    m_animation->setEasingCurve(QEasingCurve::InExpo);

    m_animation->start();
}

void WizNotifyBar::hideNotify(bool bUseAnimation)
{
    m_spacer->hide();

    m_animation->stop();
    m_type = NoNotify;
    if (maximumHeight() > 0)
    {
        if (bUseAnimation)
        {
            m_animation->setDuration(400);
            m_animation->setStartValue(Utils::WizStyleHelper::notifyBarHeight());
            m_animation->setEndValue(0);
//            m_animation->setEasingCurve(QEasingCurve::InOutQuad);

            m_animation->start();
            return;
        }
        else
        {
            setMaximumHeight(0);
            return;
        }
    }
}

void WizNotifyBar::applyStyleSheet(bool isForbidden)
{
    QString styleSheet;
    if (isForbidden)
    {
        styleSheet = ".QWidget{border:1px solid #E84C3D; border-radius:2px; background-color:#FADBD8;}"
                     "QLabel {font:11px; color:#E84C3D;}";

    }
    else
    {
        styleSheet = ".QWidget{border:1px solid #FF9C00; border-radius:2px; background-color:#FFF6D7;}"
                     "QLabel {font:11px; color:#FF9C00;}";
    }
    m_childWgt->setStyleSheet(styleSheet);
    return;

}

void WizNotifyBar::showCloseButton(bool isForbidden)
{
    m_buttonCloseBlue->setVisible(!isForbidden);
    m_buttonCloseRed->setVisible(isForbidden);
}
