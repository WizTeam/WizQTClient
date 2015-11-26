
function initDefaultCss(document, destNode) {
	var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
	var WIZ_STYLE = 'wiz_style';
	var WIZ_LINK_VERSION = 'wiz_link_version';
	var WIZ_TODO_STYLE_VERSION = "01.00.09";

	var style = document.getElementById(WIZ_TODO_STYLE_ID);
	if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
		return;
	//
	if (style && style.parentElement) { 
		style.parentElement.removeChild(style);
	}
	//
	var strStyle = '.wiz-todo, .wiz-todo-img {width: 16px; height: 16px; cursor: default; padding: 0 10px 0 2px; vertical-align: -10%;-webkit-user-select: none;} .wiz-todo-label { display: inline-block; padding-top: 7px; padding-bottom: 6px; line-height: 1.5;} .wiz-todo-label-checked { /*text-decoration: line-through;*/ color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;} .wiz-todo-completed-info {padding-left: 44px; display: inline-block; } .wiz-todo-avatar { width:20px; height: 20px; vertical-align: -20%; margin-right:10px; border-radius: 2px;} .wiz-todo-account, .wiz-todo-dt { color: #666; }';	//
	var objStyle = document.createElement('style');
	objStyle.type = 'text/css';
	objStyle.textContent = strStyle;
	objStyle.id = WIZ_TODO_STYLE_ID;
	// objStyle.setAttribute(WIZ_STYLE, 'unsave');
	objStyle.setAttribute(WIZ_LINK_VERSION, WIZ_TODO_STYLE_VERSION);
	//
	destNode.appendChild(objStyle);
}

function WizTodoWindowsHelper(external) {

	if (external) {
		this.wizEditor = external;
	}

	this.getUserAlias = getUserAlias;
	this.getUserAvatarFileName = getUserAvatarFileName;
	this.isPersonalDocument = isPersonalDocument;
	this.getLocalDateTime = getLocalDateTime;
	this.setDocumentModified = setDocumentModified;
	this.getCheckedImageFileName = getCheckedImageFileName;
	this.getUnCheckedImageFileName = getUnCheckedImageFileName;
	this.setDocumentType = setDocumentType;
	this.initCss = initCss;
	this.canEdit = canEdit;

	function getUserAlias() {
		return this.wizEditor.UserAlias;
	}

	function getUserAvatarFileName(size) {
		return this.wizEditor.GetUserAvatarFileName(size);
	}

	function isPersonalDocument() {

		try {
			return this.wizEditor.WizDocument.IsPersonalDocument();
		}
		catch (e) {
			return false;
		}
	}

	function getLocalDateTime(dt) {
		var nosec = true;
		return this.wizEditor.GetLocalDateString(dt, true) + ' ' + this.wizEditor.GetLocalTimeString(dt, nosec);
	}

	function setDocumentModified() {
		this.wizEditor.SetContentModified(true);
	}

	function getCheckedImageFileName() {
		return this.wizEditor.AppPath + "WizTools\\htmleditor\\checked.png";
	}

	function getUnCheckedImageFileName() {
		return this.wizEditor.AppPath + "WizTools\\htmleditor\\unchecked.png";
	}

	function canEdit() {
		return true;
	}

	function initCss(document) {
		initDefaultCss(document, document.head);
	}

	function setDocumentType(type) {
		/*
		var oldType = this.wizDoc.Type;
		if (oldType) {
			if (-1 == oldType.indexOf(type)) {
				this.wizDoc.Type = oldType + ';' + type;
			}
		}
		else {
			this.wizDoc.Type = type;
		}*/
		this.wizEditor.WizDocument.Type = type;
	}
}

function WizTodoQtHelper() {

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
        return WizEditor.getUserAlias();
    }

    function getUserAvatarFileName(size) {
        return WizEditor.getUserAvatarFilePath(size);
    }

    function isPersonalDocument() {
        return WizEditor.isPersonalDocument();
    }

    function getLocalDateTime(dt) {
        return WizEditor.getFormatedDateTime();
    }

    function setDocumentModified() {
        WizEditor.setContentsChanged(true);
    }

    function getCheckedImageFileName() {
        return WizEditor.getSkinResourcePath() + "checked.png";
    }

    function getUnCheckedImageFileName() {
        return WizEditor.getSkinResourcePath() + "unchecked.png";
    }

    function canEdit() {
        return editor.body.contentEditable == "true";
    }

    function initCss(document) {
        
    	var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
		var WIZ_STYLE = 'wiz_style';
		var WIZ_LINK_VERSION = 'wiz_link_version';
		var WIZ_TODO_STYLE_VERSION = "01.00.09";
        
		var style = document.getElementById(WIZ_TODO_STYLE_ID);
		if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
			return;
		//
		if (style && style.parentElement) { 
			style.parentElement.removeChild(style);
		}
		//
		var strStyle = '.wiz-todo, .wiz-todo-img {width: 16px; height: 16px; cursor: default; padding: 0 10px 0 2px; vertical-align: -10%;-webkit-user-select: none;} .wiz-todo-label { display: inline-block; padding-top: 7px; padding-bottom: 6px; line-height: 1.5;} .wiz-todo-label-checked { /*text-decoration: line-through;*/ color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;} .wiz-todo-completed-info {padding-left: 44px; display: inline-block; } .wiz-todo-avatar { width:20px; height: 20px; vertical-align: -20%; margin-right:10px; border-radius: 2px;} .wiz-todo-account, .wiz-todo-dt { color: #666; }';
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

    function setDocumentType(type) {
        WizEditor.setCurrentDocumentType(type);
    } 
}


