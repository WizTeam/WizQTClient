#include "infobar.h"

#include <QLabel>
#include <QHBoxLayout>

#include "share/wizobject.h"
#include "share/wizDatabaseManager.h"
#include "share/wizDatabase.h"
#include "share/wizmisc.h"

using namespace Core::Internal;

InfoBar::InfoBar(QWidget *parent)
    : QWidget(parent)
{
    setStyleSheet("font-size: 11px; color: #646464;");
    setContentsMargins(5, 0, 0, 0);

    // FIXME: should be the same as editor toolbar
    setFixedHeight(32);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);
    setLayout(layout);

    m_labelCreatedTime = new QLabel(this);
    m_labelModifiedTime = new QLabel(this);
    m_labelAuthor = new QLabel(this);
    m_labelSize = new QLabel(this);

    layout->addWidget(m_labelCreatedTime);
    layout->addWidget(m_labelModifiedTime);
    layout->addWidget(m_labelAuthor);
    layout->addWidget(m_labelSize);
    layout->addStretch();
}

void InfoBar::setDocument(const WIZDOCUMENTDATA& data)
{
    QString strCreateTime = QObject::tr("Create time: ") + data.tCreated.toString("yyyy-MM-dd");
    m_labelCreatedTime->setText(strCreateTime);

    QString strModifiedTime = QObject::tr("Update time: ") + data.tModified.toString("yyyy-MM-dd");
    m_labelModifiedTime->setText(strModifiedTime);

    QString strAuthor = QObject::tr("Author: ") + data.strOwner;
    strAuthor = fontMetrics().elidedText(strAuthor, Qt::ElideRight, 150);
    m_labelAuthor->setText(strAuthor);

    QString strFile = CWizDatabaseManager::instance()->db(data.strKbGUID).GetDocumentFileName(data.strGUID);
    QString strSize = QObject::tr("Size: ") + ::WizGetFileSizeHumanReadalbe(strFile);
    m_labelSize->setText(strSize);
}
