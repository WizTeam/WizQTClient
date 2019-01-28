#ifndef WIZSEARCHWIDGET_MM_H
#define WIZSEARCHWIDGET_MM_H


#ifdef USECOCOATOOLBAR

#include "WizMacHelper.h"
#include <QTreeWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTimer;
class QTreeWidget;
class WizUserSettings;
QT_END_NAMESPACE

class WizSuggestCompletionon;

const int TOOLBARITEMHEIGHT =  23;
const int NORMALSEARCHWIDGETWIDTH = 292;
const int HIGHPIXSEARCHWIDGETWIDTH = 270;


class WizSearchView : public WizCocoaViewContainer
{
    Q_OBJECT

public:
    WizSearchView();
    void clear();
    void clearFocus();
    bool hasFocus();
    void focus();
    void onFocused(bool focused);
    void setText(const QString& text);
    QString currentText();
    //
    void setCurrentKb(const QString& kbGuid);
    QString currentKb() const { return m_strCurrentKbGuid; }
    //
    void setSearchPlaceHolder(const QString& placeHolder);

    void setUserSettings(WizUserSettings* settings);

    bool isEditing();

    void setCompleterUsable(bool usable);
    bool isCompleterVisible();
    void showCompleter();
    void hideCompleter();
    void moveCompleter(bool up);
    QString getCurrentCompleterText();

    void setSizeHint(QSize sizeHint);
    virtual QSize sizeHint() const;
    QRect globalRect();

public Q_SLOTS:
    void on_search_editFinished(const QString& strText);
    void on_search_textChanged(const QString& strText);
    void on_search_textChanging();
    void setFocus();
    void clearSearchFocus();

Q_SIGNALS:
    void doSearch(const QString& keywords);
    void textEdited(const QString& text);
    void textStartEditing();
    void textStopEditing();
    void textFocused(bool focused);

private:
    WizSuggestCompletionon* m_completer;
    QSize m_sizeHint;
    QString m_strCurrentKbGuid;
};

class WizSuggestiongList : public QTreeWidget
{
    Q_OBJECT

public:
    WizSuggestiongList(QWidget* parent = 0);

protected:
    void mouseMoveEvent(QMouseEvent* event);
    void leaveEvent(QEvent* ev);
};

class WizSuggestionSeacher : public QObject
{
    Q_OBJECT
public:
    WizSuggestionSeacher(QObject* parent = 0);

    void searchSuggestion(const QString& kbGuid, const QString& inputText);

signals:
    void searchFinished(const QStringList &choices, bool isRecentSearches);
};

class WizSuggestCompletionon : public QObject
{
    Q_OBJECT

public:
    WizSuggestCompletionon(WizSearchView *parent = 0);
    ~WizSuggestCompletionon();

    void setUserSettings(WizUserSettings* settings);
    void setCurrentKb(const QString& kbGuid) { m_strCurrentKbGuid = kbGuid; updatePlaceHolder(); }

    bool eventFilter(QObject *obj, QEvent *ev) Q_DECL_OVERRIDE;

    void setUsable(bool usable);
    bool isVisible();
    void hide();
    void selectSuggestItem(bool up);
    QString getCurrentText();
    void updatePlaceHolder();

public slots:
    void showCompletion(const QStringList &choices, bool isRecentSearches);

    void doneCompletion();
    void preventSuggest();
    void autoSuggest();
    void textChanged(QString text);
    void textFocused(bool focused);
    void startEditing();
    void stopEditing();
    void on_advanced_buttonClicked();

private:
    void resetContainerSize(int width, int height);

private:
    WizSearchView *m_editor;
    QWidget *m_popupWgt;
    QWidget *m_infoWgt;
    QTreeWidget *m_treeWgt;
    QTimer *m_timer;
    bool m_usable;
    QString m_strCurrentKbGuid;
    bool m_editing;
    bool m_focused;

    WizSuggestionSeacher* m_searcher;

    WizUserSettings* m_settings;
};

#endif

#endif // WIZSEARCHWIDGET_MM_H
