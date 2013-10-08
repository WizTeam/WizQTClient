var
    editor = null,
    m_inited = false;
    objApp = WizExplorerApp,
    objCommon = objApp.CreateWizObject("WizKMControls.WizCommonUI"),
    m_currentGUID = "",
    m_currentFileName = "",
    m_editingMode = true;
    m_bIsSourceMode = false;


// setup ueditor
try {
    var strSave = "Save";

    var editorOption = {
    toolbars: [],
    initialStyle: 'body{font-size:13px}',
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
    objApp.ResetInitialStyle();
    editor.render('editorArea');

    editor.addListener('beforePaste', function(type, data) {
        objApp.ProcessClipboardBeforePaste(data);
    });

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

} catch (err) {
    alert(err);
}

function setModified()
{
    objApp.SetDocumentModified(true);
}

function setEditorHtml(html)
{
    editor.removeListener('contentChange', setModified);

    editor.reset();

    editor.setContent(html);

    window.UE.utils.domReady(function() {
        editor.window.scrollTo(0, 0);
    });

    editor.addListener('contentChange', setModified);
}

function setEditing(mode) {
    if (m_editingMode == mode) {
        return;
    }

    mode ? editor.setEnabled() : editor.setDisabled();
    m_editingMode = mode;
}

function viewDocument(guid, filename, mode)
{
    try {
        m_currentGUID = guid;
        m_currentFileName = filename;

        var html = objCommon.LoadTextFromFile(filename);

        if (m_inited) {
            setEditorHtml(html);
            setEditing(mode);
        } else {
            editor.ready(function() {
                setEditorHtml(html);
                setEditing(mode);
                m_inited = true;
            });
        }
        
        return true;
    } catch (err) {
        alert(err);
        return false;
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
