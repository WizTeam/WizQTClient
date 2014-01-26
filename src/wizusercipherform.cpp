#include "wizusercipherform.h"
#include "ui_wizusercipherform.h"

#include <QGraphicsDropShadowEffect>

#include "wizmainwindow.h"

using namespace Core::Internal;

CWizUserCipherForm::CWizUserCipherForm(CWizExplorerApp& app, QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::CWizUserCipherForm)
    , m_app(app)
    , m_bSaveForSession(false)
{
    ui->setupUi(this);

    ui->editUserCipher->setEchoMode(QLineEdit::Password);

    setFrameShadow(QFrame::Raised);
    setFrameShape(QFrame::StyledPanel);
    setAutoFillBackground(true);
    setAttribute(Qt::WA_MacShowFocusRect, true);

    connect(ui->buttonOk, SIGNAL(clicked()), SLOT(accept()));
    connect(ui->editUserCipher, SIGNAL(returnPressed()), ui->buttonOk, SIGNAL(clicked()));
    connect(ui->checkSave, SIGNAL(stateChanged(int)), SLOT(onCheckSave_stateChanged(int)));
}

CWizUserCipherForm::~CWizUserCipherForm()
{
    delete ui;
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

    raise();
}

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
    int x = (mainWindow->clientSize().width() - sizeHint().width()) / 2;
    const QPoint& endP = mainWindow->client()->mapTo(mainWindow, QPoint(x, 0));
    setGeometry(endP.x(), endP.y(), sizeHint().width(), sizeHint().height());
    show();

    ui->editUserCipher->setFocus(Qt::MouseFocusReason);
}

void CWizUserCipherForm::accept()
{
    m_userCipher = ui->editUserCipher->text();
    ui->editUserCipher->clear();

    hide();

    emit accepted();
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
