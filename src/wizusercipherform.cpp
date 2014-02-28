#include "wizusercipherform.h"
#include "ui_wizusercipherform.h"

#include <QGraphicsDropShadowEffect>

#include "wizmainwindow.h"

using namespace Core::Internal;

CWizUserCipherForm::CWizUserCipherForm(CWizExplorerApp& app, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CWizUserCipherForm)
    , m_app(app)
    , m_bSaveForSession(false)
{
    ui->setupUi(this);
    this->setFixedSize(350, 140);
    ui->editUserCipher->setEchoMode(QLineEdit::Password);

    m_animation = new QPropertyAnimation(ui->editUserCipher, "pos");

    connect(ui->buttonOk, SIGNAL(clicked()), SLOT(onButtonOK_clicked()));
    connect(ui->checkSave, SIGNAL(stateChanged(int)), SLOT(onCheckSave_stateChanged(int)));

}

CWizUserCipherForm::~CWizUserCipherForm()
{
    delete ui;

    if (NULL != m_animation){
        delete m_animation;
    }
}

QSize CWizUserCipherForm::sizeHint()
{
    return QSize(350, 120);
}

void CWizUserCipherForm::showEvent(QShowEvent* event)
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

void CWizUserCipherForm::cipherError()
{
    QPoint pos = ui->editUserCipher->pos();
    m_animation->setDuration(500);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setStartValue(QPoint(pos.x() + 5, pos.y()));
    m_animation->setEndValue(QPoint(pos.x() - 5, pos.y()));
    m_animation->start();

    m_animation->setDuration(250);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setStartValue(QPoint(pos.x() - 5, pos.y()));
    m_animation->setEndValue(pos);
    m_animation->start();

    ui->editUserCipher->selectAll();
}

void CWizUserCipherForm::cipherCorrect()
{
    ui->editUserCipher->setText(QString());
    ui->checkSave->setCheckState(Qt::Unchecked);
    this->hide();
}

void CWizUserCipherForm::onButtonOK_clicked()
{
    m_userCipher = ui->editUserCipher->text();

    emit cipherCheckRequest();
}

void CWizUserCipherForm::onCheckSave_stateChanged(int state)
{
    if (state == Qt::Checked) {
        m_bSaveForSession = true;
    } else {
        m_bSaveForSession = false;
    }
}

void CWizUserCipherForm::setHint(const QString& strHint)
{
    ui->labelHint->setText(strHint);
}
