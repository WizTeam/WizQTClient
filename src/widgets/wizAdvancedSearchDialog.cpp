#include "wizAdvancedSearchDialog.h"
#include "ui_wizAdvancedSearchDialog.h"
#include <QCursor>
#include <QLineEdit>
#include <QGroupBox>
#include <QMessageBox>
#include <QPainter>
#include <QButtonGroup>
#include <QDebug>
#include "share/wizDatabase.h"
#include "share/wizDatabaseManager.h"
#include "share/wizmisc.h"
#include "wiznotestyle.h"
#include "utils/stylehelper.h"

#define PARAM_SELECT_PARAM    QObject::tr("Select search param")
#define PARAM_SELECT_FOLDER  QObject::tr("Folder")
#define PARAM_SELECT_TAG          QObject::tr("Tag")
#define PARAM_SELECT_CONTAIN  QObject::tr("Contains")
#define PARAM_SELECT_DTCREATED  QObject::tr("Created time")
#define PARAM_SELECT_DTMODIFIED  QObject::tr("Modified time")
#define PARAM_SELECT_DTACCESSED  QObject::tr("Accessed time")

#define PARAM_CONTAIN_ATTACHMENT  QObject::tr("Attachment")

#define PARAM_DATE_BEFORE   QObject::tr("Before")
#define PARAM_DATE_AFTER    QObject::tr("After")

#define PARAM_DATE_TODAY              QObject::tr("Today")
#define PARAM_DATE_YESTODAY         QObject::tr("Yestoday")
#define PARAM_DATE_DAYBEFOREYESTODAY            QObject::tr("Day before yestoday")
#define PARAM_DATE_LASTWEEK         QObject::tr("Last week")
#define PARAM_DATE_LASTMONTH        QObject::tr("Last month")
#define PARAM_DATE_LASTYEAR         QObject::tr("Last year")

#define PARAM_FOLDER_CHILDFOLDER QObject::tr("--Select child folder--")

#define PARAM_NAME      "name:"
#define PARAM_SCOPE     "scope:"
#define PARAM_KEYWORD       "keyword:"
#define PARAM_PARAM     "param:"



CWizAdvancedSearchDialog::CWizAdvancedSearchDialog(bool searchOnly, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CWizAdvancedSearchDialog)
  , m_radioGroup(new QButtonGroup(this))
{
    ui->setupUi(this);

    ui->lineEdit_keyword->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->lineEdit_name->setAttribute(Qt::WA_MacShowFocusRect, false);
    ui->listWidget->setAttribute(Qt::WA_MacShowFocusRect, false);

    CWizListItemStyle<CWizSearchParamItem>* listStyle = new CWizListItemStyle<CWizSearchParamItem>();
    ui->listWidget->setStyle(listStyle);

    ui->lineEdit_name->setVisible(!searchOnly);
    ui->label->setVisible(!searchOnly);
    if (!searchOnly) {
        ui->lineEdit_name->setText(tr("Untitled Search"));
        setWindowTitle(tr("Custom advanced search"));
    }
    else {
        setWindowTitle(tr("Advanced search"));
    }
    ui->lineEdit_keyword->setPlaceholderText(tr("Multiple keywords should be separated by blank"));

    ui->comboBox_second->setVisible(false);
    ui->comboBox_third->setVisible(false);

    ui->radioButton_personalNotes->setChecked(true);
    initFirstCombox(false);

    m_radioGroup->addButton(ui->radioButton_allNotes, 0);
    m_radioGroup->addButton(ui->radioButton_personalNotes, 1);
    m_radioGroup->addButton(ui->radioButton_groupNotes, 2);
    connect(m_radioGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            SLOT(onRadioButtonClicked(QAbstractButton*)));

    ui->listWidget->setStyleSheet(Utils::StyleHelper::wizCommonListViewStyleSheet());    

    //
//    ui->label_5->setVisible(false);
//    ui->groupBox->setVisible(false);
}

CWizAdvancedSearchDialog::~CWizAdvancedSearchDialog()
{
    delete ui;
}

