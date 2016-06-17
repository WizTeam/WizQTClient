#include "wizexecutingactiondialog.h"
#include "ui_wizexecutingactiondialog.h"

#include "share/wizmisc.h"
#include "share/wizthreads.h"
#include "utils/stylehelper.h"

#include <QMovie>


WizExecutingActionDialog::WizExecutingActionDialog(QString description, int threadId, std::function<void(void)> fun, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizExecutingActionDialog)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setPalette(QPalette(QColor(0, 0, 0)));
    //
    ui->setupUi(this);
    //
    QString strThemeName = Utils::StyleHelper::themeName();
    QString fileName = ::WizGetSkinResourceFileName(strThemeName, "executing_action");
    QMovie *movie = new QMovie(fileName);
    movie->setScaledSize(QSize(50, 50));
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
    //
    ::WizExecuteOnThread(threadId, [=]{
        //
        fun();
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
            //
            QDialog::reject();
            //
        });
    });
}

WizExecutingActionDialog::~WizExecutingActionDialog()
{
    delete ui;
}

void WizExecutingActionDialog::reject()
{

}

void WizExecutingActionDialog::executeAction(QString description, int threadId, std::function<void(void)> fun)
{
    WizExecutingActionDialog dlg(description, threadId, fun);
    dlg.exec();
}

