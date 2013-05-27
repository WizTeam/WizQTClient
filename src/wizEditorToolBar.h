#ifndef WIZEDITORTOOLBAR_H
#define WIZEDITORTOOLBAR_H

#include <QWidget>
#include <QComboBox>
#include <QColorDialog>
#include <QMenu>
#include <QPoint>
#include <QPointer>

class CWizExplorerApp;
class CWizToolButton;
class CWizToolButtonColor;
class CWizToolComboBox;
class CWizToolComboBoxFont;

struct WizEditorContextMenuItem;

class CWizEditorToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit CWizEditorToolBar(CWizExplorerApp& app, QWidget *parent = 0);

    // editor status reflect
    void resetToolbar();
    void resetContextMenuAndPop(const QPoint& pos);

protected:
    QSize sizeHint() const;

private:
    CWizExplorerApp& m_app;
    std::map<QString, QAction*> m_actions;
    QPointer<QMenu> m_menuContext;

    CWizToolComboBoxFont* m_comboFontFamily;
    CWizToolComboBox* m_comboFontSize;
    CWizToolButtonColor* m_btnForeColor;
    CWizToolButton* m_btnBold;
    CWizToolButton* m_btnItalic;
    CWizToolButton* m_btnUnderLine;
    CWizToolButton* m_btnJustifyLeft;
    CWizToolButton* m_btnJustifyCenter;
    CWizToolButton* m_btnJustifyRight;
    CWizToolButton* m_btnUnorderedList;
    CWizToolButton* m_btnOrderedList;
    CWizToolButton* m_btnTable;
    CWizToolButton* m_btnHorizontal;

    WizEditorContextMenuItem* contextMenuData();
    void buildMenu();
    int buildMenu(QMenu* pMenu, int indx);

protected Q_SLOTS:
    void on_actionFormatFontFamily_activated(const QString& strFamily);
    void on_actionFormatFontSize_activated(const QString& strSize);
};

#endif // WIZEDITORTOOLBAR_H
