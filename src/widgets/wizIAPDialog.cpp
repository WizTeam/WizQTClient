#include "wizIAPDialog.h"
#include "ui_wizIAPDialog.h"
#include <QMessageBox>
#include <QWebView>
#include <QTextBrowser>
#include <QWebFrame>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#if QT_VERSION > 0x050000
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif
#include <QUrl>
#include <QDebug>

#if defined Q_OS_MAC
#include "mac/wizIAPHelper.h"
#include "sync/token.h"
#include "sync/apientry.h"
#include "utils/stylehelper.h"
#include "share/wizmisc.h"
#include "share/wizMessageBox.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "wizmainwindow.h"
#include "coreplugin/icore.h"
#include "rapidjson/document.h"

#define WIZ_PRODUCT_MONTH "cn.wiz.wiznote.mac.pro.monthly"
#define WIZ_PRODUCT_YEAR "cn.wiz.wiznote.mac.pro.yearly"

#define APPSTORE_IAP "APPSTORE_IAP"
#define APPSTORE_UNFINISHEDTRANSATION  "UNFINISHED_TRANSATION"

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
    disconnect(this, SIGNAL(checkReceiptRequest(QByteArray,QString)),
               this, SLOT(onCheckReceiptRequest(QByteArray,QString)));
    connect(this, SIGNAL(checkReceiptRequest(QByteArray,QString)),
            SLOT(onCheckReceiptRequest(QByteArray,QString)), Qt::QueuedConnection);
    connect(ui->webView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
            SLOT(onEditorPopulateJavaScriptWindowObject()));
}

CWizIAPDialog::~CWizIAPDialog()
{
    delete ui;
}

void CWizIAPDialog::onProductsLoaded(const QList<CWizIAPProduct>& productList)
{
    QTimer::singleShot(0, this, SLOT(stopWaitTimer()));
    setPurchaseAvailable(true);
    foreach (CWizIAPProduct product, productList)
    {
        if (product.id == WIZ_PRODUCT_MONTH)
        {
            ui->btn_month->setText(QString(tr("%1 / month")).arg(product.localizedPrice));
        }
        else if (product.id == WIZ_PRODUCT_YEAR)
        {
            ui->btn_year->setText(QString(tr("%1 / year")).arg(product.localizedPrice));
        }
    }
    QTimer::singleShot(0, this, SLOT(checkUnfinishedTransation()));
}

void CWizIAPDialog::onPurchaseFinished(bool ok, const QByteArray& receipt, const QString& strTransationID)
{
    m_waitingMsgBox->done(ok);    

    if (ok)
    {
        saveUnfinishedTransation(strTransationID);
        emit checkReceiptRequest(receipt, strTransationID);
    }
    else
    {
        hideInfoLabel();
        ui->label_failed->setVisible(true);
    }
}

void CWizIAPDialog::loadUserInfo()
{
    setWindowTitle(tr("Account settings"));
   ui->stackedWidget->setCurrentIndex(0);
   QString extInfo = WizService::CommonApiEntry::appstoreParam(false);
   QString strToken = WizService::Token::token();
   QString strUrl = WizService::CommonApiEntry::makeUpUrlFromCommand("user_info", strToken, extInfo);
   qDebug() << "load user info : " << strUrl;
   ui->webView->load(QUrl(strUrl));
}

void CWizIAPDialog::loadIAPPage()
{
    setWindowTitle(tr("Upgrade VIP"));
    ui->stackedWidget->setCurrentIndex(1);
    hideInfoLabel();
    if (!ui->btn_month->isEnabled())
    {
        QTimer::singleShot(10, this, SLOT(loadProducts()));
    }
}

