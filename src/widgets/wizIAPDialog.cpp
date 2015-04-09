#include "wizIAPDialog.h"
#include "ui_wizIAPDialog.h"
#include "mac/wizIAPHelper.h"
#include "sync/token.h"
#include "sync/apientry.h"
#include "utils/stylehelper.h"
#include "share/wizmisc.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include <QMessageBox>
#include <QWebView>
#include <QTextBrowser>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QDebug>

#define WIZ_PRODUCT_MONTH "cn.wiz.wiznote.mac.pro.monthly"
#define WIZ_PRODUCT_YEAR "cn.wiz.wiznote.mac.pro.yearly"

CWizIAPDialog::CWizIAPDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizIAPDialog),
    m_waitingMsgBox(new QMessageBox(this)),
    m_iAPhelper(0)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(1);
    hideInfoLabel();
    setPurchaseAvailable(false);

    initStyles();

    connect(&m_timer, SIGNAL(timeout()), SLOT(onWaitingTimeOut()));
    QTimer::singleShot(100, this, SLOT(loadProducts()));
}

CWizIAPDialog::~CWizIAPDialog()
{
    delete ui;
}

void CWizIAPDialog::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    m_timer.stop();
    setPurchaseAvailable(true);
    foreach (CWizIAPProduct product, productList)
    {
        qDebug() << product.id << product.localizedTitle << product.localizedPrice << product.localizedDescription;
        if (product.id == WIZ_PRODUCT_MONTH)
        {
            ui->btn_month->setText(QString(tr("%1 / month")).arg(product.localizedPrice));
        }
        else if (product.id == WIZ_PRODUCT_YEAR)
        {
            ui->btn_year->setText(QString(tr("%1 / year")).arg(product.localizedPrice));
        }
    }
}

void CWizIAPDialog::onPurchaseFinished(bool ok, const QString& receipt)
{
    qDebug() << "pruchage finished : " << ok << receipt;
    m_waitingMsgBox->done(ok);

    ui->label_success->setVisible(ok);
    ui->label_failed->setVisible(!ok);

    if (ok)
    {
        checkReceiptInfo(receipt);
    }
}

void CWizIAPDialog::initStyles()
{
    QString strThemeName = Utils::StyleHelper::themeName();

    ui->btn_goBack->setText("");
    QString strBack= ::WizGetSkinResourceFileName(strThemeName, "iap_back");
    QString strBackOn = ::WizGetSkinResourceFileName(strThemeName, "iap_back_on");
    ui->btn_goBack->setStyleSheet(QString("QToolButton {border:none; background-image: url(%1);"
                                          "background-position: center; background-repeat: no-repeat} QToolButton:pressed {background-image: url(%2); }")
                                  .arg(strBack).arg(strBackOn));
    ui->label_headTitle->setStyleSheet(QString("QLabel {border: none;background: none;"
                                               "font:16px #5e5e5e; padding-right: 70px;}"));

    ui->btn_month->setStyleSheet(QString("QPushButton { border: 1px; border-radius: 4px; background-color: #48a4e0; "
                                          "color: #ffffff;  font:14px; } QPushButton:pressed {background-color: #227dbf; }"));
    ui->btn_month->setCursor(Qt::PointingHandCursor);

    ui->btn_year->setStyleSheet(QString("QPushButton { border: 1px; border-radius: 4px; background-color: #48a4e0; "
                                          "color: #ffffff;  font:14px; } QPushButton:pressed {background-color: #227dbf; }"));
    ui->btn_year->setCursor(Qt::PointingHandCursor);

    QString strIcon1 = ::WizGetSkinResourceFileName(strThemeName, "iap_huifubanben");
    ui->label_text1->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 140px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon1));
    QString strIcon2 = ::WizGetSkinResourceFileName(strThemeName, "iap_note");
    ui->label_text2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon2));
    QString strIcon3 = ::WizGetSkinResourceFileName(strThemeName, "iap_liuliang");
    ui->label_text3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 140px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon3));
    QString strIcon4 = ::WizGetSkinResourceFileName(strThemeName, "iap_user");
    ui->label_text4->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon4));
}

void CWizIAPDialog::createIAPHelper()
{
    if (m_iAPhelper == 0)
        m_iAPhelper = new CWizIAPHelper(this);
}

void CWizIAPDialog::setPurchaseAvailable(bool b)
{
    ui->label_downloading->setVisible(!b);
    ui->btn_month->setEnabled(b);
    ui->btn_year->setEnabled(b);
}

void CWizIAPDialog::hideInfoLabel()
{
    ui->label_downloading->setVisible(false);
    ui->label_success->setVisible(false);
    ui->label_failed->setVisible(false);
}

void CWizIAPDialog::checkReceiptInfo(const QString& receipt)
{
    QString strPlat;
#ifdef Q_OS_MAC
    strPlat = "macosx";
#else
    strPlat = "linux";
#endif
    QString asServerUrl = WizService::ApiEntry::asServerUrl();
    QString checkUrl = asServerUrl + "/a/pay2/ios";
    CWizDatabase& db = CWizDatabaseManager::instance()->db();
    QString userID = db.GetUserId();
    QString userGUID = db.GetUserGUID();
    QString receiptBase64(receipt.toUtf8().toBase64());
    QString strExtInfo = QString("client_type=%2&user_id=%3&user_guid=%4&receipt=%5")
            .arg(strPlat).arg(userID).arg(userGUID).arg(receiptBase64);


//    checkUrl = checkUrl + "?" + strExtInfo;
    qDebug() << "check receipt : " << checkUrl << strExtInfo;

    QNetworkAccessManager net;
    QNetworkRequest request;
    request.setUrl(checkUrl);
//    QNetworkReply* reply = net.get(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
    QNetworkReply* reply = net.post(request, strExtInfo.toUtf8());

    QEventLoop loop;
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "setdata error;" ;
        reply->deleteLater();
        return;
    }

    qDebug() << "reply from server : " << reply->readAll();
    reply->deleteLater();

}

void CWizIAPDialog::on_btn_goBack_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    QString extInfo = WizService::ApiEntry::appstoreParam(false);
    QString strToken = WizService::Token::token();
    QString strUrl = WizService::ApiEntry::standardCommandUrl("user_info", strToken, extInfo);
    ui->webView->load(QUrl(strUrl));
}

void CWizIAPDialog::on_btn_month_clicked()
{
    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText("Waiting for AppStore...");
    m_waitingMsgBox->open();

    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_MONTH);
}

void CWizIAPDialog::on_btn_year_clicked()
{
    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText("Waiting for AppStore...");
    m_waitingMsgBox->open();

    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_YEAR);
}

void CWizIAPDialog::loadProducts()
{
    if (m_iAPhelper == 0)
        m_iAPhelper = new CWizIAPHelper(this);

    m_iAPhelper->requestProducts();
    m_timer.start(2 * 60 * 1000);
}

void CWizIAPDialog::onWaitingTimeOut()
{
    m_timer.stop();
    m_waitingMsgBox->close();
//    m_waitingMsgBox->setModal(false);
    m_waitingMsgBox->setStandardButtons(QMessageBox::Ok);
    m_waitingMsgBox->setText("Can not connect to AppStore, please try again later.");
    m_waitingMsgBox->exec();
    accept();
}
