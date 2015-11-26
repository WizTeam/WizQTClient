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
#include "share/wizobject.h"

class CWizExplorerApp;
class CWizDatabaseManager;
class CWizTagListWidget;

namespace Core {
namespace Internal {

class CWizStringListCompleter : public QCompleter
{
    Q_OBJECT

public:
    inline CWizStringListCompleter(const QStringList& words, QObject * parent)
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

class CTagLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    CTagLineEdit(QWidget* parent = 0);
    void resetCompleter(const QStringList& tagNames);

signals:
    void completerFinished();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    CWizStringListCompleter* m_completer;
};

class CTagItem : public QWidget
{
    Q_OBJECT
public:
    explicit CTagItem(const QString guid, const QString text, QWidget *parent = 0);
    ~CTagItem();

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
    void selectedItemChanged(CTagItem* item);
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
    std::shared_ptr<QPixmap> m_pixDeleteNormal;
    std::shared_ptr<QPixmap> m_pixDeletePressed;
};

class CWizTagBar : public QWidget
{
    Q_OBJECT
public:
    explicit CWizTagBar(CWizExplorerApp& app, QWidget *parent = 0);
    ~CWizTagBar();

    void setDocument(const WIZDOCUMENTDATA& doc);    

public slots:
    void on_removeTagRequest(const QString& guid);
    void on_deleteTagRequest(const QString& guid);
    void on_renameTagRequest(const QString& guid, const QString& newName);
    void on_lineEditReturnPressed();
    void on_selectedItemChanged(CTagItem* item);
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
    CWizTagListWidget* m_tagList;
    QLabel* m_label;
    QWidget* m_tagWidget;
    QToolButton* m_btnAdd;
    QToolButton* m_btnMore;
    QHBoxLayout* m_tagLayout;
    CTagLineEdit* m_lineEdit;
    std::map<QString, CTagItem*> m_mapTagWidgets;
    std::map<QString, QString> m_mapMoreTags;
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
};

}}

#endif // CWIZTAGBAR_H
