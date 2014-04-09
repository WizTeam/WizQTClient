var
    editor = null,
    m_inited = false,
    objApp = WizExplorerApp,
    m_currentGUID = "",
    m_defaultCss = WizEditor.getDefaultCssFilePath(),
    m_defaultCssVersion = 1,
    m_header = "",
    wiz_html = "",
    wiz_head = "";


// setup ueditor
try {
    var editorOption = {
    toolbars: [],
    //iframeCssUrl: m_defaultCss,
    //initialStyle: 'body{font-size:13px}',
    //enterTag: 'div',
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
    editor.document.head.innerHTML = wiz_head;
    //if (bEditing) {
    //    editor.document.head.innerHTML = wiz_head + m_header; // restore original header
    //} else {
    //    editor.document.head.innerHTML = wiz_head;
    //}

    editor.document.body.innerHTML = html;
    editor.fireEvent('aftersetcontent');
    editor.fireEvent('contentchange');

    bEditing ? editor.setEnabled() : editor.setDisabled();

    window.UE.utils.domReady(function() {
        editor.window.scrollTo(0, 0);
    });
}


function setEditing(bEditing) {
    editor.document.head.innerHTML = wiz_head;
    //if (bEditing) {
    //    editor.document.head.innerHTML = wiz_head + m_header; // restore original header
    //} else {
    //    editor.document.head.innerHTML = wiz_head;
    //}

    editor.document.body.innerHTML = wiz_html;
    editor.fireEvent('aftersetcontent');
    editor.fireEvent('contentchange');

    bEditing ? editor.setEnabled() : editor.setDisabled();
}

function viewNote(strGUID, bEditing, strHtml, strHead)
{
    try {
        m_currentGUID = strGUID;
        wiz_html = strHtml;
        wiz_head = strHead;

        if (m_inited) {
            setEditorHtml(wiz_html, bEditing);
        WizEditor.initTodoListEnvironment();
        } else {
            editor.ready(function() {
                m_header = editor.document.head.innerHTML; // save original header
                setEditorHtml(wiz_html, bEditing);
        WizEditor.initTodoListEnvironment();
                m_inited = true;
            });
        }
        
        return true;
    } catch (err) {
        alert(err);
        return false;
    }
}

function viewCurrentNote()
{
    return viewNote(WizEditor.currentNoteGUID(), WizEditor.currentIsEditing(),
                    WizEditor.currentNoteHtml(), WizEditor.currentNoteHead());
}

function updateCurrentNoteHtml()
{
    if (m_currentGUID == WizEditor.currentNoteGUID())
    {
        wiz_html = WizEditor.currentNoteHtml();
        //wiz_head = WizEditor.currentNoteHead();
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



function initDefaultCss(document) {
    var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
    var WIZ_STYLE = 'wiz_style';
    var WIZ_LINK_VERSION = 'wiz_link_version';
    var WIZ_TODO_STYLE_VERSION = "01.00.02";

    var style = document.getElementById(WIZ_TODO_STYLE_ID);
    if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
        return;
    //
    if (style && style.parentElement) { 
        style.parentElement.removeChild(style);
    }
    //
    var strStyle = '.wiz-todo, .wiz-todo-img {cursor: default; padding: 0 10px 0 2px; vertical-align: -10%;-webkit-user-select: none;} .wiz-todo-label { display: inline-block; min-height: 2.5em; line-height: 1;} .wiz-todo-label-checked { text-decoration: line-through; color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;} .wiz-todo-completed-info {padding-left: 44px;} .wiz-todo-avatar { vertical-align: -30%; padding-right:10px;} .wiz-todo-account, .wiz-todo-dt { color: #666; }';
    //
    var objStyle = document.createElement('style');
    objStyle.type = 'text/css';
    objStyle.textContent = strStyle;
    objStyle.id = WIZ_TODO_STYLE_ID;
    // objStyle.setAttribute(WIZ_STYLE, 'unsave');
    objStyle.setAttribute(WIZ_LINK_VERSION, WIZ_TODO_STYLE_VERSION);
    //
    document.head.appendChild(objStyle);
}

/// Wiz todo func

function WizTodoQtHelper() {

    this.wizDoc = editor.document;
    this.getUserAlias = getUserAlias;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.getLocalDateTime = getLocalDateTime;
    this.setDocumentModified = setDocumentModified;
    this.getCheckedImageFileName = getCheckedImageFileName;
    this.getUnCheckedImageFileName = getUnCheckedImageFileName;
    this.initCss = initCss;
    this.canEdit = canEdit;
    this.setDocumentType = setDocumentType;

    function getUserAlias() {
        return objApp.getUserAlias();
    }

    function getUserAvatarFileName(size) {
        return objApp.getUserAvatarFilePath(size);
    }

    function isPersonalDocument() {
        return objApp.isPersonalDocument();
    }

    function getLocalDateTime(dt) {
        return objApp.getFormatedDateTime();
    }

    function setDocumentModified() {
        WizEditor.setContentsChanged(true);
    }

    function getCheckedImageFileName() {
        return objApp.getSkinResourcePath() + "checked.png";
    }

    function getUnCheckedImageFileName() {
        return objApp.getSkinResourcePath() + "unchecked.png";
    }

    function canEdit() {
        return editor.body.contentEditable == "true";
    }

    function initCss(document) {
        var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
        var WIZ_STYLE = 'wiz_style';
        var WIZ_LINK_VERSION = 'wiz_link_version';
        var WIZ_TODO_STYLE_VERSION = "01.00.02";

        var style = document.getElementById(WIZ_TODO_STYLE_ID);
        if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
            return;
        //
        if (style && style.parentElement) { 
            style.parentElement.removeChild(style);
        }
        //
        var strStyle = '.wiz-todo, .wiz-todo-img {cursor: default; padding: 0 10px 0 2px; vertical-align: -10%;-webkit-user-select: none;} .wiz-todo-label { display: inline-block; min-height: 2.5em; line-height: 1;} .wiz-todo-label-checked { text-decoration: line-through; color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;} .wiz-todo-completed-info {padding-left: 44px;} .wiz-todo-avatar { vertical-align: -30%; padding-right:10px;} .wiz-todo-account, .wiz-todo-dt { color: #666; }';
        //
        var objStyle = document.createElement('style');
        objStyle.type = 'text/css';
        objStyle.textContent = strStyle;
        objStyle.id = WIZ_TODO_STYLE_ID;
        // objStyle.setAttribute(WIZ_STYLE, 'unsave');
        objStyle.setAttribute(WIZ_LINK_VERSION, WIZ_TODO_STYLE_VERSION);
        //
        document.body.appendChild(objStyle);
    }  

    function setDocumentType(type) {
        this.wizDoc.Type = type;
    } 
}



///  Wiz todo read check func

function WizDoc(wizDoc) {

    this.doc = wizDoc;

    this.getHtml = getHtml;
    this.setHtml = setHtml;
    this.canEdit = canEdit;
    this.getTitle = getTitle;

    function getHtml() {
        if (!this.doc)
            return null;
        //
        var html = this.doc.GetHtml();
        //
        return html;
    }

    function setHtml(html, resources) {
        if (!this.doc)
            return;
        //
        this.doc.SetHtml2(html, resources);
    }

    function canEdit() {
        return this.doc.CanEdit;
    }

    function getTitle() {
        return this.doc.Title;
    }
}


function WizTodoReadCheckedQt () {

    this.getDocHtml = getDocHtml;
    this.setDocHtml = setDocHtml;
    this.canEdit = canEdit;
    this.getCheckedImageFileName = getCheckedImageFileName;
    this.getUnCheckedImageFileName = getUnCheckedImageFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.getUserAlias = getUserAlias;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.getLocalDateTime = getLocalDateTime;
    this.getAvatarName = getAvatarName;

    function getDocHtml() {
        return objApp.getCurrentNoteHtml();
    }

    function setDocHtml(html, resources) {
        objApp.saveHtmlToCurrentNote(html, resources);
    }

    function canEdit() {
        var htmlEditable = editor.body.contentEditable == "false";
        var userPermission = objApp.hasEditPermissionOnCurrentNote();
        return htmlEditable && userPermission;
    }

    function getCheckedImageFileName() {
        return objApp.getSkinResourcePath() + "checked.png";
    }

    function getUnCheckedImageFileName() {
        return objApp.getSkinResourcePath() + "unchecked.png";
    }

    function isPersonalDocument() {
        return objApp.isPersonalDocument();
    }

    function getLocalDateTime(dt) {
        return objApp.getFormatedDateTime();
    }

    function getUserAlias() {
        return objApp.getUserAlias();
    }

    function getUserAvatarFileName(size) {
        return objApp.getUserAvatarFilePath(size);
    }

    function getAvatarName(avatarFileName) {
        if (!avatarFileName)
            return "";
        //
        var pos = avatarFileName.lastIndexOf('\\');
        if (-1 == pos) {
            pos = avatarFileName.lastIndexOf('/');
        }
        //
        if (-1 != pos) {
            return avatarFileName.substr(pos + 1);
        }
        else return "";
    }
}
