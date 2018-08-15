#include "WizInfoBar.h"

#include <QLabel>
#include <QHBoxLayout>
#include <sstream>
#include <string>
#include <QDebug>

#include "utils/WizStyleHelper.h"
#include "share/WizObject.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizMisc.h"
#include "WizDef.h"


WizInfoBar::WizInfoBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
    QString strStyle;
    strStyle += "font-size: " + QString::number(WizSmartScaleUI(12)) + "px; color: #a2a2a2;";
    setStyleSheet(strStyle);
    setContentsMargins(0, 0, 0, 6);

    int nHeight = Utils::WizStyleHelper::infoBarHeight();
    setFixedHeight(nHeight);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    setLayout(layout);

    m_labelCreatedTime = new QLabel(this);
    m_labelModifiedTime = new QLabel(this);
    m_labelOwner = new QLabel(this);
    m_labelSize = new QLabel(this);

    layout->addWidget(m_labelCreatedTime);
    layout->addWidget(m_labelModifiedTime);
    layout->addWidget(m_labelOwner);
    layout->addWidget(m_labelSize);
    layout->addStretch();
}

void WizInfoBar::setDocument(const WIZDOCUMENTDATA& data)
{
    QString strCreateTime = QObject::tr("Create time: ") + data.tCreated.toString("yyyy/M/d");
    m_labelCreatedTime->setText(strCreateTime);

    QString strModifiedTime = QObject::tr("Update time: ") + data.tDataModified.toString("yyyy/M/d");
    m_labelModifiedTime->setText(strModifiedTime);


    WizDatabase& db = m_app.databaseManager().db(data.strKbGUID);
    if (db.isGroup())
    {
        QString strOwner = db.getDocumentOwnerAlias(data);
        strOwner = QObject::tr("Owner: ") + (strOwner.isEmpty() ? data.strOwner : strOwner);
        strOwner = fontMetrics().elidedText(strOwner, Qt::ElideRight, 150);
        m_labelOwner->setVisible(true);
        m_labelOwner->setText(strOwner);
    }
    else
    {
        m_labelOwner->setVisible(false);
    }

    QString strFile = db.getDocumentFileName(data.strGUID);
    QString strSize = QObject::tr("Size: ") + ::WizGetFileSizeHumanReadalbe(strFile);
    m_labelSize->setText(strSize);
}
