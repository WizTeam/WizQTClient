#include "wizdownloadobjectdatadialog.h"
#include "ui_wizdownloadobjectdatadialog.h"

#include <QMessageBox>

WizDownloadObjectDataDialog::WizDownloadObjectDataDialog(CWizDatabase& db, \
                                                         const CString& strAccountsApiURL, \
                                                         const WIZOBJECTDATA& data, \
                                                         QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizDownloadObjectDataDialog)
    , m_data(data)
    , m_downloader(db, strAccountsApiURL, data)
    , m_bUserCanceled(false)
{
    ui->setupUi(this);

    ui->labelDownloading->setText(ui->labelDownloading->text().arg(m_data.strDisplayName));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);

    m_downloader.startDownload();

    connect(&m_downloader, SIGNAL(processLog(const QString&)), SLOT(downloader_processLog(const QString&)));
    connect(&m_downloader, SIGNAL(progressChanged(int)), SLOT(downloader_progress(int)));
    connect(&m_downloader, SIGNAL(downloadDone(bool)), SLOT(downloader_done(bool)));

    setFixedSize(size());
}

WizDownloadObjectDataDialog::~WizDownloadObjectDataDialog()
{
    m_downloader.abort();
    delete ui;
}

void WizDownloadObjectDataDialog::downloader_processLog(const QString& msg)
{
    TOLOG(msg);
}

void WizDownloadObjectDataDialog::downloader_progress(int pos)
{
    ui->progressBar->setValue(pos);
}

void WizDownloadObjectDataDialog::reject()
{
    m_bUserCanceled = true;
    m_downloader.abort();

    QDialog::reject();
}

void WizDownloadObjectDataDialog::downloader_done(bool succeeded)
{
    if (succeeded)
    {
        accept();
    }
    else
    {
        QDialog::reject();
    }
}

bool WizDownloadObjectDataDialog::downloadObjectData(CWizDatabase& db, const CString& strAccountsApiURL, const WIZOBJECTDATA& data, QWidget* parent)
{
    WizDownloadObjectDataDialog dlg(db, strAccountsApiURL, data, parent);
    bool ret = QDialog::Accepted == dlg.exec();
    if (!ret && !dlg.m_bUserCanceled)
    {
        QString strMsg = QString(tr("Failed to download data!\n\n%1")).arg(dlg.m_slError.join("\n"));
        QMessageBox::critical(parent, parent->windowTitle(), strMsg);
    }

    return ret;
}