int CWizIAPDialog::exec()
{
    if (ui->stackedWidget->currentIndex() == 1)
    {
        hideInfoLabel();
        if (ui->btn_month->isEnabled())
        {
            checkUnfinishedTransation();
        }
    }
    return QDialog::exec();
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
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 100px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon1));
    QString strIcon2 = ::WizGetSkinResourceFileName(strThemeName, "iap_note");
    ui->label_text2->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-right: 80px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon2));
    QString strIcon3 = ::WizGetSkinResourceFileName(strThemeName, "iap_liuliang");
    ui->label_text3->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-left: 100px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon3));
    QString strIcon4 = ::WizGetSkinResourceFileName(strThemeName, "iap_user");
    ui->label_text4->setStyleSheet(QString("QLabel {border: none;background-image: url(%1);"
                                           "background-position: left center; background-repeat: no-repeat; margin-right: 80px; font:16px #5e5e5e; padding-left: 40px;}").arg(strIcon4));
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

void CWizIAPDialog::checkReceiptInfo(const QByteArray& receipt, const QString& strTransationID)
{
    QtConcurrent::run([this, receipt, strTransationID]() {
        QString strPlat;
    #ifdef Q_OS_MAC
        strPlat = "macosx";
    #else
        strPlat = "linux";
    #endif
        QString asServerUrl = WizService::CommonApiEntry::asServerUrl();
        QString checkUrl = asServerUrl + "/a/pay2/ios";
    //    QString checkUrl = "https://sandbox.itunes.apple.com/verifyReceipt";
    //    QString checkUrl = "https://buy.itunes.apple.com/verifyReceipt";
        CWizDatabase& db = CWizDatabaseManager::instance()->db();
        QString userID = db.GetUserId();
        QString userGUID = db.GetUserGUID();
        QString receiptBase64 = receipt.toBase64();
        receiptBase64 = QString(QUrl::toPercentEncoding(receiptBase64));
        QString strExtInfo = QString("client_type=%1&user_id=%2&user_guid=%3&transaction_id=%4&receipt=%5")
                .arg(strPlat).arg(userID).arg(userGUID).arg(strTransationID).arg(receiptBase64);

        qDebug() << "transation id = " << strTransationID;
    //    qDebug() << "check receipt : " << checkUrl << strExtInfo;

        QNetworkAccessManager net;
        QNetworkRequest request;
        request.setUrl(checkUrl);
        request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/x-www-form-urlencoded"));
        QNetworkReply* reply = net.post(request, strExtInfo.toUtf8());

        QEventLoop loop;
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();


        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return;
        }

        QString strResult = reply->readAll();
        reply->deleteLater();

        QMetaObject::invokeMethod(m_waitingMsgBox, "done", Qt::QueuedConnection,
                                  Q_ARG(int, 0));        
        parseCheckResult(strResult, strTransationID);
    });
}

void CWizIAPDialog::parseCheckResult(const QString& strResult, const QString& strTransationID)
{
    rapidjson::Document d;
    d.Parse<0>(strResult.toUtf8().constData());

    if (d.FindMember("error_code")) {
        qDebug() << QString::fromUtf8(d.FindMember("error")->value.GetString());
        return;
    }

    if (d.FindMember("return_code")) {
        int nCode = d.FindMember("return_code")->value.GetInt();
        if (nCode == 200) {
            qDebug() <<"IAP purchase successed!";
            QMetaObject::invokeMethod(this, "on_purchase_successed", Qt::QueuedConnection);
            removeTransationFromUnfinishedList(strTransationID);
            return;
        } else {
            QString message = QString::fromUtf8(d.FindMember("return_message")->value.GetString());
            qDebug() << "check on server failed , code :  " << nCode << "  message : " << message;
            QMetaObject::invokeMethod(this, "on_purchase_failed", Qt::QueuedConnection,
                                      Q_ARG(QString, message));
            return;
        }
    }
}

QStringList CWizIAPDialog::getUnfinishedTransations()
{
    QString transation = CWizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    return transation.split(';');
}

void CWizIAPDialog::saveUnfinishedTransation(const QString& strTransationID)
{
    QString transation = CWizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    if (!list.contains(strTransationID))
    {
        list.append(strTransationID);
    }
    transation = list.join(';');
    CWizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION, transation);
}

