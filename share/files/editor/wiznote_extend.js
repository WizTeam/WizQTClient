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


//////////      WizTodoFunc


function setDocumentModified() {
	WizEditor.setContentsChanged(true);
}

var WizTodo = (function () {

    var WIZ_HTML_CLASS_WIZ_TODO = 'wiz-todo';
    var WIZ_HTML_CLASS_CANNOT_DRAG = 'wiz-img-cannot-drag';
    var document = null;
    var window = null;

    function getElementDisply(ele) {
        var displayStyle = "";
        if (ele) { 
            try { 
                if (window.getComputedStyle) {
                    displayStyle = window.getComputedStyle(ele, null).getPropertyValue('display');
                } else {
                    displayStyle = ele.currentStyle.display;
                }
            }
            catch (e) {
                displayStyle = "";
            }
        }
        //
        return displayStyle;
    }

    function getClassValue(ele) {
        if (!ele)
            return "";
        //
        var classValue = !!ele.getAttribute && ele.getAttribute('class');
        if (!classValue)
            return "";
        //
        return classValue.toString();
    }

    function isWizTodoBlockElement(ele) {
        if (!ele)
            return false;
        //
        var displayValue = getElementDisply(ele);
        if (!displayValue)
            return false;
        //
        var value = displayValue.toString().toLowerCase().trim();
        //
        if (!value)
            return false;
        //
        if (value == 'block' || value == 'list-item' || value == 'table-cell') 
            return true;
        //
        return false;
    }

    function getBlockParentElement(ele) {
        if (!ele)
            return null;
        //
        var p = ele;
        while (p) {
            if (p.tagName.toString().toLowerCase() == 'body')
                return null;
            //
            if (isWizTodoBlockElement(p))
                return p;
            //
            p = p.parentElement;
        }
        //
        return null;
    }

    function isWizTodoFirstNode(ele) {
        if (!ele)
            return false;
        //
        // if (ele.tagName.toString().toLowerCase() == 'body')
        //  return true;
        var childnodes = ele.childNodes;
        if (!childnodes || childnodes.length < 1 || !childnodes[0].getAttribute)
            return false;
        if (childnodes.length < 1)
            return false;
        //
        var firstChild = childnodes[0];
        if (-1 != getClassValue(firstChild).indexOf('wiz-todo-label')) {
            var todoLabel = firstChild;
            //
            var childnodes = todoLabel.childNodes;
            //
            if (!childnodes || childnodes.length < 1 || !childnodes[0].getAttribute)
                return false;
            if (childnodes.length < 1)
                return false;
            //
            if (-1 != getClassValue(childnodes[0]).indexOf(WIZ_HTML_CLASS_WIZ_TODO))
                return true;
        }
        else if (-1 != getClassValue(firstChild).indexOf(WIZ_HTML_CLASS_WIZ_TODO)) {
            return true;
        }
        //
        return false;
    }

    var g_canInsertWizTodo = false;
    function canInsertWizTodo() {
        return !!g_canInsertWizTodo;
    }

    var g_todoImagePath = "";
    function initTodoImagePath() {
        //
        if (objApp) {
            g_todoImagePath = WizEditor.getDefaultImageFilePath();
        }
    }

    function reBindAction() {
        var todos = document.getElementsByClassName('wiz-todo');
        //
        function onTodoClick(todoEle) {
            //
            var label = getParentTodoLabelElement(todoEle);
            var classValue = getClassValue(label);
            //
            var isChecked = todoEle.getAttribute('state') == 'checked';
            var imgSrc = isChecked ? g_todoImagePath + 'unchecked.png' : g_todoImagePath + 'checked.png';
            var state = isChecked ? 'unchecked' : 'checked';
            //
            if (isChecked) {
                if (-1 != classValue.indexOf('wiz-todo-label-checked')) {
                    classValue = classValue.replace('wiz-todo-label-checked', 'wiz-todo-label-unchecked');
                }
                else {
                    classValue += ' wiz-todo-label-unchecked';
                }
            } 
            else {
                //
                if (-1 != classValue.indexOf('wiz-todo-label-unchecked')) {
                    classValue = classValue.replace('wiz-todo-label-unchecked', 'wiz-todo-label-checked');
                }
                else {
                    classValue += ' wiz-todo-label-checked';
                }           
            }
	        setDocumentModified();
            //          
            todoEle.src = imgSrc;
            todoEle.setAttribute('state', state);
            label.setAttribute('class', classValue);
        }
        //
        for (var i in todos) {
            todos[i].onclick = function(e) {
                onTodoClick(this);
            }
        }
    }
    
    function isSelectionInEmptyLabel(retLabel) {
        retLabel.ret = null;
        //
        var rgn = document.getSelection().getRangeAt(0);
        //
        var start = rgn.startContainer;
        var label = getParentTodoLabelElement(start);
        if (!label)
            return false;
        //
        if (!label.hasChildNodes()) {
            retLabel.ret = label;
            return true;
        }
        var childnodes = label.childNodes;
        //
        for (var i in childnodes) {
            var node = childnodes[i];
            //
            if (node && !!node.getAttribute && -1 != getClassValue(node).indexOf(WIZ_HTML_CLASS_WIZ_TODO))
                return false;
        }
        //
        retLabel.ret = label;
        return true;
    }

    function isSelectionExactlyAfterTodo() {
        var rgn = document.getSelection().getRangeAt(0);
        //
        var start = rgn.startContainer;
        var label = getParentTodoLabelElement(start);
        if (!label)
            return false;
        //
        if (!label.hasChildNodes()) 
            return false;
        //
        var childnodes = label.childNodes;
        if (childnodes.length != 1)
            return false;
        //
        if (!childnodes[0].getAttribute || -1 == getClassValue(childnodes[0]).indexOf(WIZ_HTML_CLASS_WIZ_TODO))
            return false;
        //
        if (1 != rgn.startOffset || 1 != rgn.endOffset)
            return false;
        //
        return true;
    }

    function deleteEmptyLabel() {

        var label = {};
        if (isSelectionInEmptyLabel(label)) {
            var label = label.ret;
            var p = label.parentElement;
            //
            var sel = document.getSelection();
            //
            if (label.hasChildNodes()) {
                var childnodes = label.childNodes;
                while (childnodes.length > 0) {
                    p.appendChild(childnodes[0]);
                }
                //
                p.removeChild(label);
            }
            else {
                p.removeChild(label);
            }
            //
            p.insertBefore(document.createElement('br'), p.firstChild);
            //
            var range = document.createRange();
            range.setStart(p, 0);
            range.setEnd(p, 1);
            //
            sel.removeAllRanges();
            sel.addRange(range);
        }
    }

    function isSelectionDirectlyInBody() {
        var rgn = document.getSelection().getRangeAt(0);
        //
        var p = rgn.startContainer;
        //
        while (p && !isWizTodoBlockElement(p)) {
            p = p.parentElement;
        }
        //
        if (p && p.tagName.toString().toLowerCase() == 'body')
            return true;
        //
        return false;
    }

    function wrapBodyInlineChildren() {
        if (!document.body)
            return;
        //
        if (!document.body.hasChildNodes())
            return;
        //
        var childnodes = document.body.childNodes;
        //
        var div = document.createElement('div');
        //
        while (childnodes.length > 0) {
            var node = childnodes[0];
            //
            if (!isWizTodoBlockElement(node)) {
                div.appendChild(node);
            } 
            else break;
        }
        //
        if (div.hasChildNodes()) { 
            var sel = document.getSelection();
            //
            document.body.insertBefore(div, document.body.firstChild);
            //
            range = document.createRange();
            range.setStart(div.lastChild, 1);
            range.setEnd(div.lastChild, 1);
            //
            sel.removeAllRanges();
            sel.addRange(range);
        }
    }

    function isLabel(ele) {
        if (!ele)
            return false;
        //
        if (-1 != getClassValue(ele).indexOf('wiz-todo-label'))
            return true;
        //
        return false;
    }

    function removeNestLabel() {
        var rgn = document.getSelection().getRangeAt(0);
        //
        var start = rgn.startContainer;
        //
        var label = getParentTodoLabelElement(start);
        if (!label)
            return;
        //
        var p = label.parentElement;
        if (!isLabel(p)) 
            return;
        //
        if (!p.hasChildNodes())
            return;
        while (p.childNodes.length > 0) {
            p.parentElement.insertBefore(p.childNodes[0], p);
        }
        //
        p.parentElement.removeChild(p);
    }

    function insertOneTodo() {

        // deleteEmptyLabel();
        //
        var strHTML = "<label class='wiz-todo-label-unchecked'>" + 
                        "<img class='wiz-todo wiz-img-cannot-drag' src='%1' state='unchecked'>" + 
                      "</label>";
        //
        strHTML = strHTML.replace("%1", g_todoImagePath + "unchecked.png");
        //
        document.execCommand('insertHTML', false, strHTML);
        removeNestLabel();
        //
        if (isSelectionDirectlyInBody()) {
            wrapBodyInlineChildren();
        }
        //
        reBindAction();
    }

    function getParentTodoLabelElement(start) {
        if (!start)
            return null;
        //
        var p = start;
        while(p && !isWizTodoBlockElement(p)) {

            if (!!p.tagName && p.tagName.toLowerCase() == 'body') 
                break;
            //
            if (!!p.tagName && p.tagName.toLowerCase() == 'label' && -1 != getClassValue(p).indexOf('wiz-todo-label'))
                return p;
            //
            p = p.parentElement;
        }
        //
        return null;
    }

    function hasPrevSiblingTodo(ele) {
        if (!ele)
            return false;
        //
        var prevSibling = ele.previousSibling;
        //
        if (!prevSibling)
            return false;
        if (!prevSibling.tagName || !ele.tagName)
            return false;
        //
        if (prevSibling.tagName.toString() != ele.tagName.toString())
            return false;
        //
        if (!isWizTodoFirstNode(prevSibling))
            return false;
        //
        return true;
    }

    function onKeyDown(e) {
        if (13 != e.keyCode) // Return key
            return;
        //
        var rgn = document.getSelection().getRangeAt(0);
        //
        if (0 == rgn.startOffset && 0 == rgn.endOffset) {
            g_canInsertWizTodo = false;
            return;
        }
        //
        var start = rgn.startContainer;
        start = start.nodeType == 1 ? start : start.parentElement;
        //
        var label = getParentTodoLabelElement(start);
        //
        var parentEle = getBlockParentElement(start);
        //
        if (isWizTodoBlockElement(parentEle) && isWizTodoFirstNode(parentEle)) {

            if (isSelectionExactlyAfterTodo() && hasPrevSiblingTodo(parentEle)) {
                label.parentElement.removeChild(label);
                //
                g_canInsertWizTodo = false;
                return;
            }
            // 
            g_canInsertWizTodo = true;
        }
        else {
            g_canInsertWizTodo = false;
        }
    }

    function onKeyUp(e) {
        if (13 != e.keyCode) // Return key
            return;
        //
        if (canInsertWizTodo()) { 
            insertOneTodo();
        }
        else {
            deleteEmptyLabel();
        }
    }

    function registerEvent() {
        document.removeEventListener('keydown', onKeyDown);
        document.removeEventListener('keyup', onKeyUp);
        //
        document.addEventListener('keydown', onKeyDown);
        document.addEventListener('keyup', onKeyUp);    
    }

    function init(doc, wnd) { 
        document = doc;
        window = wnd;

        registerEvent();
        //
        reBindAction();
        //
        initTodoImagePath();
    }

    function onPluginGetHtml() {}

    return {
        init: init,
        insertOneTodo: insertOneTodo
    }
})();