QString CWizAdvancedSearchDialog::getParams()
{
    int scope = m_radioGroup->checkedId();
    QString strParam = PARAM_NAME + ui->lineEdit_name->text() + "/" + PARAM_KEYWORD + ui->lineEdit_keyword->text()
            + "/" + PARAM_SCOPE + QString::number(scope);
    for (int i = 0; i < ui->listWidget->count(); i++)
    {
        strParam += QString("/") + PARAM_PARAM + ui->listWidget->item(i)->text();
    }

    return strParam;
}

void CWizAdvancedSearchDialog::setParams(const QString& strParam)
{
    QStringList paramList = strParam.split('/');
    foreach (QString strParam, paramList) {
        if (strParam.startsWith(PARAM_NAME))
        {
            QString strName = strParam.remove(PARAM_NAME);
            ui->lineEdit_name->setText(strName);
        }
        else if (strParam.startsWith(PARAM_KEYWORD))
        {
            QString strKeyword = strParam.remove(PARAM_KEYWORD);
            ui->lineEdit_keyword->setText(strKeyword);
        }
        else if (strParam.startsWith(PARAM_SCOPE))
        {
            int scopeId = strParam.remove(PARAM_SCOPE).toInt();
            QAbstractButton* button = m_radioGroup->button(scopeId);
            button->setChecked(true);
        }
        else if (strParam.startsWith(PARAM_PARAM))
        {
            QString param = strParam.remove(PARAM_PARAM);
            CWizSearchParamItem* item = new CWizSearchParamItem(param, ui->listWidget);
            ui->listWidget->addItem(item);
        }
    }
}

bool CWizAdvancedSearchDialog::paramToSQL(const QString& param, QString& sqlWhere,
                                          QString& keyword, QString& name, int& scope)
{
    QStringList paramList = param.split('/');
    foreach (QString strParam, paramList) {
        if (strParam.startsWith(PARAM_NAME))
        {
            name = strParam.remove(PARAM_NAME);
        }
        else if (strParam.startsWith(PARAM_KEYWORD))
        {
            keyword = strParam.remove(PARAM_KEYWORD);
        }
        else if (strParam.startsWith(PARAM_SCOPE))
        {
            scope = strParam.remove(PARAM_SCOPE).toInt();
        }
        else if (strParam.startsWith(PARAM_PARAM))
        {
            QStringList paramList = strParam.remove(PARAM_PARAM).split(',');
            Q_ASSERT(paramList.count() >= 2);
            if (paramList.first() == PARAM_SELECT_FOLDER)
            {
                QString strWhere = "DOCUMENT_LOCATION like '/" + paramList.at(1) + QString("/") +
                        (paramList.last().isEmpty() ? "%'" : paramList.last() + "/%'");
                sqlWhere += sqlWhere.isEmpty() ? strWhere : " and " + strWhere;
            }
//            else if (paramList.first() == PARAM_SELECT_TAG)
//            {
//                QString strWhere = "DOCUMENT_LOCATION like " + "/" + paramList.at(1) + "/" +
//                        (paramList.count() == 3 ? paramList.last() + "/" : "");
//                strSQL += strSQL.isEmpty() ? strWhere : " and " + strWhere;
//            }
            else if (paramList.first() == PARAM_SELECT_CONTAIN)
            {
                if (paramList.at(1) == PARAM_CONTAIN_ATTACHMENT)
                {
                    QString strWhere = "DOCUMENT_ATTACHEMENT_COUNT > 0";
                    sqlWhere += sqlWhere.isEmpty() ? strWhere : " and " + strWhere;
                }
            }
            else if (paramList.first() == PARAM_SELECT_DTCREATED )
            {
                QString strWhere = " DT_CREATED";
                strWhere += paramList.at(1) == PARAM_DATE_AFTER ? " > " : " < ";
                strWhere += ::WizTimeToSQL(getDateTimeByInterval(paramList.last()));
                sqlWhere += sqlWhere.isEmpty() ? strWhere : " and " + strWhere;
            }
            else if (paramList.first() == PARAM_SELECT_DTMODIFIED)
            {
                QString strWhere = " DT_MODIFIED";
                strWhere += paramList.at(1) == PARAM_DATE_AFTER ? " > " : " < ";
                strWhere += ::WizTimeToSQL(getDateTimeByInterval(paramList.last()));
                sqlWhere += sqlWhere.isEmpty() ? strWhere : " and " + strWhere;
            }
            else if (paramList.first() == PARAM_SELECT_DTACCESSED)
            {
                QString strWhere = " DT_ACCESSED";
                strWhere += paramList.at(1) == PARAM_DATE_AFTER ? " > " : " < ";
                strWhere += ::WizTimeToSQL(getDateTimeByInterval(paramList.last()));
                sqlWhere += sqlWhere.isEmpty() ? strWhere : " and " + strWhere;
            }
        }
    }
    return true;
}

