#include "wizTagBar.h"
#include <QPalette>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "utils/stylehelper.h"
#include "share/wizmisc.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "wizdef.h"

const int TAGITEM_MARGIN = 16;
const int TAGITEM_HEIGHT  = 16;

using namespace Core::Internal;

CWizTagBar::CWizTagBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
{
    int nHeight = Utils::StyleHelper::tagBarHeight();
    setFixedHeight(nHeight);

//    setStyleSheet("font-size: 11px; color: #646464;");
    setContentsMargins(8, 0, 0, 0);
//    setFocusPolicy(Qt::StrongFocus);

    QPalette pl = palette();
    pl.setBrush(QPalette::Window, QBrush(QColor("#f7f8f9")));
    setPalette(pl);

    //
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    m_label = new QLabel(tr("Tag"), this);
    hLayout->addWidget(m_label);
    m_btnAdd = new QToolButton(this);
    hLayout->addWidget(m_btnAdd);
    m_tagWidget = new QWidget(this);
    hLayout->addWidget(m_tagWidget);
    m_lineEdit = new QLineEdit(this);
    hLayout->addWidget(m_lineEdit);
    hLayout->addStretch();

    //
    m_tagLayout = new QHBoxLayout(m_tagWidget);
    m_tagLayout->setContentsMargins(0, 0, 0, 0);

    applyStyleSheet();

    connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(on_lineEditReturnPressed()));

}

CWizTagBar::~CWizTagBar()
{

}

void CWizTagBar::setDocument(const WIZDOCUMENTDATA& doc)
{
    reset();
    //
    m_doc = doc;
    CWizDatabase&db = m_app.databaseManager().db(m_doc.strKbGUID);
    CWizTagDataArray arrayTag;
    db.GetDocumentTags(m_doc.strGUID, arrayTag);
    for (WIZTAGDATA tag : arrayTag)
    {
        qDebug() << "add tag item : " << tag.strName;
        addTag(tag.strGUID, tag.strName);
    }
}

void CWizTagBar::addTag(const QString& guid, const QString text)
{
    CTagItem* tagItem = new CTagItem(guid, text, this);
    m_tagLayout->addWidget(tagItem);
    m_mapTagWidgets.insert(std::make_pair(guid, tagItem));

    connect(tagItem, SIGNAL(deleteTagRequest(QString)), SLOT(on_deleteTagRequest(QString)));
    connect(tagItem, SIGNAL(selectedItemChanged(CTagItem*)), SLOT(on_selectedItemChanged(CTagItem*)));

}

void CWizTagBar::calculateTagWidgetWidth()
{
    int maxWidth = width() - m_label->width() - m_btnAdd->width() * 3 - m_lineEdit->width();
    int wgtWidth = 0;
    std::map<QString, QString> mapTemp;
    // calculate current tagWidgets
    std::for_each(std::begin(m_mapTagWidgets), std::end(m_mapTagWidgets), [&](const std::pair<QString, CTagItem*>& it){
        int itemWidth = it.second->sizeHint().width();
        if (wgtWidth < maxWidth && wgtWidth + itemWidth < maxWidth)
        {
            wgtWidth += itemWidth;
        }
        else
        {
            mapTemp.insert(std::make_pair(it.first, it.second->name()));
            m_tagLayout->removeWidget(it.second);
            it.second->deleteLater();
        }
    });

    // if temp set is empty move item from moreTags to tagWidgets
    if (mapTemp.empty())
    {
        std::for_each(std::begin(m_mapMoreTags), std::end(m_mapMoreTags), [&](const std::pair<QString, QString>& it){
            int itemWidth = CTagItem::textWidth(it.second);
            if (wgtWidth < maxWidth && wgtWidth + itemWidth < maxWidth)
            {
                wgtWidth += itemWidth;
                //
                addTag(it.first, it.second);
                mapTemp.insert(std::make_pair(it.first, it.second));
            }
        });

        // clear moreTags
        std::for_each(std::begin(mapTemp), std::end(mapTemp), [&](const std::pair<QString, QString>& it){
            m_mapMoreTags.erase(it.first);
        });
    }
    else
    {
        //  if temp set is not empty, add item to moreTags
        std::for_each(std::begin(mapTemp), std::end(mapTemp), [&](const std::pair<QString, QString>& it){
            m_mapMoreTags.insert(std::make_pair(it.first, it.second));
        });
    }
}

void CWizTagBar::on_deleteTagRequest(const QString& guid)
{
//    auto tagItem = m_mapTagWidgets.find(guid);
//    if (tagItem != m_mapTagWidgets.end());
    for (auto tagItem : m_mapTagWidgets)
    {
        if (tagItem.first != guid)
            continue;

        QWidget* tag = tagItem.second;
        m_tagLayout->removeWidget(tag);
        tag->deleteLater();
        m_mapTagWidgets.erase(guid);


        CWizDatabase& db = m_app.databaseManager().db(m_doc.strKbGUID);
        WIZTAGDATA data;
        if (db.TagFromGUID(guid, data))
        {
            db.DeleteTag(data, true, false);
        }
        else
        {
            qDebug() << "[Tag]Can not delete tag : " << guid;
        }
    }
}

void CWizTagBar::on_lineEditReturnPressed()
{
    //TODO:
}

void CWizTagBar::on_selectedItemChanged(CTagItem* item)
{
    for (auto tagItem : m_mapTagWidgets)
    {
        tagItem.second->setSelected(false);
    }
    if (item)
    {
        item->setSelected(true);
    }
    //
    update();
}

