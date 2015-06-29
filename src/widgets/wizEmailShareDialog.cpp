#include "wizEmailShareDialog.h"
#include "ui_wizEmailShareDialog.h"
#include <QListWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QMessageBox>
#include <QTextCodec>
#include <QPixmap>
#include <QVBoxLayout>
#include <QDebug>

#include "sync/apientry.h"
#include "sync/token.h"
#include "share/wizsettings.h"
#include "share/wizDatabaseManager.h"
#include "rapidjson/document.h"
#include "utils/stylehelper.h"

#define EMAIL_CONTACTS "EMAILCONTACTS"

enum returnCode {
    codeOK = 200,                   //:ok,
    codeErrorParam = 322,      //:参数错误，
    codeErrorFile = 3330,         //: 笔记不存在，
    codeErrorSize = 3370,        //:邮件大小超出尺寸，
    codeErrorEncrypt = 3371,   //:加密笔记无法分享，
    codeErrorEmail = 3372,      //：邮箱未验证，无法发送分享邮件，
    codeErrorServer = 500           //:服务器错误
};

CWizEmailShareDialog::CWizEmailShareDialog(CWizExplorerApp& app, QWidget *parent) :
    m_app(app),
    QDialog(parent),
    ui(new Ui::CWizEmailShareDialog)
{
    ui->setupUi(this);
    ui->checkBox_saveNotes->setVisible(false);
    QPixmap pix(Utils::StyleHelper::skinResourceFileName("send_email"));
    QIcon icon(Utils::StyleHelper::skinResourceFileName("send_email"));
    ui->toolButton_send->setIcon(icon);
    ui->toolButton_send->setIconSize(pix.size());
    ui->toolButton_contacts->setIcon(QIcon(Utils::StyleHelper::skinResourceFileName("document_badge")));
    ui->toolButton_contacts->setToolTip(tr("Show frequent contacts list"));

    m_contactDialog = new QDialog(this);
    m_contactDialog->setWindowTitle(tr("Frequent Contacts List"));
    QVBoxLayout* layout = new QVBoxLayout(m_contactDialog);
    m_contactList = new QListWidget(m_contactDialog);
    m_contactList->setAttribute(Qt::WA_MacShowFocusRect, 0);
    layout->addWidget(m_contactList);
    connect(m_contactList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            SLOT(on_contactsListItemClicked(QListWidgetItem*)));
}

CWizEmailShareDialog::~CWizEmailShareDialog()
{
    delete ui;
}

void CWizEmailShareDialog::setNote(const WIZDOCUMENTDATA& note, const QString& sendTo)
{
    m_note = note;
    ui->lineEdit_subject->setText(m_note.strTitle);
    ui->lineEdit_to->setText(sendTo);
    ui->comboBox_replyTo->insertItem(0, m_app.userSettings().user());
    ui->comboBox_replyTo->insertItem(1, m_app.userSettings().myWizMail());
}

void CWizEmailShareDialog::on_toolButton_send_clicked()
{
    Q_ASSERT(!m_note.strGUID.isEmpty());

    QString strToken = WizService::Token::token();
    QString strKS = WizService::ApiEntry::kUrlFromGuid(strToken, m_note.strKbGUID);
    QString strExInfo = getExInfo();
    QString strUrl = WizService::ApiEntry::mailShareUrl(strKS, strExInfo);
//    strUrl += strExInfo;

//    QUrl url(strKS);
//    strUrl.remove("http://{ks_host}");
//    strUrl = url.scheme() + "://" + url.host() + strUrl;



    QNetworkAccessManager net;
    QNetworkReply* reply = net.get(QNetworkRequest(strUrl));

//    QEventLoop loop;
    QMessageBox msgBox(this);
    msgBox.setText(tr("Sending..."));
    msgBox.setWindowTitle(tr("Info"));
    connect(reply, SIGNAL(finished()), &msgBox, SLOT(accept()));
    msgBox.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::information(this, tr("Info"), reply->errorString());
        reply->deleteLater();
        return;
    }

    QString strReply = QString::fromUtf8(reply->readAll());
    int nCode;
    QString returnMessage;
    processReturnMessage(strReply, nCode, returnMessage);
    mailShareFinished(nCode, returnMessage);

    reply->deleteLater();
}