void CWizAdvancedSearchDialog::onRadioButtonClicked(QAbstractButton* button)
{
    ui->comboBox_first->setCurrentIndex(0);
    ui->comboBox_second->clear();
    ui->comboBox_second->setVisible(false);
    ui->comboBox_third->setVisible(false);
    if (button == ui->radioButton_allNotes || button == ui->radioButton_groupNotes)
    {
        int index = ui->comboBox_first->findText(PARAM_SELECT_FOLDER);
        if (index != -1)
        {
            ui->listWidget->clear();
            ui->comboBox_first->removeItem(index);
        }
    }
    else if (button == ui->radioButton_personalNotes)
    {
        int index = ui->comboBox_first->findText(PARAM_SELECT_FOLDER);
        if (index == -1)
        {
            ui->comboBox_first->insertItem(1, PARAM_SELECT_FOLDER);
        }
    }
}

void CWizAdvancedSearchDialog::getDateList(QStringList& dateList)
{
    dateList.clear();
    dateList.append(PARAM_DATE_TODAY);
    dateList.append(PARAM_DATE_YESTODAY);
    dateList.append(PARAM_DATE_DAYBEFOREYESTODAY);
    dateList.append(PARAM_DATE_LASTWEEK);
    dateList.append(PARAM_DATE_LASTMONTH);
    dateList.append(PARAM_DATE_LASTYEAR);
}

void CWizAdvancedSearchDialog::getFirstLevelFolders(QStringList& folders)
{
    if (m_strFolders.isEmpty())
    {
        m_strFolders = CWizDatabaseManager::instance()->db().GetFolders();
    }

    QStringList folderList = m_strFolders.split('*', QString::SkipEmptyParts);
    foreach (QString strFolder, folderList) {
        if (strFolder.count('/') == 2)
        {
            strFolder.remove('/');
            folders.append(strFolder);
        }
    }    
}

void CWizAdvancedSearchDialog::getSecondLevelFolders(const QString& firstLevelFolder, QStringList& folders)
{
    if (m_strFolders.isEmpty())
    {
        m_strFolders = CWizDatabaseManager::instance()->db().GetFolders();
    }

    QStringList folderList = m_strFolders.split('*', QString::SkipEmptyParts);
    foreach (QString strFolder, folderList) {
        if (strFolder.startsWith("/" + firstLevelFolder) && strFolder.count('/') == 3)
        {
            strFolder.remove("/");
            strFolder.remove(0, firstLevelFolder.length());
            folders.append(strFolder);
        }
    }
}

void CWizAdvancedSearchDialog::getAllTags(QStringList& tags)
{
    if (m_strTags.isEmpty())
    {
        CWizTagDataArray arrayTag;
        CWizDatabaseManager::instance()->db().GetAllTags(arrayTag);
        CWizTagDataArray::const_iterator it;
        for (it = arrayTag.begin(); it != arrayTag.end(); it++) {
            m_strTags.append(it->strName + ",");
        }
    }

    if (!m_strTags.isEmpty())
        tags = m_strTags.split(",", QString::SkipEmptyParts);
}

