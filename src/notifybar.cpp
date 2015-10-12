#include "notifybar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QDebug>

#include "widgets/wizImageButton.h"
#include "utils/stylehelper.h"


using namespace Core::Internal;

NotifyBar::NotifyBar(QWidget *parent)
    : QWidget(parent)
    , m_type(NoNotify)
    , m_childWgt(new QWidget(this))
{
    //setStyleSheet("* {font-size:12px; color: #FFFFFF;} *:active {background: url(:/notify_bg.png);} *:!active {background: url(:/notify_bg_inactive.png);}");
//    setFixedHeight(Utils::StyleHelper::notifyBarHeight());
    setAutoFillBackground(true);
    applyStyleSheet(false);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);    // On most platforms, the margin is 11 pixels in all directions.
    layout->setSpacing(0);
    setLayout(layout);

    //Q_OBJECT标志与stylesheet不能共存，创建一个子Widget来调整样式
    QHBoxLayout* childLayout = new QHBoxLayout();
    childLayout->setContentsMargins(12, 5, 12, 5);    // On most platforms, the margin is 11 pixels in all directions.
    childLayout->setSpacing(15);
    m_childWgt->setLayout(childLayout);
    layout->addWidget(m_childWgt);

    m_labelNotify = new QLabel(this);
    m_labelNotify->setAttribute(Qt::WA_NoSystemBackground, true);
    m_labelNotify->setAlignment(Qt::AlignVCenter);
    m_buttonClose = new wizImageButton(this);
    m_buttonClose->setMaximumSize(16, 16);
    m_buttonClose->setIcon(Utils::StyleHelper::loadIcon("closeNotifyBar"));
    m_buttonClose->setLockNormalStatus(true);
    childLayout->addWidget(m_labelNotify);
    childLayout->addStretch();
    childLayout->addWidget(m_buttonClose);
    connect(m_labelNotify, SIGNAL(linkActivated(QString)), SIGNAL(labelLink_clicked(QString)));

    connect(m_buttonClose, SIGNAL(clicked()), SLOT(on_closeButton_Clicked()));

    setMaximumHeight(0);
    m_animation = new QPropertyAnimation(this, "maximumHeight", this);
}

void NotifyBar::showPermissionNotify(int type)
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

void NotifyBar::showMessageTips(Qt::TextFormat format, const QString& info)
{
    if (!info.isEmpty())
    {
        setStyleForEditing();
        m_labelNotify->setTextFormat(format);
        m_labelNotify->setText(info);
        showNotify();
        m_type = CustomMessage;
    }
    else
    {
        hideNotify(false);
    }
}

void NotifyBar::hideMessageTips(bool useAnimation)
{
    hideNotify(useAnimation);
}

void NotifyBar::on_closeButton_Clicked()
{
    hideNotify(true);
}

void NotifyBar::setStyleForPermission()
{
    QPalette paletteBG(palette());
    paletteBG.setBrush(QPalette::Window, QBrush("#F4BDBD"));
    setPalette(paletteBG);
}

void NotifyBar::setStyleForEditing()
{
    QPalette paletteBG(palette());
    paletteBG.setBrush(QPalette::Window, QBrush("#F6F3D3"));
    setPalette(paletteBG);
}

void NotifyBar::showNotify()
{
//    if (maximumHeight() > 0)
//        return;

    m_animation->stop();
    m_animation->setDuration(800);
    m_animation->setStartValue(maximumHeight());
    m_animation->setEndValue(Utils::StyleHelper::notifyBarHeight());
    m_animation->setEasingCurve(QEasingCurve::InExpo);

    m_animation->start();
}

void NotifyBar::hideNotify(bool bUseAnimation)
{
    m_animation->stop();
    m_type = NoNotify;
    if (maximumHeight() > 0)
    {
        if (bUseAnimation)
        {
            m_animation->setDuration(400);
            m_animation->setStartValue(Utils::StyleHelper::notifyBarHeight());
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

void NotifyBar::applyStyleSheet(bool isForbidden)
{
    QString styleSheet;
    if (isForbidden)
    {
        styleSheet = ".QWidget{border:1px solid #ABCDF3; border-radius:2px; background-color:#F5FAFD;}"
                     "QLabel {font:11px; color:#5990EF;}";
    }
    else
    {
        styleSheet = ".QWidget{border:1px solid #ABCDF3; border-radius:2px; background-color:#F5FAFD;}"
                     "QLabel {font:11px; color:#5990EF;}";
    }
    setStyleSheet(styleSheet);
    m_childWgt->setStyleSheet(styleSheet);

    return;

    QPalette paletteBG(palette());
    paletteBG.setBrush(QPalette::Window, QBrush("#F6F3D3"));
    paletteBG.setColor(QPalette::Text, QColor("#003348"));
    setPalette(paletteBG);
}
