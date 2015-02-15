#ifndef WIZEDITORTOOLBAR_H
#define WIZEDITORTOOLBAR_H

#include <QWidget>
#include <QPointer>
#include <QMap>

class QString;
class QMenu;
class CWizDocumentWebView;
class CWizToolButton;
class CWizToolButtonColor;
class CWizToolComboBox;
class CWizToolComboBoxFont;

struct WizEditorContextMenuItem;

namespace Core {
namespace Internal {


class EditorToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit EditorToolBar(QWidget *parent);
    void setDelegate(CWizDocumentWebView* editor);

    bool hasFocus();

protected:
    QSize sizeHint() const;

private:
    CWizDocumentWebView* m_editor;
    QMap<QString, QAction*> m_actions;
    QPointer<QMenu> m_menuContext;
    CWizToolComboBoxFont* m_comboFontFamily;
    CWizToolComboBox* m_comboFontSize;
    CWizToolButtonColor* m_btnForeColor;
    CWizToolButtonColor* m_btnBackColor;
    CWizToolButton* m_btnFormatMatch;
    CWizToolButton* m_btnBold;
    CWizToolButton* m_btnItalic;
    CWizToolButton* m_btnUnderLine;
    CWizToolButton* m_btnStrikeThrough;
    CWizToolButton* m_btnJustifyLeft;
    CWizToolButton* m_btnJustifyCenter;
    CWizToolButton* m_btnJustifyRight;
    CWizToolButton* m_btnUnorderedList;
    CWizToolButton* m_btnOrderedList;
    CWizToolButton* m_btnTable;
    CWizToolButton* m_btnHorizontal;
    CWizToolButton* m_btnCheckList;
    CWizToolButton* m_btnInsertImage;
    CWizToolButton* m_btnSearchReplace;
    CWizToolButton* m_btnMobileImage;
    CWizToolButton* m_btnScreenShot;
    CWizToolButton* m_btnViewSource;
    CWizToolButton* m_btnInsertCode;

    QString m_strImageSrc;

    WizEditorContextMenuItem* contextMenuData();
    void buildMenu();
    int buildMenu(QMenu* pMenu, int indx);

    // editor status reflect
    void resetToolbar();

    QAction* actionFromName(const QString& strName);

    bool processImageSrc(bool bUseForCopy, bool& bNeedSubsequent);
    bool processBase64Image(bool bUseForCopy);

protected Q_SLOTS:
    void on_editor_google_triggered();
    void on_editor_baidu_triggered();
    void on_editor_cut_triggered();
    void on_editor_copy_triggered();
    void on_editor_paste_triggered();

    void on_comboFontFamily_indexChanged(const QString& strFamily);
    void on_comboFontSize_indexChanged(const QString& strSize);
    void on_btnFormatMatch_clicked();
    void on_BtnForeColor_clicked();
    void on_BtnBackColor_clicked();
    void on_btnBold_clicked();
    void on_btnItalic_clicked();
    void on_btnUnderLine_clicked();
    void on_btnStrikeThrough_clicked();
    void on_btnJustifyLeft_clicked();
    void on_btnJustifyCenter_clicked();
    void on_btnJustifyRight_clicked();
    void on_btnSearchReplace_clicked();
    void on_btnUnorderedList_clicked();
    void on_btnOrderedList_clicked();
    void on_btnTable_clicked();
    void on_btnHorizontal_clicked();
    void on_btnCheckList_clicked();
    void on_btnImage_clicked();
    void on_btnMobileImage_clicked();
    void on_btnScreenShot_clicked();
    void on_btnViewSource_clicked();
    void on_btnInsertCode_clicked();
    void on_editor_saveImageAs_triggered();
    void on_editor_copyImage_triggered();
    void on_editor_copyImageLink_triggered();

    void on_delegate_requestShowContextMenu(const QPoint& pos);
    void on_delegate_selectionChanged();

    void saveImage(QString strFileName);
    void copyImage(QString strFileName);
};


} // namespace Internal
} // namespace Core

#endif // WIZEDITORTOOLBAR_H
