﻿#ifndef WIZEDITORTOOLBAR_H
#define WIZEDITORTOOLBAR_H

#include <QWidget>
#include <QPointer>
#include <QMap>
#include <QTimer>
#include <functional>
#include <QToolButton>
#include <QDateTime>

class QFontDialog;
class QString;
class QMenu;
class WizDocumentWebEngine;
class WizDocumentWebView;
class CWizToolButton;
class CWizToolButtonColor;
class CWizToolComboBox;
class CWizToolComboBoxFont;
class WizExplorerApp;
class WizTipsWidget;
class CWizToolLineWidthComboBox;
class QWidgetAction;


class WizDblclickableToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit WizDblclickableToolButton(QWidget *parent = Q_NULLPTR);
protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    QDateTime m_tDblClicked;
Q_SIGNALS:
    void dblClicked();
};


class WizEditorToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit WizEditorToolBar(WizExplorerApp& app, QWidget *parent);
    void setDelegate(WizDocumentWebView* editor);

    bool hasFocus();

    void adjustButtonPosition();
    void switchToNormalMode();
    void applyTheme();
    //
    WizTipsWidget* showCoachingTips();

private:
    WizExplorerApp& m_app;
    //
    QString m_strToolbarType;

    WizDocumentWebView* m_editor;
    QMap<QString, QAction*> m_actions;
    CWizToolComboBox* m_comboParagraph;
    CWizToolComboBox* m_comboFontFamily;
    CWizToolComboBox* m_comboFontSize;
    CWizToolButtonColor* m_btnForeColor;
    CWizToolButtonColor* m_btnBackColor;
    CWizToolButton* m_btnFormatPainter;
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
    CWizToolButton* m_btnStartMarkup;
    CWizToolButton* m_btnInsertPainter;
    CWizToolButton* m_btnInsertDate;
    CWizToolButton* m_btnSearchReplace;
    CWizToolButton* m_btnMobileImage;
    CWizToolButton* m_btnScreenShot;
    CWizToolButton* m_btnInsertCode;
    CWizToolButton* m_btnShowExtra;
    QMenu* m_menuJustify;
    QAction* m_actionJustifyLeft;
    QAction* m_actionJustifyCenter;
    QAction* m_actionJustifyRight;
    QWidgetAction* m_tableAction;

    QString m_strImageSrc;

    QWidget* m_firstLineButtonContainer;
    QWidget* m_secondLineButtonContainer;
    //
    //
    //markup tools
    CWizToolButtonColor* m_btnPencil;
    CWizToolLineWidthComboBox* m_comboPencil;
    CWizToolButtonColor* m_btnHighlighter;
    CWizToolLineWidthComboBox* m_comboHighlighter;
    CWizToolButton* m_btnEraser;
    CWizToolLineWidthComboBox* m_comboEraser;
    CWizToolButtonColor* m_btnShape;
    CWizToolLineWidthComboBox* m_comboShape;
    CWizToolButton* m_btnPenWidth;
    CWizToolButton* m_btnUndo;
    CWizToolButton* m_btnRedo;
    CWizToolButton* m_btnGoback;
    //
    //outline tools
    CWizToolButton* m_btnUndoOutline;
    CWizToolButton* m_btnRedoOutline;
    CWizToolButton* m_btnOutdentOutline;
    CWizToolButton* m_btnIndentOutline;
    CWizToolButton* m_btnNotesOutline;
    CWizToolButton* m_btnInsertImagesOutline;
    CWizToolButton* m_btnCompleteOutline;
    //text input would call resetToolbar and cause input delay, lock to ignore reset request
    QString m_currentStyle;
#ifdef Q_OS_WIN
    __int64 m_lastUpdateUIRequest;
#else
    int64_t m_lastUpdateUIRequest;
#endif
    QTimer m_delayUpdateUITimer;

    // editor status reflect
    void resetToolbar(const QString& currentStyle);
    //
    bool avaliableInToolbar(QWidget* widget);
    void setAvaliableInToolbar(QWidget* widget, QString type);
    void setToolbarType(QString type);
    void setPainterToolType(QString type);

    QAction* actionFromName(const QString& strName);

    bool processImageSrc(bool bUseForCopy, bool& bNeedSubsequent);
    bool processBase64Image(bool bUseForCopy);
    void savePixmap(QPixmap& pix, const QString& strType, bool bUseForCopy);
    void saveGif(const QByteArray& ba);

    QMenu* createColorMenu(const char *slot, const char *slotColorBoard = nullptr);
    QMenu* createLineWidthMenu(const char *slot);

    //QList<QWidget*> m_buttonContainersInFirstLine;
    //QList<QWidget*> m_buttonContainersInSecondLine;
    QList<QWidget*> buttonContainersInFirstLine();
    QList<QWidget*> buttonContainersInSecondLine();
    //
    int firstLineWidth();
    int secondLineWidth();