function WizTodoAndroidHelper() {

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
	this.canInsert = canInsert;

	function getUserAlias() {
		return window.WizNote.getUserAlias();
	}

	function getUserAvatarFileName(size) {
		return window.WizNote.getUserAvatarFileName(size);
	}

	function isPersonalDocument() {
		return window.WizNote.isPersonalDocument();
	}

	function getLocalDateTime(dt) {
		return window.WizNote.getLocalDateTime(dt);
	}

	function setDocumentModified() {
		window.WizNote.setDocumentModified();
	}

	function getCheckedImageFileName() {
		return window.WizNote.getCheckedImageFileName();
	}

	function getUnCheckedImageFileName() {
		return window.WizNote.getUnCheckedImageFileName();
	}

	function canEdit() {
		return true;
	}

	function initCss(document) {
		initDefaultCss(document, document.head);
	}

	function setDocumentType(type) {
		window.WizNote.setDocumentType(type);
	}

	function canInsert(caninsert) {
		window.WizNote.canInsert(caninsert);
	}
}

function WizTodoIphoneHelper() {

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
	this.setUserAlias = setUserAlias;
	this.setUserAvatarFileName = setUserAvatarFileName;
	this.setCheckedImageFileName = setCheckedImageFileName;
	this.setUnCheckedImageFileName = setUnCheckedImageFileName;
	this.onAddTodoCompletedInfo = onAddTodoCompletedInfo;
	this.setIsPersonalDocument = setIsPersonalDocument;
	this.canInsert = canInsert;
    
	this.userAlias = null;
	this.avatarFileName = null;
	this.checkedFileName = null;
	this.unCheckedFileName = null;
	this.personalDocument = undefined;
    
	function setUserAlias(alias) {
		this.userAlias = alias;
	}

	function setUserAvatarFileName(avatarFileName) {
		this.avatarFileName = avatarFileName;
	}

	function setCheckedImageFileName(fileName) {
		this.checkedFileName = fileName;
	}

	function setUnCheckedImageFileName(fileName) {
		this.unCheckedFileName = fileName;
	}

	function setIsPersonalDocument(isPersonalDocument) {
		this.personalDocument = isPersonalDocument;
	}

	function getUserAlias() {
		return this.userAlias;
	}
    
	function getUserAvatarFileName(size) {
        return this.avatarFileName;
	}
    
	function isPersonalDocument() {
		return this.personalDocument === 'true';
	}
    
	function getLocalDateTime(dt) {
        return "";
	}
    
	function setDocumentModified() {
        window.location.href = "wiztodolist://setDocumentModified/";
	}
    
	function getCheckedImageFileName() {
		return this.checkedFileName;
	}
    
	function getUnCheckedImageFileName() {
		return this.unCheckedFileName;
	}
    
	function canEdit() {
		return true;
	}
    
	function initCss(document) {
		initDefaultCss(document, document.head);
	}

	function setDocumentType(type) {
        window.location.href = "wiztodolist://setDocumentType/" + "?type="+ type;
	}

	function onAddTodoCompletedInfo(isChecked, id, dt, callBack) {
        var href = "wiztodolist://onAddTodoCompletedInfo/" + "?checked="+ isChecked +"&id="+ id +"&dt="+ dt +"&callback="+ callBack;
		window.location.href = href;
	}
    
	function canInsert(caninsert) {
		if (caninsert)
			return;
		window.location.href = "wiztodolist://canInsert/" + "?canInsert"+ caninsert;
	}
}


