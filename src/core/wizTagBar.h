#ifndef CWIZTAGBAR_H
#define CWIZTAGBAR_H

#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <memory>
#include <map>
#include "share/wizobject.h"

class CWizExplorerApp;
class CWizDatabaseManager;
class CWizTagListWidget;

namespace Core {
namespace Internal {

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
    QSize	sizeHint() const override;
    static int textWidth(const QString text);

signals:
    void deleteTagRequest(const QString& guid);
    void removeTagRequest(const QString& guid);
    void selectedItemChanged(CTagItem* item);
    void renameTagRequest(const QString& guid, const QString& newName);


protected:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

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

protected:
    void resizeEvent(QResizeEvent* event);
    void focusOutEvent(QFocusEvent* event);

private:
    void reset();
    void applyStyleSheet();
    void addTagToTagBar(const QString& guid, const QString text);
    void calculateTagWidgetWidth();
    void clearTagSelection();

private:
    WIZDOCUMENTDATA m_doc;
    CWizTagListWidget* m_tagList;
    QLabel* m_label;
    QWidget* m_tagWidget;
    QToolButton* m_btnAdd;
    QToolButton* m_btnMore;
    QHBoxLayout* m_tagLayout;
    QLineEdit* m_lineEdit;
    std::map<QString, CTagItem*> m_mapTagWidgets;
    std::map<QString, QString> m_mapMoreTags;
    CWizExplorerApp& m_app;
    CWizDatabaseManager& m_dbMgr;
};

}}

#endif // CWIZTAGBAR_H
