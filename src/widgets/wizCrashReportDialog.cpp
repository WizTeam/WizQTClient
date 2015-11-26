#include "wizCrashReportDialog.h"
#include "ui_wizCrashReportDialog.h"

#ifdef Q_OS_MAC
#include <QUrlQuery>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QPlainTextEdit>
#include <QtConcurrentRun>
#include "wizdef.h"
#include "share/wizEventLoop.h"
#include "sync/apientry.h"
#include "sync/token.h"
#include "share/wizmisc.h"
#include "rapidjson/document.h"

CWizCrashReportDialog::CWizCrashReportDialog(const QString& text, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizCrashReportDialog)
    , m_reports(text)
    , m_textEdit(new QPlainTextEdit(this))
{
    ui->setupUi(this);

    m_textEdit->setVisible(false);
    m_textEdit->setReadOnly(true);
    layout()->addWidget(m_textEdit);
}

CWizCrashReportDialog::~CWizCrashReportDialog()
{
    delete ui;
}

void CWizCrashReportDialog::on_btn_yes_clicked()
{
    accept();

    QString text = m_reports;
    QtConcurrent::run([text](){
        QString reportText = text;
        const int maxSize = 1024 * 128 - 100;
        if (reportText.size() > maxSize)
        {
            reportText.truncate(maxSize);
            reportText.append("\n Wow! too may words, truncated!");
        }

        //
        QString url = WizService::WizApiEntry::crashReportUrl();
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded");

        QUrlQuery postData;
        postData.addQueryItem("platform", "mac");
        postData.addQueryItem("token", "");
        postData.addQueryItem("subject", "Version: " + QString(WIZ_CLIENT_VERSION) + "  UUID : " + WizGenGUIDLowerCaseLetterOnly());
        postData.addQueryItem("error", reportText);

        QNetworkAccessManager net;
        QNetworkReply* reply = net.post(request, postData.toString(QUrl::FullyEncoded).toUtf8());

        CWizAutoTimeOutEventLoop loop(reply);
        loop.exec();

        if (loop.error() != QNetworkReply::NoError)
        {
            qDebug() << "[Crash report]Upload failed!";
            return;
        }

        rapidjson::Document d;
        d.Parse<0>(loop.result().toUtf8().constData());

        if (!d.HasMember("return_code"))
        {
            qDebug() << "[Crash report]Can not get return code ";
            return;
        }

        int returnCode = d.FindMember("return_code")->value.GetInt();
        if (returnCode != 200)
        {
            qDebug() << "[Crash report]Return code was not 200, error :  " << returnCode << loop.result();
            return;
        }
        else
        {
            qDebug() << "[Crash report]Upload OK";
        }

    });
}

void CWizCrashReportDialog::on_btn_no_clicked()
{
    reject();
}

void CWizCrashReportDialog::on_btn_details_clicked()
{
    m_textEdit->setVisible(!m_textEdit->isVisible());
    if (m_textEdit->toPlainText().isEmpty())
    {
        m_textEdit->insertPlainText(m_reports);
    }

    if (m_textEdit->isVisible())
    {
        setMaximumHeight(QWIDGETSIZE_MAX);
        resize(width(), 450);
    }
    else
    {
        setMaximumHeight(100);
        resize(width(), 100);
    }

}

#endif
