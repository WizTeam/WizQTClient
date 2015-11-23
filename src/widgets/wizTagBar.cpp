#include "wizTagBar.h"
#include <QPalette>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QAbstractItemView>
#include <QDebug>
#include "utils/stylehelper.h"
#include "utils/logger.h"
#include "share/wizmisc.h"
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "wizdef.h"
#include "wizLineInputDialog.h"
#include "wiztaglistwidget.h"
#include "share/wizAnalyzer.h"

const int TAGITEM_MARGIN = 16;
const int TAGITEM_HEIGHT  = 16;
const int TAGITEM_DELETEICONSIZE = 10;

using namespace Core::Internal;

CWizTagBar::CWizTagBar(CWizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_tagList(nullptr)
{
    int nHeight = Utils::StyleHelper::tagBarHeight() - 4;
    setFixedHeight(nHeight);

//    setStyleSheet("font-size: 11px; color: #646464;");
    setFocusPolicy(Qt::ClickFocus);

/*    QPalette pl = palette();
    pl.setColor(QPalette::Window, QColor("#FFFFFF"));
//    pl.setBrush(QPalette::Window, QColor("#f7f8f9"));
    setPalette(pl);
    setAutoFillBackground(true)*/;

    //
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(6);
    m_label = new QLabel(tr("Tag"), this);
    m_label->setStyleSheet("font-size:12px; color:#A2A2A2;");
    hLayout->addWidget(m_label);
    m_btnAdd = new QToolButton(this);
    hLayout->addWidget(m_btnAdd);
    m_tagWidget = new QWidget(this);
    hLayout->addWidget(m_tagWidget);
    m_btnMore = new QToolButton(this);
    hLayout->addWidget(m_btnMore);
    m_btnMore->setVisible(false);
    m_lineEdit = new CTagLineEdit(this);
    QFont f = m_lineEdit->font();
    f.setPixelSize(12);
    m_lineEdit->setFont(f);
    hLayout->addWidget(m_lineEdit);
    hLayout->addStretch();

    //
    m_tagLayout = new QHBoxLayout(m_tagWidget);
    m_tagLayout->setContentsMargins(0, 0, 0, 0);
    m_tagLayout->setSpacing(6);

    applyStyleSheet();

    connect(m_lineEdit, SIGNAL(returnPressed()), SLOT(on_lineEditReturnPressed()));
    connect(m_lineEdit, SIGNAL(completerFinished()), SLOT(on_lineEditReturnPressed()));
    connect(m_lineEdit, SIGNAL(textChanged(QString)), SLOT(on_lineEditTextChanged(QString)));
    connect(m_btnMore, SIGNAL(clicked()), SLOT(on_buttonMoreClicked()));
    connect(m_btnAdd, SIGNAL(clicked()), SLOT(on_buttonAddClicked()));

    //
    connect(&m_dbMgr, SIGNAL(tagCreated(WIZTAGDATA)),
            SLOT(on_tagCreated(WIZTAGDATA)));
    connect(&m_dbMgr, SIGNAL(tagModified(WIZTAGDATA,WIZTAGDATA)),
            SLOT(on_tagModified(WIZTAGDATA,WIZTAGDATA)));
    connect(&m_dbMgr, SIGNAL(tagDeleted(WIZTAGDATA)),
            SLOT(on_tagDeleted(WIZTAGDATA)));
    connect(&m_dbMgr, SIGNAL(documentTagModified(WIZDOCUMENTDATA)),
            SLOT(on_documentTagModified(WIZDOCUMENTDATA)));

    //
    resetLineEditCompleter();
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
        // qDebug() << "add tag item : " << tag.strName;
        addTagToTagBar(tag.strGUID, tag.strName);
    }
}

void CWizTagBar::on_removeTagRequest(const QString& guid)
{
    CWizDatabase& db = m_app.databaseManager().db(m_doc.strKbGUID);
    WIZTAGDATA tagData;
    if (db.TagFromGUID(guid, tagData))
    {
        WIZDOCUMENTDATA docData;
        db.DocumentFromGUID(m_doc.strGUID, docData);
        CWizDocument doc(m_dbMgr.db(), docData);
        doc.RemoveTag(tagData);
        //
        on_tagDeleted(tagData);
    }
    else
    {
        qWarning() << "[Tag]Can not find tag : " << guid;
    }
}

