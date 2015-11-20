#ifndef WIZSEARCHWIDGET_MM_H
#define WIZSEARCHWIDGET_MM_H


#ifdef USECOCOATOOLBAR

#include <QMacCocoaViewContainer>
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTimer;
class QTreeWidget;
class CWizUserSettings;
QT_END_NAMESPACE

class WizSuggestCompletionon;

const int TOOLBARITEMHEIGHT =  23;
const int NORMALSEARCHWIDGETWIDTH = 292;
const int HIGHPIXSEARCHWIDGETWIDTH = 270;

class CWizSearchWidget : public QMacCocoaViewContainer
{
    Q_OBJECT

public:
    CWizSearchWidget(QWidget* parent = 0);
    void clear();
    void clearFocus();
    void focus();
    void setText(const QString& text);
    QString currentText();

    void setUserSettings(CWizUserSettings* settings);

    bool isEditing();

    void setCompleterUsable(bool usable);
    bool isCompleterVisible();
    void showCompleter();
    void hideCompleter();
    void moveCompleter(bool up);
    QString getCurrentCompleterText();

    void setPopupWgtOffset(int popupWgtWidth, const QSize& offset);

    void setSizeHint(QSize sizeHint);
    virtual QSize sizeHint() const;

    void processEvent(QEvent* ev);

public Q_SLOTS:
    void on_search_editFinished(const QString& strText);
    void on_search_textChanged(const QString& strText);
    void setFocus();
    void clearSearchFocus();
    void on_advanced_buttonClicked();

Q_SIGNALS:
    void doSearch(const QString& keywords);
    void textEdited(const QString& text);
    void advancedSearchRequest();

private:
    WizSuggestCompletionon* m_completer;
    QSize m_sizeHint;
};

class CWizSuggestiongList : public QTreeWidget
{
    Q_OBJECT

public:
    CWizSuggestiongList(QWidget* parent = 0);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* ev);
};


class WizSuggestCompletionon : public QObject
{
    Q_OBJECT

public:
    WizSuggestCompletionon(CWizSearchWidget *parent = 0);
    ~WizSuggestCompletionon();

    void setUserSettings(CWizUserSettings* settings);

    bool eventFilter(QObject *obj, QEvent *ev) Q_DECL_OVERRIDE;

    void setUsable(bool usable);
    bool isVisible();
    void hide();
    void selectSuggestItem(bool up);
    QString getCurrentText();

    void setPopupOffset(int popupWgtWidth, const QSize& offset);

public slots:
    void showCompletion(const QStringList &choices, bool isRecentSearches);

    void doneCompletion();
    void preventSuggest();
    void autoSuggest();

private:
    void resetContainerSize(int width, int height);

private:
    CWizSearchWidget *m_editor;
    QWidget *m_popupWgt;
    QWidget *m_infoWgt;
    QTreeWidget *m_treeWgt;
    QTimer *m_timer;
    QSize m_popupOffset;
    int m_popupWgtWidth;
    bool m_usable;

    CWizUserSettings* m_settings;
};

#endif

#endif // WIZSEARCHWIDGET_MM_H
