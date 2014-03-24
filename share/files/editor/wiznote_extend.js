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
    var windowEdit = null;
    var external = null;

    function getElementDisply(ele) {
        var displayStyle = "";
        if (ele) { 
            try { 
                if (windowEdit.getComputedStyle) {
                    displayStyle = windowEdit.getComputedStyle(ele, null).getPropertyValue('display');
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

    function pasteHtmlAtCaret(html, selectPastedContent) {
        var sel, range, retNode;
        if (window.getSelection) {
            // IE9 and non-IE
            sel = window.getSelection();
            if (sel.getRangeAt && sel.rangeCount) {
                range = sel.getRangeAt(0);
                range.deleteContents();

                // Range.createContextualFragment() would be useful here but is
                // only relatively recently standardized and is not supported in
                // some browsers (IE9, for one)
                var el = document.createElement("div");
                el.innerHTML = html;
                var frag = document.createDocumentFragment(), node, lastNode;
                while ( (node = el.firstChild) ) {
                    lastNode = frag.appendChild(node);
                }
                var firstNode = frag.firstChild;
                retNode = firstNode;
                range.insertNode(frag);
                
                // Preserve the selection
                if (lastNode) {
                    range = range.cloneRange();
                    range.setStartAfter(lastNode);
                    if (selectPastedContent) {
                        range.setStartBefore(firstNode);
                    } else {
                        range.collapse(true);
                    }
                    sel.removeAllRanges();
                    sel.addRange(range);
                }
            }
        } else if ( (sel = document.selection) && sel.type != "Control") {
            // IE < 9
            var originalRange = sel.createRange();
            originalRange.collapse(true);
            sel.createRange().pasteHTML(html);
            if (selectPastedContent) {
                range = sel.createRange();
                range.setEndPoint("StartToStart", originalRange);
                range.select();
            }
        }
        //
        return retNode;
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

    function removeInvalidText(ele) {
        if (!ele)
            return;
        if (!ele.hasChildNodes())
            return;
        //
        for (var i = ele.childNodes.length - 1; i >= 0; i --) {
            var child = ele.childNodes[i];
            //
            if (child.nodeType == 3 && child.nodeValue == "") {
                ele.removeChild(child);
            }

        }
    }

    function isTodoAtFirst(ele) {
        if (!ele)
            return false;
        //
        // if (ele.tagName.toString().toLowerCase() == 'body')
        //  return true;
        removeInvalidText(ele);
        //
        var childnodes = ele.childNodes;
        if (!childnodes || childnodes.length < 1 || !childnodes[0].getAttribute)
            return false;
        if (childnodes.length < 1)
            return false;
        //
        var firstChild = childnodes[0];
        if (-1 != getClassValue(firstChild).indexOf('wiz-todo-label')) {
            var todoLabel = firstChild;
            removeInvalidText(todoLabel);
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

    function isTodoImage(ele) {
        if (!ele)
            return false;
        if (!ele.getAttribute || -1 == getClassValue(ele).indexOf(WIZ_HTML_CLASS_WIZ_TODO))
            return false;
        //
        return true;
    }

    function isSelectionExactlyAfterTodoImage() {
        var rgn = document.getSelection().getRangeAt(0);
        //
        var start = rgn.startContainer;
        if (!start)
            return false;
        if (isWizTodoBlockElement(start)) {
            if (!start.hasChildNodes())
                return false;
            //
            for (var i in start.childNodes) {
                if (i + 1 > rgn.startOffset)
                    return false;
                var child = start.childNodes[i];
                //
                if (child && child.tagName 
                    && -1 != getClassValue(child).indexOf('wiz-todo-label')
                    && i + 1 == rgn.startOffset) {
                    
                    removeInvalidText(child);
                    //
                    var lastChild = child.lastChild;
                    if (isTodoImage(lastChild))
                        return true;
                    else 
                        return false;
                }
            }
            //
            return false;
        }
        else {
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
            // p.insertBefore(document.createElement('br'), p.firstChild);
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
            range.setStartAfter(div.lastChild);
            range.setEndAfter(div.lastChild);
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

    function setCaret(label) {
        if (!label)
            return;
        var sel = document.getSelection();
        //
        var range = document.createRange();
        range.setStartAfter(label.lastChild);
        range.collapse(true);
        //
        sel.removeAllRanges();
        sel.addRange(range);
    }

    function removeNestLabel(label) {

        if (!label)
            return;
        //
        var p = label.parentElement;
        if (!isLabel(p) || p.childElementCount < 1 || !isLabel(p.children[0])) 
            return;
        //
        if (!p.hasChildNodes())
            return;
        var meetLabel = false;
        while (p.childNodes.length > 0) {
            var child = p.childNodes[0];
            //
            if (child == label) {
                meetLabel = true;
            }
            //
            if (child != label && meetLabel) {
                label.appendChild(child);
            }
            else if (child == label) {
                p.parentElement.insertBefore(child, p.nextElementSibling);
            }
            else {
                p.parentElement.insertBefore(child, p);
            }
        }
        //
        p.parentElement.removeChild(p);
        // remove br after label
        if (label.nextElementSibling && label.nextElementSibling.tagName.toLowerCase() == 'br') {
            label.parentElement.removeChild(label.nextElementSibling);
        }
        if (label.lastElementChild && label.lastElementChild.tagName.toLowerCase() == 'br') {
            label.removeChild(label.lastElementChild);
        }
        //
        setCaret(label);
    }

    function mergeNextSibilingTextChild(ele, mergePrev) {
        if (!ele)
            return;
        //
        while (ele.nextSibling && ele.nextSibling.nodeType == 3) {
            ele.appendChild(ele.nextSibling);
        }
        //
        if (mergePrev) {
            var imgs = ele.getElementsByClassName(WIZ_HTML_CLASS_WIZ_TODO);
            if (!imgs || imgs.length < 1)
                return;
            //
            var img = imgs[0];
            var target = img.nextSibling;
            //
            while (ele.previousSibling && ele.previousSibling.nodeType == 3) {
                if (target) {
                    ele.insertBefore(ele.previousSibling, target);
                }
                else {
                    ele.appendChild(ele.previousSibling);
                }
            }
        }
        //
        setCaret(ele);
    }

    function divideFromParentLabel(label) {
        if (!label)
            return;
        //
        var sel = document.getSelection();
        //
        var p = label.parentElement;
        if (!p || !isLabel(p))
            return;
        //
        p.parentElement.insertBefore(label, p.nextElementSibling);
        //
        setCaret(label);
    }

    function insertOneTodo(fromKeyUp) {

        // deleteEmptyLabel();
        //
        var strHTML = "<label class='wiz-todo-label-unchecked'>" + 
                        "<img class='wiz-todo wiz-img-cannot-drag' src='%1' state='unchecked'>" + 
                      "</label>";
        //
        strHTML = strHTML.replace("%1", g_todoImagePath + "unchecked.png");
        //
        var label = pasteHtmlAtCaret(strHTML, false);
        removeNestLabel(label);
        //
        if (isSelectionDirectlyInBody()) {
            wrapBodyInlineChildren();
        }
        //
        mergeNextSibilingTextChild(label, !!fromKeyUp);
        //
        divideFromParentLabel(label);
        //
        setCaret(label);
        //
        reBindAction();

        alert("In insertOneTodo");
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

    function getLabelBeforeCaret() {
        var sel = document.getSelection();
        //
        if (!sel || sel.type.toLowerCase() == 'none')
            return null;
        if (sel.type.toLowerCase() != 'caret')
            return null;
        //
        var rgn = sel.getRangeAt(0);
        //
        var start = rgn.startContainer;
        if (isWizTodoBlockElement(start)) {
            if (!start.hasChildNodes())
                return null;
            //
            for (var i = 0; i < start.childNodes.length; i ++) {
                if (i + 1 > rgn.startOffset)
                    return null;
                var child = start.childNodes[i];
                //
                if (child && child.tagName 
                    && -1 != getClassValue(child).indexOf('wiz-todo-label')
                    && i + 1 == rgn.startOffset) {
                    return child;
                }
            }
            //
            return null;
        }
        else {
            return getParentTodoLabelElement(start.nodeType == 1 ? start : start.parentElement);
        }

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
        if (!isTodoAtFirst(prevSibling))
            return false;
        //
        return true;
    }

    function onKeyDown(e) {
        if (13 != e.keyCode) // Return key
            return;
        //
        var sel = document.getSelection();
        if (sel.type.toLowerCase() == 'none')
            return;
        //
        var rgn = sel.getRangeAt(0);
        //
        if (0 == rgn.startOffset && 0 == rgn.endOffset) {
            g_canInsertWizTodo = false;
            return;
        }
        //
        var label = null;
        if (sel.type.toLowerCase() == 'caret') {
            label = getLabelBeforeCaret();
        }
        else {
            var start = rgn.startContainer;
            start = start.nodeType == 1 ? start : start.parentElement;
            //
            label = getParentTodoLabelElement(start);
        }
        //
        var parentEle = getBlockParentElement(label);
        //
        if (isWizTodoBlockElement(parentEle) && isTodoAtFirst(parentEle)) {

            if (isSelectionExactlyAfterTodoImage() && hasPrevSiblingTodo(parentEle)) {
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

            insertOneTodo(true);
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

    function initCss() {
        var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
        var WIZ_STYLE = 'wiz_style';
        var WIZ_LINK_VERSION = 'wiz_link_version';
        var WIZ_TODO_STYLE_VERSION = "01.00.00";
        var WIZ_TODO_JS_VERSION = "01.00.10";

        var style = document.getElementById(WIZ_TODO_STYLE_ID);
        if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
            return;
        //
        if (style && style.parentElement) { 
            style.parentElement.removeChild(style);
        }
        //
        var strStyle = '.wiz-todo {cursor: default; padding: 0 8px 0 2px; } .wiz-todo-label-checked { text-decoration: line-through; color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;}';
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

    function init(doc, wnd) { 
        if (!doc || !wnd) {
            if (console) {
                console.log('WizTodo, init, doc: ' + doc.toString() + 'wnd: ' + wnd.toString() + 'ext: ' + toString());
            }
            return;
        }
        //
        document = doc;
        windowEdit = wnd;

        registerEvent();
        //
        reBindAction();
        //
        initTodoImagePath();
        //
        initCss();
    }

    return {
        init: init,
        insertOneTodo: insertOneTodo
    }
})();