void CWizTagBar::addTagToTagBar(const QString& guid, const QString text)
{
    CTagItem* tagItem = new CTagItem(guid, text, this);            

    int maxWidth = width() - m_label->width() - m_btnAdd->width() * 3 - m_lineEdit->width() * 1.5;
    int wgtWidth = 0;

    // calculate current tagWidgets
    // qDebug() << "before cal m_mapTagWidgets size : " << m_mapTagWidgets.size();
    std::for_each(std::begin(m_mapTagWidgets), std::end(m_mapTagWidgets), [&](const std::pair<QString, CTagItem*>& it){
        // qDebug() << "item guid : " << it.first << " item : " << it.second;
        if (guid == it.first)
        {
            qInfo() << "[Tag]Try to add a same tag";
            return;
        }

        //
        int itemWidth = it.second->sizeHint().width();
        if (wgtWidth < maxWidth && wgtWidth + itemWidth < maxWidth)
        {
            wgtWidth += itemWidth;
        }
        else
        {
            wgtWidth = maxWidth + 1;
            // qDebug() << "to much item return;";
            return;
        }
    });

    // qDebug() << "current wgt width : " << wgtWidth;
    if (wgtWidth + tagItem->sizeHint().width() >= maxWidth)
    {
        tagItem->deleteLater();
        m_mapMoreTags.insert(std::make_pair(guid, text));
        m_btnMore->setVisible(true);
        return;
    }

    //
    m_tagLayout->addWidget(tagItem);
    m_mapTagWidgets.insert(std::make_pair(guid, tagItem));
    //
    connect(tagItem, SIGNAL(deleteTagRequest(QString)), SLOT(on_deleteTagRequest(QString)));
    connect(tagItem, SIGNAL(selectedItemChanged(CTagItem*)), SLOT(on_selectedItemChanged(CTagItem*)));
    connect(tagItem, SIGNAL(renameTagRequest(QString, QString)), SLOT(on_renameTagRequest(QString,QString)));
    connect(tagItem, SIGNAL(removeTagRequest(QString)), SLOT(on_removeTagRequest(QString)));

}

void CWizTagBar::calculateTagWidgetWidth()
{
    int maxWidth = width() - m_label->width() - m_btnAdd->width() * 3 - m_lineEdit->width() * 2;
    int wgtWidth = 0;
    std::map<QString, QString> mapTemp;
    // calculate current tagWidgets
    // qDebug() << "before cal m_mapTagWidgets size : " << m_mapTagWidgets.size();
    std::for_each(std::begin(m_mapTagWidgets), std::end(m_mapTagWidgets), [&](const std::pair<QString, CTagItem*>& it){
        // qDebug() << "item guid : " << it.first << " item : " << it.second;
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
                addTagToTagBar(it.first, it.second);
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
            m_mapTagWidgets.erase(it.first);
        });
    }

    bool visible =m_mapMoreTags.size() > 0;
    m_btnMore->setVisible(visible);
}

void CWizTagBar::clearTagSelection()
{
    for (auto tagItem : m_mapTagWidgets)
    {
        tagItem.second->setSelected(false);
    }
}

void CWizTagBar::resetLineEditCompleter()
{
    CWizTagDataArray arrayTag;
    m_dbMgr.db().GetAllTags(arrayTag);
    QSet<QString> tagNameSet;
    for (WIZTAGDATA tag : arrayTag)
    {
        tagNameSet.insert(tag.strName);
    }
    QStringList tagNames(tagNameSet.toList());
    m_lineEdit->resetCompleter(tagNames);
}

void CWizTagBar::on_deleteTagRequest(const QString& guid)
{    
    CWizDatabase& db = m_app.databaseManager().db(m_doc.strKbGUID);
    WIZTAGDATA data;
    if (db.TagFromGUID(guid, data))
    {
        db.DeleteTag(data, true, false);
    }
    else
    {
        qWarning() << "[Tag]Can not delete tag : " << guid;
    }
}

