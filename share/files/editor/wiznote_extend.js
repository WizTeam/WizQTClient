var
    editor = null,
    m_inited = false;
    objApp = WizExplorerApp,
    objCommon = objApp.CreateWizObject("WizKMControls.WizCommonUI"),
    m_currentGUID = "",
    m_currentFileName = "",
    m_header = null,
    m_html = null;


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
        setTimeout(function() {WizEditor.onNoteLoadFinished();}, 3000);
    });

} catch (err) {
    alert(err);
}

function setEditorHtml(html, bEditing)
{
    editor.reset();
    editor.setContent(html);
    editor.document.head = m_header; // restore original header
    bEditing ? editor.setEnabled() : editor.setDisabled();

    window.UE.utils.domReady(function() {
        editor.window.scrollTo(0, 0);
    });
}

function setEditing(bEditing) {
    editor.setContent(m_html);
    editor.document.head = m_header; // restore original header
    bEditing ? editor.setEnabled() : editor.setDisabled();
}

function viewDocument(guid, filename, bEditing)
{
    try {
        m_currentGUID = guid;
        m_currentFileName = filename;

        var html = objCommon.LoadTextFromFile(filename);
        var body = html.match(/<body[\S\s]*<\/body>/g);
        if (body) {
            m_html = body;
        } else {
            m_html = html;
        }

        console.log(m_html);

        //var root = window.UE.htmlparser(html);
        //editor.filterInputRule(root);
        //m_html = root.toHtml();

        if (m_inited) {
            setEditorHtml(m_html, bEditing);
        } else {
            editor.ready(function() {
                m_header = editor.document.head; // save original header
                setEditorHtml(m_html, bEditing);
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
