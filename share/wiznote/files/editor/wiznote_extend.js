
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
        [saveButton,
        '|',
        'FontFamily', 'FontSize', '|', 'Bold', 'Italic', 'Underline', 'StrikeThrough', '|',
        'JustifyLeft', 'JustifyCenter', 'JustifyRight', 'JustifyJustify', '|',
        'Indent', 'Outdent', 'InsertOrderedList', 'InsertUnorderedList', '|',
        'Link', 'ForeColor', 'BackColor', '|', 'RemoveFormat', 'Undo', 'Redo'
        ]
    ],
    elementPathEnabled: false,
    wordCount: false,
    imagePopup: false,
    maximumWords: 100000000,
    fullscreen: true
    };

    editor = new baidu.editor.ui.Editor(editorOption);
    editor.render('editorArea');
    editor.addListener('contentChange', onEditorContentChanged);
    editor.addListener('keydown', onEditorKeyDown);
} catch (err) {
    alert(err);
}

function onEditorContentChanged()
{
    setModified(true);
}

function onEditorKeyDown(type, evt) {
    if (!isEditing())
        return;

    var e = evt;
    if (!e.ctrlKey && !e.altKey && !e.shiftKey) {
        setModified(true);
    }

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
    // This Api have bug when showing pictures, don't use it!
    //editor.setContent(html);

    editor.document.body.innerHTML = '<p>' + html + '</p>';
    editor.undoManger.reset();
    setModified(false);
    editor.window.scrollTo(0,0);
}

function getEditorHtml() {
    //return editor.getContent();

    return editor.document.documentElement.outerHTML;
}

function setEditorFocus() {
    editor.focus();
    var range = editor.selection.getRange();
    range.select(true);
    editor._selectionChange();
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

    if (mode) {
        setTimeout(function() {
            setEditorFocus();
        }, 100);
    }
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

        // FIXME: stub here
        setInterval(function(){
            saveDocument(false);
        }, 5*60*1000);

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

