var
    editor = null,
    m_inited = false;
    objApp = WizExplorerApp,
    objDatabase = objApp.Database,
    objCommon = objApp.CreateWizObject("WizKMControls.WizCommonUI"),
    m_currentGUID = "",
    m_currentFileName = "",
    m_editingMode = true;
    m_bIsSourceMode = false;


// setup ueditor
try {
    var strSave = "Save";

    var editorOption = {
    toolbars:
    [
        [//saveButton, '|', //'Source',
        'FontFamily', 'FontSize', '|', 'BackColor', 'ForeColor', 'Bold', 'Italic', 'Underline', '|',
        'JustifyLeft', 'JustifyCenter', 'JustifyRight', '|',
        'InsertOrderedList', 'InsertUnorderedList', '|',
        'horizontal', 'inserttable', 'formatmatch']
    ],
    'fontfamily':[
        { label:'',name:'songti',val:'宋体,SimSun'},
        { label:'',name:'kaiti',val:'楷体,楷体_GB2312, SimKai'},
        { label:'',name:'heiti',val:'黑体, SimHei'},
        { label:'',name:'lishu',val:'隶书, SimLi'},
        { label:'',name:'andaleMono',val:'andale mono'},
        { label:'',name:'arial',val:'arial, helvetica,sans-serif'},
        { label:'',name:'arialBlack',val:'arial black,avant garde'},
        { label:'',name:'comicSansMs',val:'comic sans ms'},
        { label:'',name:'impact',val:'impact,chicago'},
        { label:'',name:'timesNewRoman',val:'times new roman'}
    ],
    'fontsize':[9, 10, 11, 12, 13, 14, 16, 18, 24, 36, 48],
    initialStyle:'body{font-size:13px}',
    contextMenu: [],
    elementPathEnabled: false,
    wordCount: false,
    imagePopup: false,
    maximumWords: 100000000,
    };

    editor = new baidu.editor.ui.Editor(editorOption);
    objApp.ResetInitialStyle();
    editor.render('editorArea');

    editor.addListener('selectionChange', function() {
        objApp.ResetEditorToolBar();
    });

    editor.addListener('beforePaste', function(type, data) {
        objApp.ProcessClipboardBeforePaste(data);
    });

    editor.addListener("sourceModeChanged",function(type,mode){
        m_bIsSourceMode = mode;
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

    //editor.setContent(html);
    editor.document.body.innerHTML = html;

    window.UE.utils.domReady(function() {
        editor.window.scrollTo(0, 0);
    });

    editor.ui.setFullScreen(true);

    editor.addListener('contentChange', setModified);
}

function getEditorHtml() {
    return editor.getContent();
    //return editor.document.documentElement.outerHTML;
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
