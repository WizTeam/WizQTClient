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
        //special process to remove css style added by phone
        WizSpecialProcessForPhoneCss();
        WizEditor.initCheckListEnvironment();
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

    //special process to remove css style added by phone
    WizSpecialProcessForPhoneCss();

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
    return viewNote(WizEditor.currentNoteGUID(), WizEditor.currentIsEditing(),
                    WizEditor.currentNoteHtml(), WizEditor.currentNoteHead());
}

function updateCurrentNoteHtml()
{
    if (m_currentGUID == WizEditor.currentNoteGUID())
    {
        wiz_html = WizEditor.currentNoteHtml();
        wiz_head = WizEditor.currentNoteHead();
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