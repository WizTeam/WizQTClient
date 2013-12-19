var
    editor = null,
    m_inited = false,
    objApp = WizExplorerApp,
    m_currentGUID = "",
    m_defaultCss = WizEditor.getDefaultCssFilePath(),
    m_defaultCssVersion = 1,
    m_header = "",
    wiz_html = "";


// setup ueditor
try {
    var editorOption = {
    toolbars: [],
    iframeCssUrl: m_defaultCss,
    //initialStyle: 'body{font-size:13px}',
    enterTag: 'div',
    fullscreen: true,
    autoHeightEnabled: false,
    scaleEnabled: false,
    contextMenu: [],
    elementPathEnabled: false,
    wordCount: false,
    imagePopup: false,
    maximumWords: 100000000,
    };

    editor = new baidu.editor.ui.Editor(editorOption);
    //objApp.ResetInitialStyle();
    editor.render('editorArea');

    editor.addListener("sourceModeChanged",function(type,mode){
    });

    // hide builtin toolbar, from UE dev team.
    editor.addListener('ready', function () {
        editor.ui.getDom('toolbarbox').style.display = 'none';
        editor.ui.getDom('bottombar').style.display = 'none';
        editor.ui.getDom('scalelayer').style.display = 'none';
        editor.ui.getDom('elementpath').style.display = "none";
        editor.ui.getDom('wordcount').style.display = "none";
        editor.ui._updateFullScreen();

    });

    editor.addListener('aftersetcontent', function() {
        updateCss();
        WizEditor.onNoteLoadFinished();
    });

} catch (err) {
    alert(err);
}

function setEditorHtml(html, bEditing)
{
    editor.reset();
    editor.document.head.innerHTML = m_header; // restore original header

    bEditing ? editor.setEnabled() : editor.setDisabled();
    editor.setContent(html);

    window.UE.utils.domReady(function() {
        editor.window.scrollTo(0, 0);
    });
}

function setEditing(bEditing) {
    editor.document.head.innerHTML = m_header; // restore original header

    bEditing ? editor.setEnabled() : editor.setDisabled();
    editor.setContent(wiz_html);
}

function viewNote(strGUID, bEditing, strHtml)
{
    try {
        m_currentGUID = strGUID;
        wiz_html = strHtml;

        if (m_inited) {
            setEditorHtml(wiz_html, bEditing);
        } else {
            editor.ready(function() {
                m_header = editor.document.head.innerHTML; // save original header
                setEditorHtml(wiz_html, bEditing);
                m_inited = true;
            });
        }
        
        return true;
    } catch (err) {
        alert(err);
        return false;
    }
}

function updateCss()
{
    var css= editor.document.getElementsByTagName('link');
    for (var i = 0; i < css.length; i++) {
        if (css[i].rel != 'stylesheet') return;
        if (css[i].type != 'text/css') return;
        if (css[i].href.match(m_defaultCss)) {
            css[i].href = m_defaultCss + "?v=" + m_defaultCssVersion;
            m_defaultCssVersion++;
        }
    }
}

// helper functions
function WizGetLinkUrl() {
    try {
        var range = editor.selection.getRange(),
            link = range.collapsed ? editor.queryCommandValue( "link" ) : editor.selection.getStart(),
            url;

        link = UE.dom.domUtils.findParentByTagName( link, "a", true );
        if(link){
            url = UE.utils.html(link.getAttribute( 'data_ue_src' ) || link.getAttribute( 'href', 2 ));
        }
    } catch (error) {
        alert(error);
        return "ERROR";
    }

    return url ? url: '';
}