COleDateTime CWizAdvancedSearchDialog::getDateTimeByInterval(const QString& str)
{
    COleDateTime dt;
    if (str == PARAM_DATE_TODAY)
    {
        dt = dt.addDays(-1);
    }
    else if (str == PARAM_DATE_YESTODAY)
    {
        dt = dt.addDays(-2);
    }
    else if (str == PARAM_DATE_DAYBEFOREYESTODAY)
    {
        dt = dt.addDays(-3);
    }
    else if (str == PARAM_DATE_LASTWEEK)
    {
        dt = dt.addDays(-8);
    }
    else if (str == PARAM_DATE_LASTMONTH)
    {
        dt = dt.addMonths(-1);
    }
    else if (str == PARAM_DATE_LASTYEAR)
    {
        dt = dt.addYears(-1);
    }
    return dt;
}

void CWizAdvancedSearchDialog::on_comboBox_first_activated(const QString &arg1)
{
    ui->comboBox_second->setVisible(true);
    ui->comboBox_third->setVisible(false);
    ui->comboBox_second->clear();
    ui->comboBox_third->clear();
    if (arg1 == PARAM_SELECT_PARAM)
    {
        ui->comboBox_second->setVisible(false);
    }
    else if (arg1 == PARAM_SELECT_FOLDER)
    {
        QStringList folders;
        getFirstLevelFolders(folders);
        ui->comboBox_second->addItems(folders);

        //
        ui->comboBox_third->clear();

        if (folders.count() > 0)
        {
            QString strFirstFolder = folders.first();
            folders.clear();
            getSecondLevelFolders(strFirstFolder, folders);
            if (folders.count() > 0)
            {
                ui->comboBox_third->clear();
                ui->comboBox_third->addItem(PARAM_FOLDER_CHILDFOLDER);
                ui->comboBox_third->addItems(folders);
                ui->comboBox_third->setVisible(true);
                ui->comboBox_third->setCurrentIndex(0);
            }
        }
    }
//    else if (arg1 == PARAM_SELECT_TAG)
//    {
//        QStringList tags;
//        getAllTags(tags);
//        if (tags.count() > 0)
//        {
//            ui->comboBox_second->addItems(tags);
//        }
//        else
//        {
//            ui->comboBox_second->setVisible(false);
//        }
//    }
    else if (arg1 == PARAM_SELECT_CONTAIN)
    {
        ui->comboBox_second->addItem(PARAM_CONTAIN_ATTACHMENT);
    }
    else if (arg1 == PARAM_SELECT_DTCREATED || arg1 == PARAM_SELECT_DTMODIFIED
             || arg1 == PARAM_SELECT_DTACCESSED)
    {
        ui->comboBox_second->addItem(PARAM_DATE_BEFORE);
        ui->comboBox_second->addItem(PARAM_DATE_AFTER);
        ui->comboBox_second->setCurrentIndex(0);

        QStringList dateList;
        getDateList(dateList);
        ui->comboBox_third->setVisible(true);
        ui->comboBox_third->addItems(dateList);
        ui->comboBox_third->setCurrentIndex(0);
    }
}

void CWizAdvancedSearchDialog::initFirstCombox(bool bSearchGroup)
{
    ui->comboBox_first->clear();

    ui->comboBox_first->addItem(PARAM_SELECT_PARAM);
    if (!bSearchGroup)
    {
        ui->comboBox_first->addItem(PARAM_SELECT_FOLDER);
//        ui->comboBox_first->addItem(PARAM_SELECT_TAG);
    }
    ui->comboBox_first->addItem(PARAM_SELECT_CONTAIN);
    ui->comboBox_first->addItem(PARAM_SELECT_DTCREATED);
    ui->comboBox_first->addItem(PARAM_SELECT_DTMODIFIED);
    ui->comboBox_first->addItem(PARAM_SELECT_DTACCESSED);
    ui->comboBox_first->setCurrentIndex(0);
}

