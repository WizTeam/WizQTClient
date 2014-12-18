#include "notifybar.h"

#include <QHBoxLayout>
#include <QLabel>

#include "widgets/wizImageButton.h"
#include "utils/stylehelper.h"

using namespace Core::Internal;

NotifyBar::NotifyBar(QWidget *parent)
    : QWidget(parent)
{
    //setStyleSheet("* {font-size:12px; color: #FFFFFF;} *:active {background: url(:/notify_bg.png);} *:!active {background: url(:/notify_bg_inactive.png);}");
    setFixedHeight(Utils::StyleHelper::notifyBarHeight());
    setAutoFillBackground(true);
    QPalette paletteBG(palette());
    paletteBG.setBrush(QPalette::Window, QBrush("#F6F3D3"));
    paletteBG.setColor(QPalette::Text, QColor("#003348"));
    setPalette(paletteBG);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(8, 5, 23, 5);    // On most platforms, the margin is 11 pixels in all directions.
    layout->setSpacing(15);
    setLayout(layout);

    m_labelNotify = new QLabel(this);
    m_labelNotify->setAttribute(Qt::WA_NoSystemBackground, true);
    m_labelNotify->setAlignment(Qt::AlignVCenter);
    m_buttonClose = new wizImageButton(this);
    m_buttonClose->setMaximumSize(16, 16);
    m_buttonClose->setIcon(Utils::StyleHelper::loadIcon("closeNotifyBar"));
    m_buttonClose->setLockNormalStatus(true);
    layout->addWidget(m_labelNotify);
    layout->addStretch();
    layout->addWidget(m_buttonClose);

    connect(m_buttonClose, SIGNAL(clicked()), SLOT(on_closeButton_Clicked()));
}

void NotifyBar::showPermissionNotify(int type)
{
    setStyleForPermission();

    switch (type) {
    case Locked:
        m_labelNotify->setText(QObject::tr("The note is locked and read only, press unlock button if you need edit."));
        show();
        break;
    case Deleted:
        m_labelNotify->setText(QObject::tr("This note is deleted, You can edit after move to other folders."));
        show();
        break;
    case PermissionLack:
        m_labelNotify->setText(QObject::tr("Your permission is not enough to edit this note."));
        show();
        break;
    case LockForGruop:
        setStyleForEditing();
        m_labelNotify->setText(QObject::tr("Checking for the version of note, please wait for a second..."));
        show();
        break;
    default:
        hide();
    }
}

void NotifyBar::showEditingNotify(const QString &editor)
{
    if (!editor.isEmpty())
    {
        setStyleForEditing();
        m_labelNotify->setText(QString(tr("This note is editing by %1 .")).arg(editor));
        show();
    }
    else
    {
        hide();
    }
}

void NotifyBar::on_closeButton_Clicked()
{
    hide();
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
