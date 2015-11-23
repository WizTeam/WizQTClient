#include "infobar.h"

#include <QLabel>
#include <QHBoxLayout>

#include "utils/stylehelper.h"
#include "share/wizobject.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizmisc.h"
#include "wizdef.h"

using namespace Core::Internal;

InfoBar::InfoBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
    setStyleSheet("font-size: 12px; color: #a2a2a2;");
    setContentsMargins(0, 0, 0, 6);

    int nHeight = Utils::StyleHelper::infoBarHeight();
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

void InfoBar::setDocument(const WIZDOCUMENTDATA& data)
{
    QString strCreateTime = QObject::tr("Create time: ") + data.tCreated.toString("yyyy/M/d");
    m_labelCreatedTime->setText(strCreateTime);

    QString strModifiedTime = QObject::tr("Update time: ") + data.tDataModified.toString("yyyy/M/d");
    m_labelModifiedTime->setText(strModifiedTime);


    CWizDatabase& db = m_app.databaseManager().db(data.strKbGUID);
    if (db.IsGroup())
    {
        QString strOwner = db.GetDocumentOwnerAlias(data);
        strOwner = QObject::tr("Owner: ") + (strOwner.isEmpty() ? data.strOwner : strOwner);
        strOwner = fontMetrics().elidedText(strOwner, Qt::ElideRight, 150);
        m_labelOwner->setVisible(true);
        m_labelOwner->setText(strOwner);
    }
    else
    {
        m_labelOwner->setVisible(false);
    }

    QString strFile = db.GetDocumentFileName(data.strGUID);
    QString strSize = QObject::tr("Size: ") + ::WizGetFileSizeHumanReadalbe(strFile);
    m_labelSize->setText(strSize);
}