protected Q_SLOTS:
    void on_editor_google_triggered();
    void on_editor_baidu_triggered();
    void on_editor_cut_triggered();
    void on_editor_copy_triggered();
    void on_editor_paste_triggered();
    void on_editor_pastePlain_triggered();
    void on_editor_bold_triggered();
    void on_editor_italic_triggered();
    void on_editor_underline_triggered();
    void on_editor_strikethrough_triggered();
    void on_editor_insertLink_triggered();
    void on_editor_editLink_triggered();
    void on_editor_removeLink_triggered();
    void on_editor_justifyLeft_triggered();
    void on_editor_justifyCenter_triggered();
    void on_editor_justifyRight_triggered();


    void on_comboParagraph_shortcutActivated();
    void on_comboParagraph_indexChanged(int index);
    void on_comboFontFamily_indexChanged(int index);
    void on_comboFontSize_indexChanged(const QString& strSize);
    void on_btnFormatPainter_clicked(bool checked);
    void on_btnFormatPainter_dblClicked();
    void on_btnRemoveFormat_clicked();
    void on_btnForeColor_clicked();
    void on_btnBackColor_clicked();
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
    void on_btnTable_clicked(int row, int col);
    void on_btnHorizontal_clicked();
    void on_btnCheckList_clicked();
    void on_btnInsertImage_clicked();
    void on_btnStartMarkup_clicked();
    void on_btnInsertPainter_clicked();
    void on_btnInsertLink_clicked();
    void on_btnInsertDate_clicked();
    void on_btnMobileImage_clicked();
    void on_btnScreenShot_clicked();
    void on_btnInsertCode_clicked();
    void on_btnShowExtra_clicked();
    void on_editor_saveImageAs_triggered();
    void on_editor_copyImage_triggered();
    void on_editor_copyImageLink_triggered();

    void on_delegate_showContextMenuRequest(const QPoint& pos);
    void on_delegate_selectionChanged(const QString&);
    void on_delegate_markerUndoStatusChanged(const QString&);
    void on_delegate_markerInitiated(const QString&);
    void on_delay_updateToolbar();

    void on_foreColor_changed();
    void on_showForeColorBoard();
    void on_backColor_changed();
    void on_showBackColorBoard();
    void applyForeColor(const QColor& color);
    void applyBackColor(const QColor& color);

    void on_fontDailogFontChanged(const QFont & font);    
    //
    //
    void on_btnPencil_clicked();
    void on_btnPencilColor_changed();
    void on_btnHighlighter_clicked();
    void on_btnHighlighterColor_changed();
    void on_btnEraser_clicked();
    void on_btnShape_clicked();
    void on_btnShapeColor_changed();
    void on_btnUndo_clicked();
    void on_btnRedo_clicked();
    void on_btnGoback_clicked();
    //
    void on_btnUndoOutline_clicked();
    void on_btnRedoOutline_clicked();
    void on_btnIndentOutline_clicked();
    void on_btnOutdentOutline_clicked();
    void on_btnNotesOutline_clicked();
    void on_btnCompleteOutline_clicked();
    //
    void on_pencilSize_activated(int);
    void on_highlighterSize_activated(int);
    void on_eraserSize_activated(int);
    void on_shapeSize_activated(int);

private:
    void queryCurrentFont(std::function<void(const QFont& font)> callback);
    void setCurrentFont(const QFont& font);
    void selectCurrentFontFamily(const QString& strFontFamily);
    void selectCurrentFontFamilyItem(const QString& strFontFamily);
    void setFontPointSize(const QString& strSize);

    void saveImage(QString strFileName);
    void copyImage(QString strFileName);

    void moveWidgetFromSecondLineToFirstLine(QWidget* widget);
    void moveWidgetFromFristLineToSecondLine(QWidget* widget);

};



#endif // WIZEDITORTOOLBAR_H
