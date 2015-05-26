#ifndef WIZADVANCEDSEARCHDIALOG_H
#define WIZADVANCEDSEARCHDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "share/wizqthelper.h"



namespace Ui {
class CWizAdvancedSearchDialog;
}

class CWizSearchParamItem : public QListWidgetItem
{
public:
    explicit CWizSearchParamItem(const QString &text, QListWidget *view = 0, int type = Type);

    void draw(QPainter* p, const QStyleOptionViewItemV4* vopt) const;
    QRect drawItemBackground(QPainter* p, const QRect& rect, bool selected, bool focused) const;

    bool removeIconClicked();

private:
    QPixmap m_pix;
};


class QAbstractButton;
class QButtonGroup;
class CWizAdvancedSearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CWizAdvancedSearchDialog(bool searchOnly, QWidget *parent = 0);
    ~CWizAdvancedSearchDialog();
    QString getParams();
    void setParams(const QString& strParam);

    static bool paramToSQL(const QString& param, QString& sqlWhere, QString& keyword,
                           QString& name, int& scope);

public slots:
    void onRadioButtonClicked(QAbstractButton* button);

private slots:
    void on_comboBox_first_activated(const QString &arg1);

    void on_comboBox_second_activated(const QString &arg1);

    void on_pushButton_cancel_clicked();

    void on_pushButton_ok_clicked();

    void on_toolButton_add_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);


private:
    Ui::CWizAdvancedSearchDialog *ui;
    QString m_strFolders;
    QString m_strTags;
    QButtonGroup* m_radioGroup;

    void initFirstCombox(bool bSearchGroup);

    void getDateList(QStringList& getDateList);
    void getFirstLevelFolders(QStringList& folders);
    void getSecondLevelFolders(const QString& firstLevelFolder, QStringList& folders);
    void getAllTags(QStringList& tags);
    static COleDateTime getDateTimeByInterval(const QString& str);

};

#endif // WIZADVANCEDSEARCHDIALOG_H
