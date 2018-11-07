#include "WizExecutingActionDialog.h"
#include "ui_WizExecutingActionDialog.h"

#include "share/WizMisc.h"
#include "share/WizThreads.h"
#include "utils/WizStyleHelper.h"

#include <QMovie>


WizExecutingActionDialog::WizExecutingActionDialog(QString description, int threadId, std::function<void(void)> fun, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizExecutingActionDialog)
    , m_threadId(threadId)
    , m_fun(fun)
    , m_first(true)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setPalette(QPalette(QColor(0, 0, 0)));
    setFixedSize(QSize(WizSmartScaleUI(180), WizSmartScaleUI(150)));
    //
    ui->setupUi(this);
    //
    QString strThemeName = Utils::WizStyleHelper::themeName();
    QString fileName = ::WizGetSkinResourceFileName(strThemeName, "executing_action.gif");
    QMovie *movie = new QMovie(fileName);
    movie->setScaledSize(QSize(WizSmartScaleUI(50), WizSmartScaleUI(50)));
    ui->movie->setMovie(movie);
    movie->start();
    //
    if (description.isEmpty())
    {
        ui->actionDescription->setVisible(false);
    }
    else
    {
        ui->actionDescription->setText(description);
        ui->actionDescription->setStyleSheet("QLabel{color:#ffffff}");
    }
}

WizExecutingActionDialog::~WizExecutingActionDialog()
{
    delete ui;
}

void WizExecutingActionDialog::reject()
{

}

void WizExecutingActionDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    if (!m_first)
        return;
    m_first = false;
    //
    //
    ::WizExecuteOnThread(m_threadId, [=]{
        //
        m_fun();
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            //
            QDialog::reject();
            //
        });
    });
}

void WizExecutingActionDialog::executeAction(QString description, int threadId, std::function<void(void)> fun)
{
    WizExecutingActionDialog dlg(description, threadId, fun);
    dlg.exec();
}

