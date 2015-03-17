var
    editor = {},
    m_inited = false,
    m_currentGUID = "",
    m_defaultCss = "";
    m_defaultCssVersion = 1,
    WizEditor = {};
    m_header = "",
    wiz_html = "",
    wiz_head = "";

// setup ueditor
function initialUEditor(wizEditor) {
    WizEditor = wizEditor
    wizEditor.viewNoteRequest.connect(function (strGUID, bEditing, strHtml, strHead) {
        var result = viewNote(strGUID, bEditing, strHtml, strHead);
        WizEditor.on_viewDocumentFinished(result);
    });
    WizEditor.resetDefaultCss.connect(function (strCssPath) {
        m_defaultCss = strCssPath;
    });

    console.log("try to init editor by js");
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

            console.log("ueditor get ready");
            WizEditor.on_ueditorInitFinished();

            var ueditor = document.getElementById('ueditor_0');
            $(ueditor.contentDocument.body).delegate('a', 'click', function(e) {
                var a = $(e.currentTarget),
                href = a.attr('href') || '';
                console.log("eidtor link clicked : " + href);
                if (href && href.indexOf('wiz:') === 0) {
                    //笔记内链
                
                } else if (href && (href.indexOf('mailto:') === 0 || href.indexOf('ftp:') === 0 || href.indexOf('http:') === 0 || href.indexOf('https:') === 0)) {
                    //弹出浏览器窗口显示....

                }

            e.stopPropagation();
            e.preventDefault();
            });

        });

        editor.addListener('aftersetcontent', function() {
            console.log("set content");
            updateCss();
            WizEditor.onNoteLoadFinished();
        });

        editor.addListener('selectionchange', function () {
            WizEditor.onEditorSelectionChanged();
        });

        // 因为QWebEngineView无法响应foucsInEvent,focusOutEvent或keyPressEvent，所以使用编辑器来触发
        // editor.addListener('focus', function () {
        //     WizEditor.focusInEditor();
        // });
        // editor.addListener('blur', function () {
        //     WizEditor.focusOutEditor();
        // });
        // editor.addListener('keydown', function() {
        //     WizEditor.setContentsChanged(true);
        // });

    } catch (err) {
        alert(err);
    }
}

function speakHelloWorld()
{
    console.log("speak Hello World");
}

function functionTest()
{
    editor.execCommand('paste');
    console.log("called from test");
    editor.execCommand('insertHtml', "inserted text", true);
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
        //special process to remove css style added by phone
        editor.window.scrollTo(0, 0);
    });
}

function setEditing(bEditing) {

    console.log("set document editing : " + bEditing + " head : " + wiz_head);

    editor.document.head.innerHTML = wiz_head;
    //if (bEditing) {
    //    editor.document.head.innerHTML = wiz_head + m_header; // restore original header
    //} else {
    //    editor.document.head.innerHTML = wiz_head;
    //}

    editor.document.body.innerHTML = wiz_html;

    //special process to remove css style added by phone

    editor.fireEvent('aftersetcontent');
    editor.fireEvent('contentchange');

    bEditing ? editor.setEnabled() : editor.setDisabled();
}

