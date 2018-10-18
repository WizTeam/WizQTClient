#ifndef CWIZTAGBAR_H
#define CWIZTAGBAR_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringListModel>
#include <memory>
#include <map>
#include "share/WizObject.h"

class WizExplorerApp;
class WizDatabaseManager;
class WizTagListWidget;

class WizStringListCompleter : public QCompleter
{
    Q_OBJECT

public:
    inline WizStringListCompleter(const QStringList& words, QObject * parent)
        : QCompleter(parent)
        , m_model()
    {
        setModel(&m_model);
        m_model.setStringList(words);
    }

    inline void resetStringList(QStringList wordList)
    {
        m_model.setStringList(wordList);
    }

private:
    QStringListModel m_model;
};

class WizTagLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    WizTagLineEdit(QWidget* parent = 0);
    void resetCompleter(const QStringList& tagNames);

signals:
    void completerFinished();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    WizStringListCompleter* m_completer;
};

class WizTagItem : public QWidget
{
    Q_OBJECT
public:
    explicit WizTagItem(const QString guid, const QString text, QWidget *parent = 0);
    ~WizTagItem();

    QString guid();
    QString name();
    void setName(const QString& name);

    bool	isReadOnly() const;
    void	setReadOnly(bool b);

    void setSelected(bool b);
    QSize	sizeHint() const;
    static int textWidth(const QString text);

signals:
    void deleteTagRequest(const QString& guid);
    void removeTagRequest(const QString& guid);
    void selectedItemChanged(WizTagItem* item);
    void renameTagRequest(const QString& guid, const QString& newName);


protected:
    void paintEvent(QPaintEvent* event);
    void focusInEvent(QFocusEvent* event);
    void focusOutEvent(QFocusEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);

private:
    void createContextMenu();

private slots:
    void on_menuActionRename();
    void on_menuActionRemove();
    void on_menuActionDelete();

private:
    QString m_tagGuid;
    QString m_tagName;
    bool m_readOnly;
    bool m_selected;
    bool m_closeButtonPressed;
    QMenu* m_menu;
    static QIcon m_iconDelete;
};

class WizTagBar : public QWidget
{
    Q_OBJECT
public:
    explicit WizTagBar(WizExplorerApp& app, QWidget *parent = 0);
    ~WizTagBar();

    void setDocument(const WIZDOCUMENTDATA& doc);    

public slots:
    void on_removeTagRequest(const QString& guid);
    void on_deleteTagRequest(const QString& guid);
    void on_renameTagRequest(const QString& guid, const QString& newName);
    void on_lineEditReturnPressed();
    void on_selectedItemChanged(WizTagItem* item);
    void on_buttonMoreClicked();
    void on_buttonAddClicked();

    //
    void on_tagCreated(const WIZTAGDATA& tag);
    void on_tagModified(const WIZTAGDATA& tagOld, const WIZTAGDATA& tagNew);
    void on_tagDeleted(const WIZTAGDATA& tag);
    void on_documentTagModified(const WIZDOCUMENTDATA& document);

    //
    void on_lineEditTextChanged(const QString& text);

signals:
    void widgetStatusChanged();

protected:
    void resizeEvent(QResizeEvent* event);
    void focusOutEvent(QFocusEvent* event);

    void hideEvent(QHideEvent* ev);
    void showEvent(QShowEvent* ev);

private:
    void reset();
    void applyStyleSheet();
    void addTagToTagBar(const QString& guid, const QString text);
    void calculateTagWidgetWidth();
    void clearTagSelection();
    void resetLineEditCompleter();

private:
    WIZDOCUMENTDATA m_doc;
    WizTagListWidget* m_tagList;
    QLabel* m_label;
    QWidget* m_tagWidget;
    QToolButton* m_btnAdd;
    QToolButton* m_btnMore;
    QHBoxLayout* m_tagLayout;
    WizTagLineEdit* m_lineEdit;
    std::map<QString, WizTagItem*> m_mapTagWidgets;
    std::map<QString, QString> m_mapMoreTags;
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
};


#endif // CWIZTAGBAR_H
