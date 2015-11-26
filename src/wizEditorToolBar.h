#ifndef WIZEDITORTOOLBAR_H
#define WIZEDITORTOOLBAR_H

#include <QWidget>
#include <QPointer>
#include <QMap>
#include <QTimer>

class QFontDialog;
class QString;
class QMenu;
class CWizDocumentWebEngine;
class CWizDocumentWebView;
class CWizToolButton;
class CWizToolButtonColor;
class CWizToolComboBox;
class CWizToolComboBoxFont;
class CWizExplorerApp;

struct WizEditorContextMenuItem;

namespace Core {
namespace Internal {


class EditorToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit EditorToolBar(CWizExplorerApp& app, QWidget *parent);
#ifdef USEWEBENGINE
    void setDelegate(CWizDocumentWebEngine* editor);
#else
    void setDelegate(CWizDocumentWebView* editor);
#endif

    bool hasFocus();

    void adjustButtonPosition();
    //
    void showCoachingTips();

private:
    CWizExplorerApp& m_app;

#ifdef USEWEBENGINE
    CWizDocumentWebEngine* m_editor;
#else
    CWizDocumentWebView* m_editor;
#endif
    QMap<QString, QAction*> m_actions;
    QPointer<QMenu> m_menuContext;
    CWizToolComboBox* m_comboParagraph;
    CWizToolComboBox* m_comboFontFamily;
    CWizToolComboBox* m_comboFontSize;
    CWizToolButtonColor* m_btnForeColor;
    CWizToolButtonColor* m_btnBackColor;
    CWizToolButton* m_btnFormatMatch;
    CWizToolButton* m_btnRemoveFormat;
    CWizToolButton* m_btnBold;
    CWizToolButton* m_btnItalic;
    CWizToolButton* m_btnUnderLine;
    CWizToolButton* m_btnStrikeThrough;
    CWizToolButton* m_btnJustify;
    CWizToolButton* m_btnUnorderedList;
    CWizToolButton* m_btnOrderedList;
    CWizToolButton* m_btnTable;
    CWizToolButton* m_btnHorizontal;
    CWizToolButton* m_btnCheckList;
    CWizToolButton* m_btnInsertLink;
    CWizToolButton* m_btnInsertImage;
    CWizToolButton* m_btnInsertDate;
    CWizToolButton* m_btnSearchReplace;
    CWizToolButton* m_btnMobileImage;
    CWizToolButton* m_btnScreenShot;
    CWizToolButton* m_btnViewSource;
    CWizToolButton* m_btnInsertCode;
    CWizToolButton* m_btnShowExtra;
    QMenu* m_menuJustify;
    QAction* m_actionJustifyLeft;
    QAction* m_actionJustifyCenter;
    QAction* m_actionJustifyRight;

    QString m_strImageSrc;

    QWidget* m_firstLineButtonContainer;
    QWidget* m_secondLineButtonContainer;

    //text input would call resetToolbar and cause input delay, lock to ignore reset request
    bool m_resetLocked;
    QTimer m_resetLockTimer;

    WizEditorContextMenuItem* contextMenuData();
    void buildMenu();
    int buildMenu(QMenu* pMenu, int indx);

    // editor status reflect
    void resetToolbar();

    QAction* actionFromName(const QString& strName);

    bool processImageSrc(bool bUseForCopy, bool& bNeedSubsequent);
    bool processBase64Image(bool bUseForCopy);
    void savePixmap(QPixmap& pix, const QString& strType, bool bUseForCopy);
    void saveGif(const QByteArray& ba);

    QMenu* createColorMenu(const char *slot, const char *slotColorBoard);

    QList<QWidget*> m_buttonContainersInFirstLine;
    QList<QWidget*> m_buttonContainersInSecondLine;

protected Q_SLOTS:
    void on_editor_google_triggered();
    void on_editor_baidu_triggered();
    void on_editor_cut_triggered();
    void on_editor_copy_triggered();
    void on_editor_paste_triggered();
    void on_editor_bold_triggered();
    void on_editor_italic_triggered();
    void on_editor_underline_triggered();
    void on_editor_strikethrough_triggered();
    void on_editor_insertLink_triggered();
    void on_editor_editLink_triggered();
    void on_editor_removeLink_triggered();
    void on_editor_insertTable_triggered();
    void on_editor_deleteTable_triggered();
    void on_editor_justifyLeft_triggered();
    void on_editor_justifyCenter_triggered();
    void on_editor_justifyRight_triggered();


    void on_comboParagraph_indexChanged(int index);
    void on_comboFontFamily_indexChanged(int index);
    void on_comboFontSize_indexChanged(const QString& strSize);
    void on_btnFormatMatch_clicked();
    void on_btnRemoveFormat_clicked();
    void on_btnBold_clicked();
    void on_btnItalic_clicked();
    void on_btnUnderLine_clicked();
    void on_btnStrikeThrough_clicked();
    void on_btnJustify_clicked();
    void on_btnJustifyLeft_clicked();
    void on_btnJustifyCenter_clicked();
    void on_btnJustifyRight_clicked();
    void on_btnSearchReplace_clicked();
    void on_btnUnorderedList_clicked();
    void on_btnOrderedList_clicked();
    void on_btnTable_clicked();
    void on_btnHorizontal_clicked();
    void on_btnCheckList_clicked();
    void on_btnInsertImage_clicked();
    void on_btnInsertLink_clicked();
    void on_btnInsertDate_clicked();
    void on_btnMobileImage_clicked();
    void on_btnScreenShot_clicked();
    void on_btnViewSource_clicked();
    void on_btnInsertCode_clicked();
    void on_btnShowExtra_clicked();
    void on_editor_saveImageAs_triggered();
    void on_editor_copyImage_triggered();
    void on_editor_copyImageLink_triggered();

    void on_delegate_showContextMenuRequest(const QPoint& pos);
    void on_delegate_selectionChanged();

    void on_updateToolBarStatus_request();
    void on_resetLockTimer_timeOut();

    void on_foreColor_changed();
    void on_showForeColorBoard();
    void on_backColor_changed();
    void on_showBackColorBoard();

    void on_fontDailogFontChanged(const QFont & font);    

private:
    void queryCurrentFont(QFont& font);
    void setCurrentFont(const QFont& font);
    void selectCurrentFontFamily(const QString& strFontFamily);
    void selectCurrentFontFamilyItem(const QString& strFontFamily);
    void setFontPointSize(const QString& strSize);

    void saveImage(QString strFileName);
    void copyImage(QString strFileName);

    void moveWidgetFromSecondLineToFirstLine(QWidget* widget);
    void moveWidgetFromFristLineToSecondLine(QWidget* widget);

};


} // namespace Internal
} // namespace Core

#endif // WIZEDITORTOOLBAR_H
