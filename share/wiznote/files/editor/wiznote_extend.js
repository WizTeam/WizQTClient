var
    editor = null,
    m_inited = false;
    m_modified = false,
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
    
    var saveButton = new baidu.editor.ui.Button({
        className: 'edui-for-mybutton',
        title: strSave,
        label: strSave,
        showText: false,
        onclick: function() {
            saveDocument(false);
        }
    });

    var editorOption = {
    toolbars:
    [
        [saveButton, '|',// 'Source',
        'FontFamily', 'FontSize', '|', 'Bold', 'Italic', 'Underline', 'StrikeThrough', '|',
        'JustifyLeft', 'JustifyCenter', 'JustifyRight', 'JustifyJustify', '|',
        'Indent', 'Outdent', 'InsertOrderedList', 'InsertUnorderedList', '|',
        'Link', 'ForeColor', 'BackColor', '|', 'horizontal', 'inserttable', 'formatmatch', 'RemoveFormat']
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
    initialStyle:'body{font-size:14px}',
    elementPathEnabled: false,
    wordCount: false,
    imagePopup: false,
    maximumWords: 100000000,
    fullscreen: true
    };

    editor = new baidu.editor.ui.Editor(editorOption);
    editor.render('editorArea');

    editor.addListener('contentChange', function() {
        setModified(true);
    });

    editor.addListener("sourceModeChanged",function(type,mode){
        m_bIsSourceMode = mode;
        alert("change");
    });

    editor.addListener('keydown', onEditorKeyDown);
} catch (err) {
    alert(err);
}

function onEditorContentChanged()
{
}

function onEditorKeyDown(type, evt) {
    if (!isEditing())
        return;

    var e = evt;
    //if (!e.ctrlKey && !e.altKey && !e.shiftKey) {
    //    setModified(true);
    //}

    if (e.ctrlKey && !e.altKey && !e.shiftKey) {
        if (String.fromCharCode(e.keyCode).toLocaleUpperCase() == 'X') {
            setModified(true);
            e.returnValue = true;
        }
        if (String.fromCharCode(e.keyCode).toLocaleUpperCase() == 'S') {
            saveDocument(true);
            e.returnValue = false;
        }
        else if (String.fromCharCode(e.keyCode).toLocaleUpperCase() == 'V') {
            if (autoPaste()) {
                e.returnValue = false;
                e.keyCode = 0;
            }
            else {
            }
        }
    }
}

function autoPaste()
{
    var imageFileName = objCommon.ClipboardToImage(0, "");
    if (imageFileName == null || imageFileName == "")
        return false;
    
    var html = "<img border=\"0\" src=\"file:///" + imageFileName + "\" />";
    editor.execCommand("inserthtml", html);
    return true;
}


function setModified(flag)
{
    m_modified = flag;
    if (!isEditing()) {
        m_modified = false;
        return;
    }
    objApp.SetDocumentModified(m_modified);
}

function setEditorHtml(html) {
    editor.reset();

    // This Api have bug when showing pictures, don't use it!
    //editor.setContent(html);
    editor.document.body.innerHTML = html;
    setModified(false);

    window.UE.utils.domReady(function() {
        setEditorFocus();
        editor.window.scrollTo(0, 0);
    });

    //editor.document.body.innerHTML = '<p>' + html + '</p>';
}

function getEditorHtml() {
    // This Api have bug when showing pictures, don't use it!
    //return editor.getContent();
    return editor.getContent();

    //return editor.document.documentElement.outerHTML;
}

function setEditorFocus() {
    editor.focus();
    //var range = editor.selection.getRange();
    //range.collapse();
    //range.select();
    //editor._selectionChange();
}

function isEditing() {
    return m_editingMode;
}

function setEditing(mode) {
    if (isEditing()) {
        saveDocument(false);
    }

    if (m_editingMode == mode) {
        return;
    }

    document.getElementsByClassName("edui-editor-toolbarbox")[0].style.display = mode ? "block" : "none";
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

function updateDocument(objDocument, html, url, flags)
{
    try {
        return objDocument.UpdateDocument4(html, url, flags);
    } catch (err) {
        return false;
    }
}

function saveDocument(force)
{
    try {
        if (!force && !m_modified)
            return true;
        if (!isEditing())
            return true;
        
        var objFolder = objApp.Window.CategoryCtrl.SelectedFolder;
        if (objFolder == null) {
            objFolder = objDatabase.GetFolderByLocation("/My Notes/", true);
        }

        var objDocument = objDatabase.DocumentFromGUID(m_currentGUID);
        if (objDocument == null || objDocument == "") {
            alert("wow, why the guid is null?");
            return false;
        }
        
        var html = getEditorHtml();
        
        objApp.SetSavingDocument(true);
        var ret = updateDocument(objDocument, html, m_currentFileName, 0);
        objApp.SetSavingDocument(false);
        setModified(false);

        return ret;
    } catch (err) {
        alert(err);
    }
}