void CWizTagBar::on_renameTagRequest(const QString& guid, const QString& newName)
{
    // qDebug() << "on rename tag request";
    WIZTAGDATA data;
    CWizDatabase& db = m_app.databaseManager().db(m_doc.strKbGUID);
    if (db.TagFromGUID(guid, data))
    {
        data.strName = newName;
        db.ModifyTag(data);
    }
    else
    {
        qWarning() << "[Tag]Can not find tag : " << guid;
    }
}

void CWizTagBar::on_lineEditReturnPressed()
{
    QString strTagNames = m_lineEdit->text();
    if (strTagNames.isEmpty())
        return;
    m_lineEdit->setText("");

    WizGetAnalyzer().LogAction("addTagByTagBarInput");

    WIZDOCUMENTDATA docData;
    CWizDatabase& db = m_dbMgr.db();
    db.DocumentFromGUID(m_doc.strGUID, docData);
    CWizDocument doc(db, docData);

    //
    QStringList sl = strTagNames.split(';');
    QStringList::const_iterator it;
    for (it = sl.begin(); it != sl.end(); it++) {
        CString strTagName = *it;

        // only create tag for unique name
        WIZTAGDATA tag;
        CWizTagDataArray arrayTag;
        db.TagByName(strTagName, arrayTag);
        for (WIZTAGDATA tagItem : arrayTag)
        {
            if (tagItem.strParentGUID.IsEmpty())
            {
                tag = tagItem;
                break;
            }
        }
        if (!tag.strGUID.IsEmpty())
        {
            qInfo() << QString("Tag name already exist: %1").arg(strTagName);
             doc.AddTag(tag);
        }
        else
        {
            db.CreateTag("", strTagName, "", tag);
            doc.AddTag(tag);
        }
    }    
}

void CWizTagBar::on_selectedItemChanged(CTagItem* item)
{
    clearTagSelection();
    if (item)
    {
        item->setSelected(true);
    }
    //
    update();
}

void CWizTagBar::on_buttonMoreClicked()
{
    QMenu* menu = new QMenu(this);
    for (auto moreTag : m_mapMoreTags)
    {
        menu->addAction(moreTag.second);
    }

    int buttonTopMargin  = 4;
    QPoint pos = m_btnMore->mapToGlobal(QPoint(0, height() - buttonTopMargin));
    menu->popup(pos);

    WizGetAnalyzer().LogAction("buttonMoreOnTagBar");
}

void CWizTagBar::on_buttonAddClicked()
{
    clearTagSelection();
    //
    if (!m_tagList) {
        m_tagList = new CWizTagListWidget(this);
    }

    WIZDOCUMENTDATA doc;
    m_dbMgr.db(m_doc.strKbGUID).DocumentFromGUID(m_doc.strGUID, doc);
    m_tagList->setDocument(doc);

    QRect rc = m_btnAdd->rect();
    QPoint pt = m_btnAdd->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_tagList->showAtPoint(pt);

    WizGetAnalyzer().LogAction("buttonAddOnTagBar");
}

void CWizTagBar::on_tagCreated(const WIZTAGDATA& tag)
{
    //NOTE: do not process tag created signal. new tag could be created in other place
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
    {
        resetLineEditCompleter();
    }
}

void CWizTagBar::on_tagModified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew)
{
    if (tagNew.strKbGUID == m_dbMgr.db().kbGUID())
    {
        resetLineEditCompleter();
        //
        for (auto tagWgt : m_mapTagWidgets)
        {
            if (tagWgt.first == tagOld.strGUID)
            {
                tagWgt.second->setName(tagNew.strName);
                on_documentTagModified(m_doc);
                return;
            }
        }

        for (auto tagMore : m_mapMoreTags)
        {
            if (tagMore.first == tagOld.strGUID)
            {
                tagMore.second = tagNew.strName;
                return;
            }
        }
    }
}

void CWizTagBar::on_tagDeleted(const WIZTAGDATA& tag)
{
    if (tag.strKbGUID == m_dbMgr.db().kbGUID())
    {
        resetLineEditCompleter();
        //
        for (auto tagWgt : m_mapTagWidgets)
        {
            if (tagWgt.first == tag.strGUID)
            {
                m_tagLayout->removeWidget(tagWgt.second);
                tagWgt.second->deleteLater();
                m_mapTagWidgets.erase(tagWgt.first);
                calculateTagWidgetWidth();
                update();
                return;
            }
        }

        for (auto tagMore : m_mapMoreTags)
        {
            if (tagMore.first == tag.strGUID)
            {
                m_mapMoreTags.erase(tagMore.first);
                return;
            }
        }
    }
}