void CWizTagBar::resizeEvent(QResizeEvent* event)
{
    qDebug() << "resize event , old size " << event->oldSize() << "  new size : " << event->size();
    QWidget::resizeEvent(event);

//    m_tagWidget->setMaximumWidth(width() - m_label->width() - m_btnAdd->width() - m_lineEdit->width() );
    calculateTagWidgetWidth();
}

void CWizTagBar::reset()
{
    for (auto tagIt : m_mapTagWidgets)
    {
        m_tagLayout->removeWidget(tagIt.second);
        tagIt.second->deleteLater();
    }
    m_mapTagWidgets.clear();
}

void CWizTagBar::applyStyleSheet()
{
    m_lineEdit->setPlaceholderText(tr("Click here to add tags"));
    m_lineEdit->setStyleSheet("QLineEdit {border: 0px;"
                              "background: yellow;selection-background-color: darkgray;}");
    m_lineEdit->setFixedWidth(150);

    //
    QIcon icon = ::WizLoadSkinIcon(Utils::StyleHelper::themeName(), "action_addEditorBarItem");
    m_btnAdd->setIcon(icon);
    m_btnAdd->setStyleSheet("QToolButton { border: 0px; margin-top:2px;}");
}



CTagItem::CTagItem(const QString guid, const QString text, QWidget* parent)
    : QWidget(parent)
    , m_tagName(text)
    , m_tagGuid(guid)
    , m_pixDeleteNormal(nullptr)
    , m_pixDeletePressed(nullptr)
    , m_readOnly(false)
    , m_selected(false)
    , m_closeButtonPressed(false)
{
    if (!m_pixDeleteNormal)
    {
        bool bHighPix = ::WizIsHighPixel();
        QString deleteNormal = bHighPix ? "action_deleteEditorBarItem@2x"  : "action_deleteEditorBarItem";
        QString deletePressed = bHighPix ? "action_deleteEditorBarItem_on@2x" : "action_deleteEditorBarItem_on";
        deleteNormal = Utils::StyleHelper::skinResourceFileName(deleteNormal);
        deletePressed = Utils::StyleHelper::skinResourceFileName(deletePressed);
        m_pixDeleteNormal = std::make_shared<QPixmap>(deleteNormal);
        m_pixDeletePressed = std::make_shared<QPixmap>(deletePressed);
    }

//    QFont f;
//    QFontMetrics fm(f);
//    int nWidth = fm.width(m_text) + TAGITEM_MARGIN * 2;
//    setFixedWidth(nWidth);
}

CTagItem::~CTagItem()
{

}

QString CTagItem::guid()
{
    return m_tagGuid;
}

QString CTagItem::name()
{
    return m_tagName;
}

bool CTagItem::isReadOnly() const
{
    return m_readOnly;
}

void CTagItem::setReadOnly(bool b)
{
    m_readOnly = b;
}

void CTagItem::setSelected(bool b)
{
    m_selected = b;
}

QSize CTagItem::sizeHint() const
{
    QFont f;
    QFontMetrics fm(f);
    QSize sz(fm.width(m_tagName) + TAGITEM_MARGIN * 2, TAGITEM_HEIGHT);
    return sz;
}

int CTagItem::textWidth(const QString text)
{
    QFont f;
    QFontMetrics fm(f);
    return fm.width(text) + TAGITEM_MARGIN * 2;
}

void CTagItem::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QPainter pt(this);
    QRect rcBorder(0, (height() - TAGITEM_HEIGHT) / 2, width(), TAGITEM_HEIGHT);

    pt.setPen(Qt::NoPen);
    pt.setBrush(QBrush(QColor(m_selected ? "#c4d7e7" : "#d8e7ef")));

    pt.drawRoundedRect(rcBorder, 10, 10);
    pt.setPen(Qt::black);
    pt.drawText(rcBorder, Qt::AlignCenter, m_tagName);

    if (!m_readOnly)
    {
        if (m_selected && !m_closeButtonPressed)
        {
            QRect rcDel(rcBorder.right() - TAGITEM_MARGIN, (height() - TAGITEM_HEIGHT) / 2, TAGITEM_MARGIN,
                        TAGITEM_HEIGHT);
            pt.drawPixmap(rcDel, m_pixDeleteNormal.operator *());
        }
        else if (m_closeButtonPressed)
        {
            QRect rcDel(rcBorder.right() - TAGITEM_MARGIN, (height() - TAGITEM_HEIGHT) / 2, TAGITEM_MARGIN,
                        TAGITEM_HEIGHT);
            pt.drawPixmap(rcDel, m_pixDeletePressed.operator *());
        }
    }

}

void CTagItem::focusInEvent(QFocusEvent* event)
{
    qDebug() << "tag item focus in event";
    m_selected = true;
    QWidget::focusInEvent(event);
}

void CTagItem::focusOutEvent(QFocusEvent* event)
{
    qDebug() << "tag item focus out event";
    m_selected = false;
    QWidget::focusOutEvent(event);
}

void CTagItem::mousePressEvent(QMouseEvent* event)
{
    qDebug() << "tag item mouse pressed";
    if (m_selected)
    {
        qDebug() << "mouse press pos : " << event->pos();
        QRect rc(width() - TAGITEM_MARGIN, 0,
                 width(), height());

        if (rc.contains(event->pos()))
        {
            m_closeButtonPressed = true;
        }
        else
        {
            m_selected = false;
        }
    }
    else
    {
        emit selectedItemChanged(this);
    }
    update();
    QWidget::mousePressEvent(event);
}

void CTagItem::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_closeButtonPressed)
    {
        m_closeButtonPressed = false;
        update();
        emit deleteTagRequest(m_tagGuid);
    }
    QWidget::mouseReleaseEvent(event);
}