void CWizAdvancedSearchDialog::on_comboBox_second_activated(const QString &arg1)
{
    ui->comboBox_third->setVisible(false);
    QString firstParam = ui->comboBox_first->currentText();
    if (firstParam == PARAM_SELECT_FOLDER)
    {
        ui->comboBox_third->clear();

        QStringList folders;
        getSecondLevelFolders(arg1, folders);
        if (folders.count() > 0)
        {
            ui->comboBox_third->clear();
            ui->comboBox_third->addItem(PARAM_FOLDER_CHILDFOLDER);
            ui->comboBox_third->addItems(folders);
            ui->comboBox_third->setVisible(true);
            ui->comboBox_third->setCurrentIndex(0);
        }
    }
    else if (firstParam == PARAM_SELECT_DTCREATED || firstParam == PARAM_SELECT_DTMODIFIED
             || firstParam == PARAM_SELECT_DTACCESSED)
    {
        ui->comboBox_third->setVisible(true);
    }
}

void CWizAdvancedSearchDialog::on_pushButton_cancel_clicked()
{
    reject();
}

void CWizAdvancedSearchDialog::on_pushButton_ok_clicked()
{
    if (ui->listWidget->count() == 0 && ui->lineEdit_keyword->text().isEmpty())
    {
        QMessageBox::information(0, tr("Info"), tr("Both of keywords and search param were empty!"));
        return;
    }
    accept();
}

void CWizAdvancedSearchDialog::on_toolButton_add_clicked()
{
    if (ui->comboBox_first->currentText() != PARAM_SELECT_PARAM)
    {
        QString strFirst = ui->comboBox_first->currentText();
        QString strSecond = ui->comboBox_second->isVisible() ? ui->comboBox_second->currentText() : "";
        QString strThird = ui->comboBox_third->isVisible() ? ui->comboBox_third->currentText() : "";
        if (strThird == PARAM_FOLDER_CHILDFOLDER) {
            strThird.clear();
        }
        QString strItem = strFirst + "," + strSecond + (strSecond.isEmpty() ? "" : "," + strThird);
        CWizSearchParamItem* item = new CWizSearchParamItem(strItem, ui->listWidget);
        ui->listWidget->addItem(item);
    }
}


CWizSearchParamItem::CWizSearchParamItem(const QString& text, QListWidget* view, int type)
    : QListWidgetItem(text, view, type)
{
    QString strThemeName = Utils::StyleHelper::themeName();

    m_pix = QPixmap(::WizGetSkinResourceFileName(strThemeName, "listItem_delete"));
}

void CWizSearchParamItem::draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const
{
    p->save();

    bool bSelected = vopt->state & QStyle::State_Selected;
    bool bFocused = listWidget()->hasFocus();
    drawItemBackground(p, vopt->rect, bSelected, bFocused);

    QPen pen;
    bSelected ? pen.setColor(Qt::white) : pen.setColor(Qt::black);
    p->setPen(pen);
    p->drawText(vopt->rect, Qt::AlignLeft | Qt::AlignVCenter, text());

     p->restore();
}

QRect CWizSearchParamItem::drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const
{
    QBrush brush;
    selected ? brush.setColor(Qt::blue) : brush.setColor(Qt::white);
    brush.setStyle(Qt::SolidPattern);
    p->setPen(Qt::NoPen);
    p->setBrush(brush);
    p->drawRect(rect);


    if (selected)
    {
        QRect pixRect(rect.right() - m_pix.width(), rect.top() + (rect.height() - m_pix.height()) / 2,
                      m_pix.width(), m_pix.height());
        p->drawPixmap(pixRect, m_pix);
    }
    return rect;
}

bool CWizSearchParamItem::removeIconClicked()
{
    QPoint pos = QCursor::pos();
    pos = listWidget()->mapFromGlobal(pos);
    QRect border = listWidget()->visualItemRect(this);
    QRect pixRect(border.right() - m_pix.width(), border.top() + (border.height() - m_pix.height()) / 2,
                  m_pix.width(), m_pix.height());
    return pixRect.contains(pos);
}

void CWizAdvancedSearchDialog::on_listWidget_itemClicked(QListWidgetItem *item)
{
    CWizSearchParamItem* paramItem = dynamic_cast<CWizSearchParamItem*>(item);
    Q_ASSERT(paramItem);
    if (paramItem)
    {
          if (paramItem->removeIconClicked())
          {
              ui->listWidget->takeItem(ui->listWidget->row(item));
          }
    }
}


