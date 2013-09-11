#ifndef WIZEDITORTOOLBAR_H
#define WIZEDITORTOOLBAR_H

#include <QWidget>
#include <QComboBox>
#include <QColorDialog>
#include <QMenu>
#include <QPoint>
#include <QPointer>

class CWizExplorerApp;
class CWizDocumentWebView;
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
    void setDelegate(CWizDocumentWebView* editor);

protected:
    QSize sizeHint() const;

private:
    CWizExplorerApp& m_app;
    CWizDocumentWebView* m_editor;
    std::map<QString, QAction*> m_actions;
    QPointer<QMenu> m_menuContext;
    CWizToolComboBoxFont* m_comboFontFamily;
    CWizToolComboBox* m_comboFontSize;
    CWizToolButtonColor* m_btnForeColor;
    CWizToolButtonColor* m_btnBackColor;
    CWizToolButton* m_btnFormatMatch;
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

    // editor status reflect
    void resetToolbar();

protected Q_SLOTS:
    void on_comboFontFamily_indexChanged(const QString& strFamily);
    void on_comboFontSize_indexChanged(const QString& strSize);
    void on_btnFormatMatch_clicked();
    void on_BtnForeColor_clicked();
    void on_BtnBackColor_clicked();
    void on_btnBold_clicked();
    void on_btnItalic_clicked();
    void on_btnUnderLine_clicked();
    void on_btnJustifyLeft_clicked();
    void on_btnJustifyCenter_clicked();
    void on_btnJustifyRight_clicked();
    void on_btnUnorderedList_clicked();
    void on_btnOrderedList_clicked();
    void on_btnTable_clicked();
    void on_btnHorizontal_clicked();

    void on_delegate_requestShowContextMenu(const QPoint& pos);
    void on_delegate_selectionChanged();
};

#endif // WIZEDITORTOOLBAR_H