var WizTodo = (function () {

	var WIZ_HTML_CLASS_WIZ_TODO = 'wiz-todo-img';
	var WIZ_HTML_CLASS_CANNOT_DRAG = 'wiz-img-cannot-drag';
	var WIZ_HTML_TODO_COMPLETED_INFO = 'wiz-todo-completed-info';
	var WIZ_HTML_TODO_AVATAR_SIZE = 40;
	var WIZ_DOC_TODO_TYPE = "tasklist";
	var editorDocument = null;
	var client = "";
	var todoHelper = null;

	var curTouchTarget = null;
	var enterkeyDown = false;
	var enterkeyInTodo = false;

	function getTodoHelper(wizClient) {

		switch(wizClient) {
			case 'windows':
				return new WizTodoWindowsHelper(external);
			case 'qt':
				return new WizTodoQtHelper();
			case 'iphone':
				return new WizTodoIphoneHelper();
			case 'web':
				break;
			case 'android':
				return new WizTodoAndroidHelper();
				break;
		}
	}

	function isIphone() {
		return 'iphone' === client;
	}

	function isIpad() {
		return 'ipad' === client;
	}

	function isAndroid() {
		return 'android' === client;
	}

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

	function pasteHtmlAtCaret(html, selectPastedContent) {
	    var sel, range, retNode;
	    if (editorDocument.getSelection) {
	        // IE9 and non-IE
	        sel = editorDocument.getSelection();
	        if (sel.getRangeAt && sel.rangeCount) {
	            range = sel.getRangeAt(0);
	            range.deleteContents();

	            // Range.createContextualFragment() would be useful here but is
	            // only relatively recently standardized and is not supported in
	            // some browsers (IE9, for one)
	            var el = editorDocument.createElement("div");
	            el.innerHTML = html;
	            var frag = editorDocument.createDocumentFragment(), node, lastNode;
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
	    } else if ( (sel = editorDocument.selection) && sel.type != "Control") {
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

	function isLabel(ele) {
		if (!ele)
			return false;
		//
		if (-1 != getClassValue(ele).indexOf('wiz-todo-label'))
			return true;
		//
		return false;
	}

	function isTodoImage(ele) {
		if (!ele)
			return false;
		if (!ele.getAttribute || -1 == getClassValue(ele).indexOf(WIZ_HTML_CLASS_WIZ_TODO))
			return false;
		//
		return true;
	}

	function isCompletedInfo (ele) {
		if (!ele)
			return false;
		if (!ele.getAttribute || -1 == getClassValue(ele).indexOf(WIZ_HTML_TODO_COMPLETED_INFO))
			return false;
		//
		return true;
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

	function isBlockNode(node) {
		if (!node)
			return false;
		//
		if (node.nodeType == 9 || node.nodeType == 11)
			return true;
		//
		var displayValue = getElementDisply(node);
		if (!displayValue)
			return false;
		//
		var value = displayValue.toString().toLowerCase().trim();
		//
		if (!value)
			return false;
		//
		if (value != 'inline' 
			&& value != 'inline-block' 
			&& value != 'inline-table'
			&& value != 'none') {
			return true;
		}
		//
		return false;
	}

	function isInlineNode(node) {
		return !isBlockNode(node);
	}

	function isEmptyNode(node) {
		if (!node)
			return false;
		//
		if (node.normalize) { 
			node.normalize();
		}
		//
		if (!node.hasChildNodes())
			return true;
		//
		var childnodes = node.childNodes;
		for (var i = 0; i < childnodes.length; i ++) {
			var child = childnodes[i];
			//g
			if (child.nodeType == 3) {
				if (child.nodeValue != "")
					return false;
			}
			//
			if (!isEmptyNode(child))
				return false;
		}
		//
		return true;
	}

	function getBlockParentElement(ele) {
		if (!ele)
			return null;
		//
		var p = ele;
		while (p) {
			if (p.tagName && p.tagName.toLowerCase() == 'body')
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
			if (child.nodeType == 3) {
				if (child.nodeValue === "") {
					ele.removeChild(child);
				}
				var v = child.nodeValue.replace(/\u200B/g, '');
				if (0 == v.length)
					ele.removeChild(child);
			}
		}
	}

	function isTodoAtFirst(ele) {
		if (!ele)
			return false;
		//
		if (!ele.hasChildNodes())
			return false;
		//
		removeInvalidText(ele);
		//
		var label = null;
		var todoimg = null;
		var child = ele.childNodes[0];
		while (child) {
			if (isLabel(child)) {
				label = child;
				break;
			}
			if (isTodoImage(child)) {
				todoimg = child;
				break;
			}
			//
			if (!child.hasChildNodes())
				break;
			//
			child = child.childNodes[0];
		}
		//
		if (label) {
			var todoLabel = label;
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
		else if (todoimg) {
			return true;
		}
		//
		return false;
	}

	var g_canInsertWizTodo = false;
	function canInsertWizTodo() {
		return !!g_canInsertWizTodo;
	}

	function addCompletedInfo(label, isChecked, todoId, localDateTime) {
		if (!label)
			return;
		//
		if (isChecked) {
			var html = 	"<span class='wiz-todo-account'>" + 
								"<img src='%1' class='%2'>" +
								"%3, " + 
						"</span>" + 
						"<span class='wiz-todo-dt'>%4.</span>";
			//
			var userName = todoHelper.getUserAlias();
			var avatar = todoHelper.getUserAvatarFileName(WIZ_HTML_TODO_AVATAR_SIZE);
			//
			html = html.replace('%1', avatar);
			html = html.replace('%2', WIZ_HTML_CLASS_CANNOT_DRAG + ' ' + 'wiz-todo-avatar');
			html = html.replace('%3', userName);
			html = html.replace('%4', localDateTime);
			//
			var info = editorDocument.createElement('span');
			info.className = WIZ_HTML_TODO_COMPLETED_INFO;
			info.innerHTML = html;
			info.setAttribute('wiz_todo_id', todoId);
			//
			for (var i = label.childNodes.length - 1; i >= 0; i --) {
				var child = label.childNodes[i];
				//
				if (child.tagName && child.tagName.toLowerCase() == 'br') {
					label.removeChild(child);
				}
			}
			//
			var nextSib = label.nextElementSibling;
			while (nextSib) {
				if (isLabel(nextSib)) {
					label.parentElement.insertBefore(info, nextSib);
					break;
				}
				//
				if (nextSib.tagName.toLowerCase() == 'br') {
					label.parentElement.insertBefore(info, nextSib);
					break;
				}
				//
				nextSib = nextSib.nextElementSibling;
			}
			//
			if (!nextSib) {
				label.parentElement.appendChild(info);
			}
			//
			setCaret(info);
		}
		else {// todo: find todo id
			var info = label.parentElement.getElementsByClassName(WIZ_HTML_TODO_COMPLETED_INFO);
			if (!info || info.length < 1)
				return;
			//
			for (var i = 0; i < info.length; i ++) {
				var child = info[0];
				var tmpLabel = child.getElementsByClassName('wiz-todo-label');
				var chileNextSib = child.nextElementSibling;
				//
				if (tmpLabel && tmpLabel.length > 0) {
					var nextSib = tmpLabel[0];
					while (nextSib) {
						var tmpNext = nextSib;
						nextSib = nextSib.nextSibling;
						child.parentElement.insertBefore(tmpNext, chileNextSib);
					}
				}
			}
			//
			var nextSib = label.nextElementSibling;
			while (nextSib) {
				if (isLabel(nextSib))
					break;
				//
				if (isCompletedInfo(nextSib)) {
					var tmpNode = nextSib;
					nextSib = nextSib.nextElementSibling;
					label.parentElement.removeChild(tmpNode);
					continue;
				}
				//
				nextSib = nextSib.nextElementSibling;
			}
		}
	}

	function formatIntToDateString(n){
		
		return n < 10 ? '0' + n : n;
	}

	function ToDateString(dt){
       	//
        var ret = dt.getFullYear() + "-" + 
	    			formatIntToDateString(dt.getMonth() + 1) + "-" + 
	    			formatIntToDateString(dt.getDate()) + "T" + 
	    			formatIntToDateString(dt.getHours())+ ":" + 
	    			formatIntToDateString(dt.getMinutes()) + ":" + 
	    			formatIntToDateString(dt.getSeconds());
        return ret;
    }

	function onTodoClick(todoEle) {
		//
		var label = getParentTodoLabelElement(todoEle);
		// todo img add a label parent.
		if (!label) {
			label = editorDocument.createElement('label');
			label.className = 'wiz-todo-label wiz-todo-label-unchecked';
			todoEle.parentElement.insertBefore(label, todoEle);
			var nextSib = todoEle;
			while (nextSib) {
				//
				if (isBlockNode(nextSib))
					break;
				//
				label.appendChild(nextSib);
				//
				nextSib = nextSib.nextSibling;
			}
		}
		//
		var classValue = getClassValue(label);
		//
		var isChecked = todoEle.getAttribute('state') == 'checked';
		var imgSrc = isChecked ? todoHelper.getUnCheckedImageFileName() : todoHelper.getCheckedImageFileName();
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
		//			
		todoEle.src = imgSrc;
		todoEle.setAttribute('state', state);
		label.setAttribute('class', classValue);
		//update _src for ueditor
		var backupSrc = todoEle.getAttribute('_src');
		if (backupSrc) {
			todoEle.setAttribute('_src', imgSrc);
		}
		//
		if (!todoHelper.isPersonalDocument()) {

			if (isIpad() || isIphone()) {
				todoHelper.onAddTodoCompletedInfo(!isChecked, todoEle.id, Date.now(), 'addTodoCompletedInfo');
			}
			else {
				var dt = todoHelper.getLocalDateTime(new Date());
				addCompletedInfo(label, !isChecked, todoEle.id, dt);
			}
		}
		//
		wrapSiblingIntoLabel(label);
		//
		var parent = getBlockParentElement(label);
		if (!parent) {
			var div = document.createElement('div');
			label.parentElement.insertBefore(div, label.nextSibling);
			div.appendChild(label);
		}
		//
		if (todoHelper.isPersonalDocument()) {
			todoHelper.setDocumentModified();	
		}
		else {
			if (!isIpad() && !isIphone()) {
				todoHelper.setDocumentModified();
			}
		}
	}
	
	function isEmptyLabel(label) {
		if (!label.hasChildNodes())
			return true;
		//
		var childnodes = label.childNodes;
		//
		for (var i in childnodes) {
			var node = childnodes[i];
			//
			if (node && isTodoImage(node))
				return false;
		}
		//
		return true;
	}

	function isSelectionInEmptyLabel(retLabel) {
		retLabel.ret = null;
		//
		var rng = editorDocument.getSelection().getRangeAt(0);
		//
		var start = rng.startContainer;
		var label = getParentTodoLabelElement(start);
		if (!label)
			return false;
		retLabel.ret = label;
		//
		return isEmptyLabel(label);
	}

	function isSelectionExactlyAfterTodoImage() {
		var rng = editorDocument.getSelection().getRangeAt(0);
		//
		var start = rng.startContainer;
		if (!start)
			return false;
		if (isWizTodoBlockElement(start)) {
			if (!start.hasChildNodes())
				return false;
			//
			for (var i in start.childNodes) {
				if (i + 1 > rng.startOffset)
					return false;
				var child = start.childNodes[i];
				//
				if (child && child.tagName 
					&& -1 != getClassValue(child).indexOf('wiz-todo-label')
					&& i + 1 == rng.startOffset) {
					
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
			var child0 = label.childNodes[0];
			//
			if (!isTodoImage(child0))
				return false;
			//
			var nextSib = child0.nextSibling;
			while (nextSib) {
				if (nextSib.nodeType == 3) {
					var val = nextSib.nodeValue;
					val = val.replace(/\s/ig, '');
					if (val !== "")
						return false;
					else {
						nextSib = nextSib.nextSibling;
						continue;
					}
				}
				//
				if (!nextSib.tagName)
					return false;
				//
				removeInvalidText(nextSib);
				//
				if (!isEmptyNode(nextSib)) 
					return false;
				//
				if (nextSib.tagName.toLowerCase() != 'br' && !g_emptyCanRemove[nextSib.tagName])
					return false;
				//
				nextSib = nextSib.nextSibling;
			}
			if (1 != rng.startOffset || 1 != rng.endOffset)
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
			var sel = editorDocument.getSelection();
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
			// p.insertBefore(editorDocument.createElement('br'), p.firstChild);
			//
			var range = editorDocument.createRange();
			range.setStart(p, 0);
			range.setEnd(p, 1);
			//
			sel.removeAllRanges();
			sel.addRange(range);
		}
	}

	function getSelectionBlockParent() {
		var rng = editorDocument.getSelection().getRangeAt(0);
		//
		var p = rng.startContainer;
		//
		while (p && !isWizTodoBlockElement(p)) {
			p = p.parentElement;
		}
		//
		if (p && p.tagName.toString().toLowerCase() == 'body')
			return p;
		if (p && p.id && p.id == 'content-view-body')
			return p;
		if (p && p.tagName.toLowerCase() == 'td')
			return p;
		if (p && p.tagName.toLowerCase() == 'th')
			return p;
		//
		return null;
	}

	function isShouldWrapBlockElement(ele) {
		if (!ele)
			return false;
		if (ele && ele.tagName.toString().toLowerCase() == 'body')
			return true;
		// editable div of android 
		if (ele && ele.id && ele.id == 'content-view-body')
			return true;
		if (ele && ele.tagName.toString().toLowerCase() == 'td')
			return true;
		if (ele && ele.tagName.toString().toLowerCase() == 'th')
			return true;
		//
		return false;
	}

	function wrapMainEditableInlineChildren(editableNode) {
		if (!editableNode)
			return;
		//
		if (!editableNode.hasChildNodes())
			return;
		//
		var childnodes = editableNode.childNodes;
		//
		var div = editorDocument.createElement('div');
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
			var sel = editorDocument.getSelection();
			//
			editableNode.insertBefore(div, editableNode.firstChild);
			//
			range = editorDocument.createRange();
			range.setStartAfter(div.lastChild);
			range.setEndAfter(div.lastChild);
			//
			sel.removeAllRanges();
			sel.addRange(range);
		}
	}

	function setCaret(ele) {
		if (!ele)
			return;
		if (!ele.hasChildNodes()) {
			ele.appendChild(editorDocument.createElement('br'));
		}
		var sel = editorDocument.getSelection();
		//
		var range = editorDocument.createRange();
		range.setStartAfter(ele.lastChild);
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
		var sel = editorDocument.getSelection();
		//
		var p = label.parentElement;
		if (!p || !isLabel(p))
			return;
		//
		p.parentElement.insertBefore(label, p.nextElementSibling);
		//
		setCaret(label);
	}

	function wrapSiblingIntoLabel(label) {
		if (!label)
			return;
		//
		var nextSib = label.nextSibling;
		while (nextSib) {
			//
			var tmpNext = nextSib;
			nextSib = nextSib.nextSibling;
			//
			if (isLabel(tmpNext) || isCompletedInfo(tmpNext) || isBlockNode(tmpNext))
				break;
			//
			label.appendChild(tmpNext);
		}
	}

	function insertOneTodo() {

		// deleteEmptyLabel();
		if (!canInsert())
			return;
		//
		var strHTML = "<label class='wiz-todo-label wiz-todo-label-unchecked'>" + 
						"<img id='%1' class='wiz-todo-img wiz-img-cannot-drag' src='%2' state='unchecked'><span class='wiz-todo-tail'></span>" + 
					  "</label>";
		//
		strHTML = strHTML.replace("%1", 'wiz_todo_' + Date.now() + '_' + Math.floor((Math.random()*1000000) + 1));
		strHTML = strHTML.replace("%2", todoHelper.getUnCheckedImageFileName());
		//
		var label = pasteHtmlAtCaret(strHTML, false);
		removeNestLabel(label);
		//
		var p = getSelectionBlockParent();
		if (isShouldWrapBlockElement(p)) {
			wrapMainEditableInlineChildren(p);
		}
		//
		// mergeNextSibilingTextChild(label, !!fromKeyUp);
		//
		divideFromParentLabel(label);
		//
		setCaret(label);
		//
		wrapSiblingIntoLabel(label);
		//
		todoHelper.setDocumentType(WIZ_DOC_TODO_TYPE);
		todoHelper.setDocumentModified();
		//
		return label;
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

	function getFirstLabelBeforeSelection() {

	}

	function getParentCompletedInfo(ele) {
		if (!ele)
			return null;
		//
		var p = ele;
		while (p) {
			if (isCompletedInfo(p))
				return p;
			//
			p = p.parentElement;
		}
		//
		return null;
	}

	function getLabelBeforeCaret() {
		var sel = editorDocument.getSelection();
		//
		if (!sel || sel.type.toLowerCase() == 'none')
			return null;
		if (sel.type.toLowerCase() != 'caret')
			return null;
		//
		var rng = sel.getRangeAt(0);
		//
		var start = rng.startContainer;
		var completed = getParentCompletedInfo(start);
		//
		if (isCompletedInfo(completed)) {
			var prev = completed;
			while (prev) {
				if (isLabel(prev))
					return prev;
				//
				prev = prev.previousElementSibling;
			}
		}
		//
		if (isWizTodoBlockElement(start)) {
			if (!start.hasChildNodes())
				return null;
			//
			for (var i = 0; i < start.childNodes.length; i ++) {
				if (i + 1 > rng.startOffset)
					return null;
				var child = start.childNodes[i];
				//
				if (child && child.tagName 
					&& -1 != getClassValue(child).indexOf('wiz-todo-label')
					&& i + 1 == rng.startOffset) {
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

	function removeSelfOnly(ele) {
		if (!ele)
			return null;
		if (!ele.hasChildNodes()) {
			ele.parentElement.removeChild(ele);
			return null;
		}
		var firstChild = ele.firstChild;
		//
		while (ele.childNodes.length > 0) {
			ele.parentElement.insertBefore(ele.childNodes[0], ele);
		}
		//
		ele.parentElement.removeChild(ele);
		//
		return firstChild;
	}

	function WIZTODOMAKEOBJECT(s) {
        for (var k in s) {
            s[k.toUpperCase()] = s[k];
        }
        return s;
    }

	g_emptyCanRemove = WIZTODOMAKEOBJECT({a:1,abbr:1,acronym:1,address:1,b:1,bdo:1,big:1,cite:1,code:1,del:1,dfn:1,em:1,font:1,i:1,ins:1,label:1,kbd:1,q:1,s:1,samp:1,small:1,span:1,strike:1,strong:1,sub:1,sup:1,tt:1,u:1,'var':1});

	function isCompleteInfoNode(className) {
		if (!className)
			return false;
		//
		if (-1 != className.indexOf('wiz-todo-completed-info')
			|| -1 != className.indexOf('wiz-todo-account')
			|| -1 != className.indexOf('wiz-todo-dt')) {
			return true;
		}
		//
		return false;
	}

	function emptyCanRemove(node) {
		if (!node)
			return false;
		//
		if (!node.tagName)
			return false;
		//
		if (isEmptyNode(node) && g_emptyCanRemove[node.tagName] && isTodoTag(getClassValue(node)))
			return true;
		//
		return false;
	}

	function removeBlockElement(ele) {
		if (!ele)
			return null;
		//
		if (!ele.hasChildNodes())
			return ele;
		//
		var childnodes = ele.childNodes;
		for (var i = childnodes.length - 1; i >= 0; i --) {
			var child = childnodes[i];
			//
			removeBlockElement(child);
			//
			if (isLabel(child)) {

				if (isEmptyNode(child)) {
					child.parentElement && child.parentElement.removeChild(child);
				}
				else if (isEmptyLabel(child)) {
					removeSelfOnly(child);
				}
				//
				continue;
			}
			else if (child.tagName && child.tagName.toLowerCase() == 'br'
				&& isTodoTag(getClassValue(child.parentElement))) {
				child.parentElement && child.parentElement.removeChild(child);
				continue;
			}
			else if (emptyCanRemove(child)) {
				child.parentElement && child.parentElement.removeChild(child);
				continue;
			}
			else if (isInlineNode(child) && child.nodeType != 3 && child.tagName.toLowerCase() != 'br') {
				child.parentElement && ele.parentElement.insertBefore(child, ele.nextSibling);
				continue;
			}
		}
		//
		if (isBlockNode(ele)) {
			return removeSelfOnly(ele);
		}
		//
		if (emptyCanRemove(ele)) {
			ele.parentElement && ele.parentElement.removeChild(ele);
			return null;
		}
		//
		return ele;
	}

	function getFirstLabel(ele) {
		if (!ele)
			return null;
		//
		if (isLabel(ele))
			return ele;
		if (!ele.hasChildNodes())
			return null;
		//
		var childnodes = ele.childNodes;
		for (var i = 0; i < childnodes.length; i ++) {
			var child = childnodes[i];
			//
			if (isLabel(child))
				return child;
			//
			var l = getFirstLabel(child);
			if (l)
				return l;
		}
		//
		return null;
	}

	function getFirstLabelInLine() {
		var sel = editorDocument.getSelection();
		if (sel.type.toLowerCase() == 'none' || sel.rangeCount < 1)
			return;
		//
		var rng = sel.getRangeAt(0);
		//
		var start = rng.startContainer;
		var p = getBlockParentElement(start);
		if (!p)
			return null;
		//
		if (!p.hasChildNodes())
			return null;
		//
		var label = getFirstLabel(p);
		//
		return label;
	}

	function isTodoTag(className) {
		if (!className)
			return false;
		if (-1 != className.indexOf('wiz-todo-label')
			|| -1 != className.indexOf('wiz-todo-tail')
			|| isCompleteInfoNode(className)) {
			return true;
		}
		//
		return false;
	}

	function isEnterKeyInTodo(range) {
		if (!range)
			return false;
		//
		var start = range.startContainer;
		if (isBlockNode(start)) {
			return false;
		}
		//
		var p = start;
		while (p) {
			if (isTodoTag(getClassValue(p)))
				return true;
			//
			p = p.parentElement;
		}
		//
		return false;
	}

	function initEnterKeyState() {
		enterkeyDown = true;
		//
		var sel = editorDocument.getSelection();
		var rng = sel.getRangeAt(0);
		//
		enterkeyInTodo = isEnterKeyInTodo(rng) ? true : false;
	}

	function isInUeditor() {
		var ueditor = document.getElementById('ueditor_0');
		return ueditor ? true : false;
	}

	function onKeyDown(e) {

		if (!todoHelper.canEdit())
			return;
		//
		if (13 != e.keyCode) // Return key
			return;
		if (e.shiftKey)
			return;
		//
		initEnterKeyState();
		//
		var sel = editorDocument.getSelection();
		if (sel.type.toLowerCase() == 'none')
			return;
		//
		var rng = sel.getRangeAt(0);
		//
		if (0 == rng.startOffset && 0 == rng.endOffset) {
			var start = rng.startContainer;
			if (start && isLabel(start)) { // <div>{caret at here}<label>...</label></div>
				var p = getBlockParentElement(start);
				if (p) {
					var node = editorDocument.createElement(p.tagName);
					var br = editorDocument.createElement("br");
					node.insertBefore(br, null);
					//
					p.parentElement.insertBefore(node, p);
					e.preventDefault();					
				}
			}
			//
			g_canInsertWizTodo = false;
			enterkeyInTodo = false;
			return;
		}
		//should not get this label, is a bug.It should get the first label in the line.
		var label = null;
		/*
		if (sel.type.toLowerCase() == 'caret') {
			label = getLabelBeforeCaret();
		}
		else {
			var start = rng.startContainer;
			start = start.nodeType == 1 ? start : start.parentElement;
			//
			label = getParentTodoLabelElement(start);
		}*/
		label = getFirstLabelInLine();
		//
		var parentEle = getBlockParentElement(label);
		//
		if (isWizTodoBlockElement(parentEle) && isTodoAtFirst(parentEle)) {

			var pp = parentEle;
			if (isInUeditor() && parentEle.parentElement.tagName && parentEle.parentElement.tagName.toLowerCase() == 'li') {
				pp = parentEle.parentElement;
			}

			if (isSelectionExactlyAfterTodoImage() && hasPrevSiblingTodo(pp)) {

				var hasBr = false;
				var nextSib = label.nextSibling;
				while (nextSib) {
					if (nextSib.tagName && nextSib.tagName.toLowerCase() == 'br') {
						hasBr = true;
						break;
					}

					//
					nextSib = nextSib.nextSibling;
				}
				//
				if (isInUeditor() && pp === parentEle.parentElement) {
					var ppp = pp.parentElement;
					ppp.removeChild(pp);
					//
					var div = editorDocument.createElement('div');
					div.appendChild(editorDocument.createElement('br'));
					//
					ppp.parentElement.insertBefore(div, ppp.nextSibling);
					//
					setCaret(div);
				}
				else {
					var p = label.parentElement;
					p.removeChild(label);
					if (!hasBr) {
						p.appendChild(editorDocument.createElement('br'));
					}
					setCaret(p);
				}
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
		//
		if (g_canInsertWizTodo) {
			if (parentEle) {
				var eleName = parentEle.tagName.toLowerCase() == 'body' ? 'div' : parentEle.tagName;
				var ele = editorDocument.createElement(eleName);
				ele.innerHTML = "<br/>";
				//
				var pp = parentEle.parentElement;
				var isInLiNodeInUE = false;
				//
				if (isInUeditor() && pp && pp.tagName && pp.tagName.toLowerCase() == 'li') {
					var liNode = editorDocument.createElement('li');
					liNode.appendChild(ele);
					//
					pp.parentElement.insertBefore(liNode, pp.nextSibling);
					//
					isInLiNodeInUE = true;
				}
				else {
					parentEle.parentElement.insertBefore(ele, parentEle.nextSibling);
				}
				//
				var br = ele.lastChild;
				//
				rng.deleteContents();
				rng.setEndAfter(parentEle);
				var frag = rng.extractContents();
				//				
				setCaret(ele);
				var label = insertOneTodo();
				//
				var firstChild = frag.firstChild;
				label.appendChild(frag);
				removeBlockElement(firstChild);
				//
				e.preventDefault();
				if (isInLiNodeInUE) {
					e.stopPropagation();
				}
				//
				if (br) {
					br.parentElement.removeChild(br);
				}
				//
				if (label.getBoundingClientRect().top + label.clientHeight > document.documentElement.clientHeight
					|| label.getBoundingClientRect().top + label.clientHeight < 0) {
					var labelX = label.getBoundingClientRect().left + document.body.scrollLeft;
					var labelY = document.body.scrollTop + label.clientHeight;
					//
					window.scrollTo(labelX, labelY);
				}
			}
		}
	}

	function onKeyUp(e) {

        if (!todoHelper.canEdit())
            return;
        //
		if (13 != e.keyCode) // Return key
			return;
		if (!enterkeyDown || !enterkeyInTodo)
			return;
		//		
		enterkeyDown = false;
		enterkeyInTodo = false;
		//
		if (!canInsertWizTodo()) { 
			var sel = editorDocument.getSelection();
			if (sel.type.toLowerCase() == 'none')
				return;
			//
			var rng = sel.getRangeAt(0);
			//
			var start = rng.startContainer;
			var p = getBlockParentElement(start);
			//
			var node = removeBlockElement(p.firstChild);
			if (node && node.tagName && node.tagName.toLowerCase() != 'hr') {
				if (node.tagName.toLowerCase() == 'br') {
					node = node.parentElement;
				}
				//
				setCaret(node);
			}
			else {
				if (node) {
					setCaret(node);
				}
				else {
					setCaret(p);
				}
			}
		}
	}

	function onDocumentClick(e) {
		if (!todoHelper.canEdit())
			return;
		//
		var node = e.target;
		if (!node)
			return;
		if (!node.className)
			return;
		if (-1 != getClassValue(node).indexOf('wiz-todo-img')) {
			onTodoClick(node);
		}
	}

	function onTouchStart(e) {
		curTouchTarget = e.target;
	}

	function onTouchEnd(e) {
		if (e.target !== curTouchTarget)
			return;
		//
		curTouchTarget = null;
		//
		if (isIphone() || isIpad()) {
			onDocumentClick(e);
		}
	}

	function registerEvent() {
		editorDocument.removeEventListener('keydown', onKeyDown);
		editorDocument.removeEventListener('keyup', onKeyUp);
		editorDocument.removeEventListener('click', onDocumentClick);
		editorDocument.removeEventListener('touchstart', onTouchStart);
		editorDocument.removeEventListener('touchEnd', onTouchEnd);
		//
		editorDocument.addEventListener('keydown', onKeyDown, isInUeditor() ? true : false);
		editorDocument.addEventListener('keyup', onKeyUp);
		editorDocument.addEventListener('click', onDocumentClick);
		editorDocument.addEventListener('touchstart', onTouchStart);
		editorDocument.addEventListener('touchend', onTouchEnd);
	}

	function init(wizClient) { 

		var ueditor = null;
		if (wizClient == 'qt') {
			ueditor = document.getElementById('ueditor_0');
		}
		//
		editorDocument = ueditor ? ueditor.contentDocument : document;
		//
		client = wizClient;
		//
		todoHelper = getTodoHelper(wizClient);
		//
		registerEvent();
		//
		todoHelper.initCss(editorDocument);
	}

	function setUserAlias(alias) {
		if (todoHelper.setUserAlias) {
			todoHelper.setUserAlias(alias);
		}
	}

	function setUserAvatarFileName(avatarFileName) {
		if (todoHelper.setUserAvatarFileName) {
			todoHelper.setUserAvatarFileName(avatarFileName);
		}
	}

	function setCheckedImageFileName(fileName) {
		if (todoHelper.setCheckedImageFileName) {
			todoHelper.setCheckedImageFileName(fileName);
		}
	}

	function setUnCheckedImageFileName(fileName) {
		if (todoHelper.setUnCheckedImageFileName) {
			todoHelper.setUnCheckedImageFileName(fileName);
		}
	}

	function setIsPersonalDocument(isPersonalDocument) {
		if (todoHelper.setIsPersonalDocument) {
			todoHelper.setIsPersonalDocument(isPersonalDocument);
		}
	}

	function addTodoCompletedInfo(isChecked, id, localDateTime) {
		var todoImg = editorDocument.getElementById(id);
		var label = getParentTodoLabelElement(todoImg);
		//
		addCompletedInfo(label, isChecked === 'true', id, localDateTime);
	}

	function _canInsert() {

		var sel = editorDocument.getSelection();
		if (!sel || sel.rangeCount < 1)
			return false;
		//
		var rng = sel.getRangeAt(0);
		if (!rng || !rng.collapsed) 
			return false;
		//
		return true;
	}

	function canInsert() {

		var caninsert = _canInsert();
		//
		if (todoHelper.canInsert) {
			todoHelper.canInsert(caninsert);
		}
		//
		return caninsert;
	}

	function insertOneTodoForQt() {
		insertOneTodo();
	}

	return {
		init: init,
		insertOneTodo: insertOneTodo,
		insertOneTodoForQt: insertOneTodoForQt,
		canInsert: canInsert,
		setUserAlias: setUserAlias,
		setUserAvatarFileName: setUserAvatarFileName,
		setCheckedImageFileName: setCheckedImageFileName,
		setUnCheckedImageFileName: setUnCheckedImageFileName,
		setIsPersonalDocument: setIsPersonalDocument, 
		addTodoCompletedInfo: addTodoCompletedInfo,
	}
})();