QString CWizEmailShareDialog::getExInfo()
{
//    kb_guid
//    document_guid
//    token
//    mail_to            收件人，标准的邮箱格式 `"name" <xxx@xxx.com>` 或者 `xxx@xxx.com`,可以指定多个收件人
//    subject            nullable, 邮件主题，如果为空，则取笔记标题
//    cc_to_self         nullable, 是否抄送自己，, 字符串的 true or false, 默认false
//    reply_to           nullable, 可为空，不为空时要求为标准格式
//    note
//    api_version        4

    QString strToken = WizService::Token::token();
    QString cc_to_self = ui->checkBox_sendMeCopy->isChecked() ? "true" : "false";
    QString result = "?kb_guid=" + m_note.strKbGUID + "&document_guid=" + m_note.strGUID + "&token=" + strToken
            + "&mail_to=" + ui->lineEdit_to->text() + "&subject=" + ui->lineEdit_subject->text() + "&cc_to_self=" + cc_to_self
            + "&reply_to=" + ui->comboBox_replyTo->currentText() + "&note=" + ui->textEdit_notes->toPlainText()
            + "&api_version=4";

    return result;
}

void CWizEmailShareDialog::mailShareFinished(int nCode, const QString& returnMessage)
{
    switch (nCode) {
    case codeOK:
        accept();
        break;
    case codeErrorParam:
    case codeErrorFile:
    case codeErrorSize:
    case codeErrorEncrypt:
    case codeErrorEmail:
    case codeErrorServer:
        QMessageBox::warning(this, tr("Info"), returnMessage);
        break;
    default:
        QMessageBox::warning(this, tr("Info"), tr("Unkown error."));
        break;
    }
}

void CWizEmailShareDialog::processReturnMessage(const QString& returnMessage, int& nCode, QString& message)
{
    rapidjson::Document d;
    d.Parse<0>(returnMessage.toUtf8().constData());

    if (d.FindMember("error_code")) {
        qDebug() << QString::fromUtf8(d.FindMember("error")->value.GetString());
        return;
    }

    if (d.FindMember("return_code")) {
        nCode = d.FindMember("return_code")->value.GetInt();
        if (nCode == 200) {
            qDebug() <<"[EmailShar]:send email successed!";
            saveContacts();
            return;
        } else {
            message = QString::fromUtf8(d.FindMember("return_message")->value.GetString());
            qDebug() << message << ", code = " << nCode;
            return;
        }
    }
}

void CWizEmailShareDialog::saveContacts()
{
    QString strText = ui->lineEdit_to->text();
    QStringList toList = strText.split(QRegExp(",|;"));
    QString strContact = m_app.userSettings().get(EMAIL_CONTACTS);
    foreach (QString str, toList)
    {
        if (!strContact.contains(str))
            strContact.append(str + ";");
    }
    m_app.userSettings().set(EMAIL_CONTACTS, strContact);
}

void CWizEmailShareDialog::updateContactList()
{
    m_contactList->clear();
    QString strContact = m_app.userSettings().get(EMAIL_CONTACTS);
    QStringList contactList = strContact.split(';');
    foreach(QString str, contactList)
    {
        m_contactList->addItem(str);
    }
}

void CWizEmailShareDialog::on_toolButton_contacts_clicked()
{
    updateContactList();
    m_contactDialog->exec();
    return;
}

void CWizEmailShareDialog::on_contactsListItemClicked(QListWidgetItem *item)
{
    QString strUserName = item->text();
    QString strTo = ui->lineEdit_to->text();
    if (!strTo.isEmpty() && strTo.right(1) != "," && strTo.right(1) != ";")
    {
        strTo.append(";");
    }
    strTo += strUserName + ";";
    ui->lineEdit_to->setText(strTo);
}