void CWizTagBar::on_documentTagModified(const WIZDOCUMENTDATA& document)
{
    if (document.strGUID == m_doc.strGUID)
    {
        reset();
        setDocument(document);
    }
}

void CWizTagBar::on_lineEditTextChanged(const QString& text)
{
//    if (text.isEmpty())
//    {
//        m_lineEdit->setStyleSheet("QLineEdit {border: 0px; color:#6c6c6c;}");
//    }
//    else
//    {
//        m_lineEdit->setStyleSheet("QLineEdit {border: 0px; color:#000000;}");
//    }
}

void CWizTagBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    calculateTagWidgetWidth();
}

void CWizTagBar::focusOutEvent(QFocusEvent* event)
{
    QWidget::focusOutEvent(event);
    //
    clearTagSelection();
}

void CWizTagBar::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

void CWizTagBar::showEvent(QShowEvent* ev)
{
    QWidget::showEvent(ev);

    emit widgetStatusChanged();
}

void CWizTagBar::reset()
{
    for (auto tagIt : m_mapTagWidgets)
    {
        m_tagLayout->removeWidget(tagIt.second);
        tagIt.second->deleteLater();
    }
    m_mapTagWidgets.clear();
    m_mapMoreTags.clear();
    m_btnMore->setVisible(false);
}

void CWizTagBar::applyStyleSheet()
{
    m_lineEdit->setPlaceholderText(tr("Click here to add tags"));
    m_lineEdit->setStyleSheet("QLineEdit {border: 0px; color:#B6B6B6; background-color:transparent;}");
    m_lineEdit->setFixedWidth(150);
    m_lineEdit->setAttribute(Qt::WA_MacShowFocusRect, false);

    //
    QIcon icon = ::WizLoadSkinIcon(Utils::StyleHelper::themeName(), "action_addEditorBarItem");
    m_btnAdd->setIcon(icon);
    m_btnAdd->setStyleSheet("QToolButton { border: 0px; margin-top:2px;}");

    icon = ::WizLoadSkinIcon(Utils::StyleHelper::themeName(), "action_moreTags");
    m_btnMore->setIcon(icon);
    m_btnMore->setStyleSheet("QToolButton { border: 0px; margin-top:2px;}");
}