void CWizIAPDialog::removeTransationFromUnfinishedList(const QString& strTransationID)
{
    QString transation = CWizDatabaseManager::instance()->db().meta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION);
    QStringList list = transation.split(';', QString::SkipEmptyParts);
    if (!list.contains(strTransationID))
    {
        return;
    }
    list.removeOne(strTransationID);
    transation = list.join(';');
    CWizDatabaseManager::instance()->db().setMeta(APPSTORE_IAP, APPSTORE_UNFINISHEDTRANSATION, transation);
}

void CWizIAPDialog::on_btn_goBack_clicked()
{
    loadUserInfo();
}

void CWizIAPDialog::on_btn_month_clicked()
{

    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for AppStore..."));
    m_waitingMsgBox->open();

    //
    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_MONTH);
}

void CWizIAPDialog::on_btn_year_clicked()
{
    hideInfoLabel();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for AppStore..."));
    m_waitingMsgBox->open();

    m_iAPhelper->purchaseProduct(WIZ_PRODUCT_YEAR);
}

void CWizIAPDialog::loadProducts()
{
    if (m_iAPhelper == 0)
        m_iAPhelper = new CWizIAPHelper(this);

    m_iAPhelper->requestProducts();
    m_timer.start(2 * 60 * 1000);
    ui->label_downloading->setVisible(true);
}

void CWizIAPDialog::stopWaitTimer()
{
    m_timer.stop();
}

void CWizIAPDialog::onWaitingTimeOut()
{
    m_timer.stop();
    m_waitingMsgBox->close();    
    m_waitingMsgBox->setStandardButtons(QMessageBox::Ok);
    m_waitingMsgBox->setText(tr("Can not connect to Server, please try again later."));
    m_waitingMsgBox->exec();
    accept();
}

void CWizIAPDialog::onEditorPopulateJavaScriptWindowObject()
{
    Core::Internal::MainWindow* mainWindow = qobject_cast<Core::Internal::MainWindow *>(Core::ICore::mainWindow());
    if (mainWindow) {
        ui->webView->page()->mainFrame()->addToJavaScriptWindowObject("WizExplorerApp", mainWindow->object());
    }
}

void CWizIAPDialog::onCheckReceiptRequest(const QByteArray& receipt, const QString& strTransationID)
{
    m_waitingMsgBox->close();
    m_waitingMsgBox->setModal(true);
    m_waitingMsgBox->setStandardButtons(0);
    m_waitingMsgBox->setText(tr("Waiting for Wiz Server..."));
    m_waitingMsgBox->open();

    checkReceiptInfo(receipt, strTransationID);
}

void CWizIAPDialog::checkUnfinishedTransation()
{
    qDebug() << "Check unfinished transations";
    QStringList idList = getUnfinishedTransations();
    if (idList.count() == 1 && idList.first().isEmpty())
        return;

    QByteArray receipt;
    m_iAPhelper->loadLocalReceipt(receipt);
    if (receipt.isEmpty())
    {
        CWizMessageBox::warning(this, tr("Info"), tr("Can not load receipt!"));
        qDebug() << "local receipt load failed";
        return;
    }

    int result = CWizMessageBox::information(this, tr("Info"), tr("You have unfinished transation, continue to process it?"),
                             QMessageBox::Cancel | QMessageBox::Ok, QMessageBox::Ok);
    if (QMessageBox::Ok == result)
    {
        for (int i = 0; i < idList.count(); i++)
        {
            onCheckReceiptRequest(receipt, idList.at(i));
        }
    }
}

void CWizIAPDialog::on_purchase_successed()
{
    hideInfoLabel();
    ui->label_success->setVisible(true);
}

void CWizIAPDialog::on_purchase_failed(const QString& errorMsg)
{
    hideInfoLabel();
    ui->label_failed->setVisible(true);
    ui->label_failed->setText(QString("<html><head/><body><p align=\"center\"><span style=\""
                                      " font-size:16pt; color:#ff6666;\">%1</span></p></body></html>").arg(errorMsg));
}

#endif