function viewNote(strGUID, bEditing, strHtml, strHead)
{
    console.log("view note : " + strGUID + "\n" + strHead)
    try {
        m_currentGUID = strGUID;
        wiz_html = strHtml;
        wiz_head = strHead;

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

function viewCurrentNote()
{
    console.log("view currentnote called");
    return viewNote(WizEditor.currentNoteGUID(), WizEditor.currentIsEditing(),
                    WizEditor.currentNoteHtml(), WizEditor.currentNoteHead());
}

function updateCurrentNoteHtml(strGUID, strHtml, strHead)
{
    console.log("update current note html , m_currentGUID : " + m_currentGUID + " \n editor note guid : " + strGUID);
    if (m_currentGUID == strGUID)
    {
        wiz_html = strHtml;
        wiz_head = strHead;
    }
    console.log("update current note html finished, html body ; " + wiz_html);
}

function updateCss()
{
    console.log("update csss file in wiz_extend : " + editor.document.head.innerHTML);
    var css= editor.document.getElementsByTagName('link');
    console.log("update Css. get css file : " + css)
    for (var i = 0; i < css.length; i++) {
        if (css[i].rel != 'stylesheet') return;
        if (css[i].type != 'text/css') return;
        if (css[i].href.match(m_defaultCss)) {
            css[i].href = m_defaultCss + "?v=" + m_defaultCssVersion;
            console.log("after change file location : " + css[i].href);
            m_defaultCssVersion++;
        }
    }
}

// helper functions
function WizGetLinkUrl() {
    console.log('WizGetLinkUrl called');
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

    console.log("before WizGetLinkUrl  return : " + url);
    return url ? url: '';
}

function WizGetImgElementByPoint(posX, posY) {
    var element = editor.document.elementFromPoint(posX, posY);
    if (element && element.tagName == 'IMG') {
        return element.src;
    } 

    return '';
}

//special process to remove css style added by phone.   *** should remove before 2014-10-08
function WizSpecialProcessForPhoneCss() {
    
    var cssElem = editor.document.getElementById("wiz_phone_default_css");
    if (cssElem)
    {
        if (editor.document.head.contains(cssElem))
        {
            editor.document.head.removeChild(cssElem);
        }
        else if (editor.document.body.contains(cssElem))
        {
            editor.document.body.removeChild(cssElem);
        }
    }
}


function WizReplaceText(findtxt, replacetxt, caseSensitive) {
    if (!findtxt) {
        return false;
    }
    if (findtxt == replacetxt || (!caseSensitive && findtxt.toLowerCase() == replacetxt.toLowerCase())) {
        return false;
    }
    obj = {
        searchStr:findtxt,
        dir:1,
        casesensitive:caseSensitive,
        replaceStr:replacetxt
    };
    return frCommond(obj);
}

//全部替换
function WizRepalceAll(findtxt, replacetxt, caseSensitive) {
    if (!findtxt) {
        return false;
    }
    if (findtxt == replacetxt || (!caseSensitive && findtxt.toLowerCase() == replacetxt.toLowerCase())) {
        return false;
    }
    obj = {
        searchStr:findtxt,
        casesensitive:caseSensitive,
        replaceStr:replacetxt,
        all:true
    };
    var num = frCommond(obj);
    return num;
}

//执行
var frCommond = function (obj) {
    return editor.execCommand("searchreplace", obj);
};

//插入code的代码时，li的属性会丢失。需要在head中增加li的属性
function WizAddCssForCodeLi() {
    var WIZ_INLINE_CODE_ID = 'wiz_inline_code_id';
    var WIZ_LINK_VERSION = 'wiz_link_version';
    var WIZ_TODO_STYLE_VERSION = "01.00.00";

    var style = document.getElementById(WIZ_INLINE_CODE_ID);
    if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
        return;
    //
    if (style && style.parentElement) { 
        style.parentElement.removeChild(style);
    }
    //
    var strStyle = 'ol.linenums li {color: #BEBEC5;line-height: 18px;padding-left: 12px; }';
    //
    var objStyle = document.createElement('style');
    objStyle.type = 'text/css';
    objStyle.textContent = strStyle;
    objStyle.id = WIZ_INLINE_CODE_ID;
    objStyle.setAttribute(WIZ_LINK_VERSION, WIZ_TODO_STYLE_VERSION);
    //
    editor.document.head.appendChild(objStyle);
}

//在原有代码块的基础上插入新代码时，删除之前的标志
function WizInsertCodeHtml(html) {
    // var parentElem = editor.document.getSelection();
    // if (parentElem && parentElem.type == "pre")
    // {
    //     document.removeChild(parentElem);
    // }


    editor.execCommand('insertHtml', html, true);
}

function WizAddCssForCode(cssFile) {
    console.log("WizAddCssForCode called , css file " + cssFile);
    var doc = editor.document;
    if (!doc)
        return;
    //
    var oldLink = doc.getElementById('wiz_code_highlight_link');
    if (oldLink) {
        console.log("old css link find, try to remove");
        oldLink.parentNode.removeChild(oldLink);
    }
    //
    var link = doc.createElement('link');
    if (!link)
        return;
    try {
        link.type = 'text/css';
        link.rel = 'stylesheet';
        link.id = 'wiz_code_highlight_link';
        link.href = cssFile;
        //
        if (!doc.head)
        {
            doc.insertBefore(doc.createElement('head'), doc.body);
        }
        doc.head.appendChild(link);
    }
    catch(e) {

    }
}


function WizClearEditorHeight() {
    editor.document.body.style.height='';
}


function WizMoveEnd(){  
    obj = editor.document.body;
    obj.focus();  
    var len = obj.value.length;  
    if (document.selection) {  
        var sel = obj.createTextRange();  
        sel.moveStart('character',len);  
        sel.collapse();  
        sel.select();  
    } else if (typeof obj.selectionStart == 'number' && typeof obj.selectionEnd == 'number') {  
        obj.selectionStart = obj.selectionEnd = len;  
    }  
  
}   