CTagItem::CTagItem(const QString guid, const QString text, QWidget* parent)
    : QWidget(parent)
    , m_tagGuid(guid)
    , m_tagName(text)
    , m_readOnly(false)
    , m_selected(false)
    , m_closeButtonPressed(false)
    , m_menu(nullptr)
    , m_pixDeleteNormal(nullptr)
    , m_pixDeletePressed(nullptr)
{
    if (!m_pixDeleteNormal)
    {        
        QString deleteNormal = Utils::StyleHelper::skinResourceFileName("action_deleteEditorBarItem", true);
        QString deletePressed = Utils::StyleHelper::skinResourceFileName("action_deleteEditorBarItem_on", true);
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

void CTagItem::setName(const QString& name)
{
    m_tagName = name;
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
    pt.setBrush(QBrush(QColor(m_selected ? "#535453" : "#B6B6B6")));
    pt.setRenderHint(QPainter::Antialiasing, true);
    pt.drawRoundedRect(rcBorder, 8, 8);
    pt.setPen(Qt::white);
    QFont f = pt.font();
    f.setPixelSize(11);
    pt.setFont(f);
    pt.drawText(rcBorder, Qt::AlignCenter, m_tagName);

    if (!m_readOnly)
    {
        if (m_selected && !m_closeButtonPressed)
        {
            QRect rcDel(rcBorder.right() - TAGITEM_DELETEICONSIZE - 2, (height() - TAGITEM_DELETEICONSIZE) / 2, TAGITEM_DELETEICONSIZE,
                        TAGITEM_DELETEICONSIZE);
            pt.drawPixmap(rcDel, m_pixDeleteNormal.operator *());
        }
        else if (m_closeButtonPressed)
        {
            QRect rcDel(rcBorder.right() - TAGITEM_DELETEICONSIZE - 2, (height() - TAGITEM_DELETEICONSIZE) / 2, TAGITEM_DELETEICONSIZE,
                        TAGITEM_DELETEICONSIZE);
            pt.drawPixmap(rcDel, m_pixDeletePressed.operator *());
        }
    }
}

void CTagItem::focusInEvent(QFocusEvent* event)
{
    m_selected = true;
    QWidget::focusInEvent(event);
}

void CTagItem::focusOutEvent(QFocusEvent* event)
{
    m_selected = false;
    QWidget::focusOutEvent(event);
}

void CTagItem::mousePressEvent(QMouseEvent* event)
{
    if (m_selected)
    {
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
        on_menuActionRemove();
        WizGetAnalyzer().LogAction("removeTagByCloseButton");
    }
    QWidget::mouseReleaseEvent(event);
}

void CTagItem::contextMenuEvent(QContextMenuEvent* event)
{
    if (!m_menu)
    {
        createContextMenu();
    }
    event->accept();
    m_menu->popup(mapToGlobal(event->pos()));
    //
    emit selectedItemChanged(this);
}

void CTagItem::createContextMenu()
{
    m_menu = new QMenu(this);
    QAction* actionR = m_menu->addAction(tr("Rename..."));
    connect(actionR, SIGNAL(triggered()), SLOT(on_menuActionRename()));
    m_menu->addSeparator();
    QAction* actionRe = m_menu->addAction(tr("Remove"));
    connect(actionRe, SIGNAL(triggered()), SLOT(on_menuActionRemove()));
    QAction* actionD = m_menu->addAction(tr("Delete"));
    connect(actionD, SIGNAL(triggered()), SLOT(on_menuActionDelete()));
}

void CTagItem::on_menuActionRename()
{
    CWizLineInputDialog dialog(tr("Rename tag"),
                                                          tr("Please input tag name: "),
                                                          m_tagName, window());

    int result = dialog.exec();
    if (result == QDialog::Accepted)
    {
        QString strTagName = dialog.input();

        if (strTagName.isEmpty() || strTagName == m_tagName)
            return;

        emit renameTagRequest(guid(), strTagName);
    }
}

void CTagItem::on_menuActionRemove()
{
    removeTagRequest(m_tagGuid);
}

void CTagItem::on_menuActionDelete()
{
    QMessageBox* msgBox = new QMessageBox(window());
    msgBox->setWindowTitle(tr("Delete tag"));
    msgBox->setIcon(QMessageBox::Information);
    msgBox->addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton* btnOK = msgBox->addButton(tr("OK"), QMessageBox::YesRole);
    msgBox->setDefaultButton(btnOK);

    QString strWarning = tr("Do you really want to delete tag: %1 ? (include child tags if any)").arg(m_tagName);
    msgBox->setText(strWarning);
    if (msgBox->exec() != QDialog::Accepted)
        return;

    emit deleteTagRequest(m_tagGuid);
}


CTagLineEdit::CTagLineEdit(QWidget* parent)
    : QLineEdit(parent)
    , m_completer(nullptr)
{
}

void CTagLineEdit::resetCompleter(const QStringList& tagNames)
{
    if (!m_completer)
    {        
        m_completer = new CWizStringListCompleter(tagNames, this);
        QStyledItemDelegate* mCompleterItemDelegate = new QStyledItemDelegate(m_completer);
        m_completer->popup()->setItemDelegate(mCompleterItemDelegate); //Must be set after every time the model is set
        m_completer->popup()->setStyleSheet("QAbstractItemView::item:selected{background:#448aff;}"
                                            "QAbstractItemView::item:hover{background:#448aff;}");
        m_completer->setCaseSensitivity(Qt::CaseInsensitive);
        setCompleter(m_completer);
    }

    m_completer->resetStringList(tagNames);
}

void CTagLineEdit::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (m_completer->popup()->isVisible())
        {
            m_completer->popup()->hide();
            event->accept();
            emit completerFinished();
            return;
        }
    }

    QLineEdit::keyPressEvent(event);
}
