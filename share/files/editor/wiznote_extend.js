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

    //NOTE: 不能监听contentchange事件，否则仅仅进入编辑状态就会修改笔记为已修改
    // editor.addListener('contentchange', function() {
    //     WizEditor.setContentsChanged(true);
    // });

    editor.addListener('wizcontentchange', function() {
        WizEditor.setContentsChanged(true);
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

function updateEditorHtml(bEditing)
{
    editor.reset();
    editor.document.head.innerHTML = WizEditor.currentNoteHead();
    editor.document.body.innerHTML = WizEditor.currentNoteHtml();
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

function WizGetMailSender () {
	var items = editor.document.getElementsByTagName('input');
	//
	for (var i = items.length - 1; i >= 0; i--) {
		if (items[i].type == "hidden" && RegExp("\\bwiz_mail_reply_to\\b").test(items[i].value)){
			var str = items[i].value;
			return str.replace("wiz_mail_reply_to/", "");
		}
	}
	//
	for (var i = items.length - 1; i >= 0; i--) {
		if (items[i].type == "hidden" && RegExp("\\bwiz_mail_from\\b").test(items[i].value)){
			var str = items[i].value;
			return str.replace("wiz_mail_from/", "");
		}
	}
}