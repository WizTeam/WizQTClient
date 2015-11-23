//webview的锚点跳转存在问题，通过js来控制跳转
var WizHref = function() {
    var _doc;
    var onClick = function(e) {
        var a = e.target,
            href = a.getAttribute('href') || '';
        if (href && href.indexOf('#') === 0) {
            var dom = _doc.querySelector(href);
            if (dom) {
                dom.scrollIntoView(true);
                e.stopPropagation();
                e.preventDefault();
            }
        }
    };

    var wizHref = {
        on: function(doc) {
            _doc = doc;
            _doc.body.addEventListener('click', onClick);
        },
        off: function(doc) {
            _doc = doc;
            doc.body.removeEventListener('click', onClick);
        }
    };
    return wizHref;
};



var
    editor = null,
    m_inited = false,
    m_wizReaderInited = false;
    objApp = WizExplorerApp,
    m_currentGUID = "",
    m_defaultCss = WizEditor.getDefaultCssFilePath(),
    m_defaultCssVersion = 1,
    m_header = "",
    wiz_html = "",
    wiz_head = "";
    wiz_href = new WizHref();


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

function initWizReader() {                  
    var f = window.document.getElementById('ueditor_0');
    if (!f.contentWindow.WizReader) { 
        console.log("wizReader is null");
        return;
    }
    var dependencyFilePath = WizEditor.getWizReaderDependencyFilePath();
    var cssFile = WizEditor.getMarkdownCssFilePath();

    m_wizReaderInited = f.contentWindow.WizReader.init({
    document: editor.document,
    lang: 'zh-cn',
    clientType: 'mac',
    userInfo: {},
    // usersData: '',
    noAmend: false,  //wizReader 专用参数，用于关闭 修订功能,
    dependencyCss: {
        github2: cssFile,  //markdown 使用
        wizToc: dependencyFilePath + 'wizToc.css'     //toc 样式
    },
    dependencyJs: {
        jquery: dependencyFilePath + 'jquery-1.11.3.js', //jquery
        prettify: dependencyFilePath + 'prettify.js',       //代码高亮
        raphael: dependencyFilePath + 'raphael.js',     //流程图 & 时序图 依赖
        underscore: dependencyFilePath + 'underscore.js',   //时序图 依赖
        flowchart: dependencyFilePath + 'flowchart.js',         //流程图
        sequence: dependencyFilePath + 'sequence-diagram.js', //时序图
        //mathJax 如果不传则使用 默认地址
        mathJax: 'http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_HTML'
    }
    }) | true;
}

function renderMarkdown() {    
    var reader = WizEditor.getWizReaderFilePath() + "wizReader.js";
    var wizReaderDocument = loadSingleJs(editor.document, reader);
    wizReaderDocument.onload = function() {
    	initWizReader();
    	var f = window.document.getElementById('ueditor_0');
		f.contentWindow.WizReader.markdown();
    };
}

function loadSingleJs(doc, path) {
    var jsId = 'wiz_' + path;
    if (doc.getElementById(jsId)) {
    	console.log("js file already exisits");
        return true;
    }
    var s = doc.createElement('script');
    s.type = 'text/javascript';
    s.src = path.replace(/\\/g, '/');
    s.id = jsId;
    doc.getElementsByTagName('head')[0].insertBefore(s, null);
    return s;
}

function setEditorHtml(html, bEditing)
{
    setWizHrefEnable(bEditing == false);

    editor.reset();
    editor.document.head.innerHTML = wiz_head;    
    editor.document.body.innerHTML = html;
    editor.fireEvent('aftersetcontent');
    editor.fireEvent('contentchange');

    bEditing ? editor.setEnabled() : editor.setDisabled();

    window.UE.utils.domReady(function() {
        //special process to remove css style added by phone
        WizSpecialProcessForPhoneCss(); 
        updateCustomCss();  
        WizEditor.initCheckListEnvironment();
        editor.window.scrollTo(0, 0);
    });
}

function setEditing(bEditing) {    
    setWizHrefEnable(bEditing == false);

    editor.document.head.innerHTML = wiz_head;
    editor.document.body.innerHTML = wiz_html;

    //special process to remove css style added by phone
    WizSpecialProcessForPhoneCss();
    updateCustomCss();

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
        updateCustomCss();
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

//special process to remove css style added by phone.   *** should remove before 2016-1-1
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

function updateCustomCss() {
	var WIZ_CUSTOM_STYLE_ID = 'wiz_custom_css';
	var WIZ_STYLE = 'wiz_style';

	var style = editor.document.getElementById(WIZ_CUSTOM_STYLE_ID);
	if (style)
		return;
	//
	var strStyle = 'html,body{font-size:15px}body{font-family:Helvetica,"Hiragino Sans GB","Microsoft Yahei",SimSun,SimHei,arial,sans-serif;line-height:1.6;padding:0;margin:20px 36px;margin:1.33rem 2.4rem}h1,h2,h3,h4,h5,h6{margin:20px 0 10px;margin:1.33rem 0 .667rem;padding:0;font-weight:bold}h1{font-size:21px;font-size:1.4rem}h2{font-size:20px;font-size:1.33rem}h3{font-size:18px;font-size:1.2rem}h4{font-size:17px;font-size:1.13rem}h5{font-size:15px;font-size:1rem}h6{font-size:15px;font-size:1rem;color:#777;margin:1rem 0}div,p,blockquote,ul,ol,dl,table,pre{margin:10px 0;margin:.667rem 0}ul,ol{padding-left:32px;padding-left:2.13rem}blockquote{border-left:4px solid #ddd;padding:0 12px;padding:0 .8rem;color:#aaa}blockquote>:first-child{margin-top:0}blockquote>:last-child{margin-bottom:0}img{border:0;max-width:100%;height:auto !important}table{border-collapse:collapse;border:1px solid #bbb}td{border-collapse:collapse;border:1px solid #bbb}@media screen and (max-width:660px){body{margin:20px 18px;margin:1.33rem 1.2rem}}@media only screen and (-webkit-max-device-width:1024px),only screen and (-o-max-device-width:1024px),only screen and (max-device-width:1024px),only screen and (-webkit-min-device-pixel-ratio:3),only screen and (-o-min-device-pixel-ratio:3),only screen and (min-device-pixel-ratio:3){html,body{font-size:17px}body{line-height:1.7;margin:12px 15px;margin:.75rem .9375rem;color:#353c47;text-align:justify;text-justify:inter-word}h1{font-size:34px;font-size:2.125rem}h2{font-size:30px;font-size:1.875rem}h3{font-size:26px;font-size:1.625rem}h4{font-size:22px;font-size:1.375rem}h5{font-size:18px;font-size:1.125rem}h6{color:inherit}div,p,blockquote,ul,ol,dl,table,pre{margin:0}ul,ol{padding-left:40px;padding-left:2.5rem}blockquote{border-left:4px solid #c8d4e8;padding:0 15px;padding:0 .9375rem;color:#b3c2dd}}';	//
	var objStyle = editor.document.createElement('style');
	objStyle.type = 'text/css';
	objStyle.textContent = strStyle;
	objStyle.id = WIZ_CUSTOM_STYLE_ID;
	//
	if (editor.document.head) {
		editor.document.head.appendChild(objStyle);
	}	
}

function setWizHrefEnable (enable) {
    var iframeDoc = window.document.getElementById('ueditor_0').contentDocument;
    if (enable) {       
       wiz_href.on(iframeDoc);
    } else {
       wiz_href.off(iframeDoc);
    }
}
