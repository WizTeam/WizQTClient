#include "WizUserCipherForm.h"
#include "ui_WizUserCipherForm.h"

#include <QGraphicsDropShadowEffect>

#include "WizMainWindow.h"
#include "utils/WizStyleHelper.h"


WizUserCipherForm::WizUserCipherForm(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WizUserCipherForm)
    , m_app(app)
    , m_bSaveForSession(false)
{
#ifndef Q_OS_MAC
    setStyleSheet(QString("color:#a6a6a6;font-size:%1px").arg(WizSmartScaleUI(14)));
#endif
    ui->setupUi(this);
    ui->editUserCipher->setEchoMode(QLineEdit::Password);
    ui->editUserCipher->setAttribute(Qt::WA_MacShowFocusRect, false);

    setAutoFillBackground(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setBackgroundRole(QPalette::Midlight);
    //
    ui->checkSave->setVisible(false);


    QPixmap strIconNormal = Utils::WizStyleHelper::loadPixmap("mac_icons_password_done");
    QPixmap strIconHot = Utils::WizStyleHelper::loadPixmap("mac_icons_password_done_hot");
    QPixmap strIconDown = Utils::WizStyleHelper::loadPixmap("mac_icons_password_done_down");

    ui->editUserCipher->setFixedHeight(WizSmartScaleUI(19));
    QSize szBtn(WizSmartScaleUI(19), WizSmartScaleUI(19));
    ui->buttonOk->setMinimumSize(szBtn);
    ui->buttonOk->setMaximumSize(szBtn);
    ui->buttonOk->setIconNormal(strIconNormal);
    ui->buttonOk->setIconHot(strIconHot);
    ui->buttonOk->setIconDown(strIconDown);
    ui->buttonOk->setStatusNormal();
    ui->buttonOk->setLockNormalStatus(true);


    m_animation = new QPropertyAnimation(ui->editUserCipher, "pos", this);

    connect(ui->editUserCipher, SIGNAL(returnPressed()), SLOT(onButtonOK_clicked()));
    connect(ui->editUserCipher, SIGNAL(textChanged(const QString&)),
            SLOT(onCipher_changed(const QString&)));
    connect(ui->buttonOk, SIGNAL(clicked()), SLOT(onButtonOK_clicked()));
    connect(ui->checkSave, SIGNAL(stateChanged(int)), SLOT(onCheckSave_stateChanged(int)));

}

QSize WizUserCipherForm::sizeHint()
{
    return QSize(WizSmartScaleUI(350), WizSmartScaleUI(120));
}

void WizUserCipherForm::showEvent(QShowEvent* event)
{
    Q_UNUSED(event);

    // clear state set by last time
    m_bSaveForSession = false;
    m_userCipher.clear();
    ui->editUserCipher->clear();
    ui->checkSave->setChecked(false);

}

/*
void CWizUserCipherForm::sheetShow()
{
#if 0
    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());

    int x = (mainWindow->clientSize().width() - sizeHint().width()) / 2;

    const QPoint& startP = mainWindow->client()->mapTo(mainWindow, QPoint(x, -sizeHint().height()));
    const QPoint& endP = mainWindow->client()->mapTo(mainWindow, QPoint(x, 0));

    QPropertyAnimation *animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(250);
    animation->setStartValue(startP);
    animation->setEndValue(endP);
    animation->start();
#endif

    MainWindow* mainWindow = qobject_cast<MainWindow *>(m_app.mainWindow());
    const QPoint& topLeft = mainWindow->client()->mapTo(mainWindow, QPoint(-1,-1));
    setGeometry(topLeft.x(), topLeft.y(), mainWindow->clientSize().width() + 1, mainWindow->clientSize().height() + 1);
    show();

    ui->editUserCipher->setFocus(Qt::MouseFocusReason);
}*/

void WizUserCipherForm::cipherError()
{
    QPoint pos = ui->editUserCipher->pos();
    m_animation->setDuration(500);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setStartValue(QPoint(pos.x() + WizSmartScaleUI(5), pos.y()));
    m_animation->setEndValue(QPoint(pos.x() - WizSmartScaleUI(5), pos.y()));
    m_animation->start();

    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setStartValue(QPoint(pos.x() - WizSmartScaleUI(5), pos.y()));
    m_animation->setEndValue(pos);
    m_animation->start();

    ui->editUserCipher->setGeometry(QRect(pos, ui->editUserCipher->size()));
    ui->editUserCipher->selectAll();
    ui->editUserCipher->setFocus();
}

void WizUserCipherForm::cipherCorrect()
{
    ui->editUserCipher->setText(QString());
    ui->checkSave->setCheckState(Qt::Unchecked);
}

void WizUserCipherForm::onButtonOK_clicked()
{
    if (!ui->editUserCipher->text().isEmpty()) {
        m_userCipher = ui->editUserCipher->text();

        emit cipherCheckRequest();
    }
}

void WizUserCipherForm::onCheckSave_stateChanged(int state)
{
    if (state == Qt::Checked) {
        m_bSaveForSession = true;
    } else {
        m_bSaveForSession = false;
    }
}

void WizUserCipherForm::onCipher_changed(const QString &text)
{
    if (text.isEmpty()) {
        ui->buttonOk->setStatusNormal();
        ui->buttonOk->setLockNormalStatus(true);
    } else {
        ui->buttonOk->setLockNormalStatus(false);
        ui->buttonOk->setStatusHot();
    }
}

void WizUserCipherForm::setPasswordText(const QString &strPasswordText)
{
    ui->labelPassword->setText(strPasswordText);
}

void WizUserCipherForm::setHint(const QString& strHint)
{
    if (strHint.isEmpty()) {
        ui->labelHint->setVisible(false);
        ui->label_2->setVisible(false);
    } else {
        ui->labelHint->setText(strHint);
        ui->labelHint->setVisible(true);
        ui->label_2->setVisible(true);
    }
}

void WizUserCipherForm::setCipherEditorFocus()
{
    ui->editUserCipher->setFocus();
}
