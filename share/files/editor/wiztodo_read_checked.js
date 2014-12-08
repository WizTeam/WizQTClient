
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

function WizInitReadCss(document, destNode) {
	var WIZ_TODO_STYLE_ID = 'wiz_todo_style_id';
	var WIZ_STYLE = 'wiz_style';
	var WIZ_LINK_VERSION = 'wiz_link_version';
	var WIZ_TODO_STYLE_VERSION = "01.01.00";// must above todo edit css version

	var style = document.getElementById(WIZ_TODO_STYLE_ID);
	if (style && !!style.getAttribute && style.getAttribute(WIZ_LINK_VERSION) >= WIZ_TODO_STYLE_VERSION)
		return;
	//
	if (style && style.parentElement) { 
		style.parentElement.removeChild(style);
	}
	//
	var strStyle = '.wiz-todo, .wiz-todo-img {width: 16px; height: 16px; cursor: default; padding: 0 10px 0 2px; vertical-align: -7%;-webkit-user-select: none;} .wiz-todo-label { line-height: 2.5;} .wiz-todo-label-checked { /*text-decoration: line-through;*/ color: #666;} .wiz-todo-label-unchecked {text-decoration: initial;} .wiz-todo-completed-info {padding-left: 44px; display: inline-block; } .wiz-todo-avatar { width:20px; height: 20px; vertical-align: -20%; margin-right:10px; border-radius: 2px;} .wiz-todo-account, .wiz-todo-dt { color: #666; }';
	//
	var objStyle = document.createElement('style');
	objStyle.type = 'text/css';
	objStyle.textContent = strStyle;
	objStyle.id = WIZ_TODO_STYLE_ID;
	// objStyle.setAttribute(WIZ_STYLE, 'unsave');
	objStyle.setAttribute(WIZ_LINK_VERSION, WIZ_TODO_STYLE_VERSION);
	//
	destNode.appendChild(objStyle);
}

function WizTodoReadCheckedWindows (wizApp) {

	if (wizApp) {
		this.app = wizApp;
		this.doc = new WizDoc(wizApp.Window.CurrentDocument);
		this.personalDB = wizApp.PersonalDatabase;
		this.database = wizApp.Database;
		this.commonUI = wizApp.CreateWizObject('WizKMControls.WizCommonUI');
	}

	this.initCss = initCss;
	this.getDocHtml = getDocHtml;
	this.setDocHtml = setDocHtml;
	this.canEdit = canEdit;
	this.getCheckedImageFileName = getCheckedImageFileName;
	this.getUnCheckedImageFileName = getUnCheckedImageFileName;
	this.isPersonalDocument = isPersonalDocument;
	this.getUserAlias = getUserAlias;
	this.getUserAvatarFileName = getUserAvatarFileName;
	this.getLocalDateTime = getLocalDateTime;
	this.getWizDocument = getWizDocument;
	this.getAvatarName = getAvatarName;
	
	function initCss() {
		WizInitReadCss(document, document.head);
	}

	function getDocHtml() {
		return this.doc.getHtml();
	}

	function setDocHtml(html, resources) {
		//
	    var progress = this.app.CreateWizObject("WizKMControls.WizProgressWindow");
	    if (progress) {
	        progress.Text = this.doc.getTitle();
	        progress.Show();
	        progress.Max = 1;
    	}
        //
        this.doc.setHtml(html, resources);
        //
        if (progress) {
        	progress.Hide();
        	progress.Destroy();
    	}
	}

	function canEdit() {
		return this.doc.canEdit();
	}

	function getCheckedImageFileName() {
		return this.app.AppPath + "WizTools\\htmleditor\\checked.png";
	}

	function getUnCheckedImageFileName() {
		return this.app.AppPath + "WizTools\\htmleditor\\unchecked.png";
	}

	function isPersonalDocument() {
		return this.database.KbGUID == "";
	}

	function getLocalDateTime(dt) {
		return this.commonUI.ToLocalDateString(dt, true) + ' ' + this.commonUI.ToLocalTimeString2(dt, true);
	}

	function getUserAlias() {
		if (!this.personalDB || !this.database)
			return "";
		//
		var kbguid = this.database.KbGUID;
		if (!kbguid)
			return "";
		//
		return this.personalDB.GetUserAlias(kbguid);
	}

	function getUserAvatarFileName(size) {
		if (!this.database)
			return "";
		if (!this.database.GetAvatarFileName)
			return "";
		//
		var fileName = this.database.GetAvatarFileName();
		//
		var pos = fileName.lastIndexOf('\\');
		var filePath = fileName.substr(0, pos);
		var fileTitle = fileName.substr(pos + 1, fileName.lastIndexOf('.') - pos - 1);
		var ext = fileName.substr(fileName.lastIndexOf('.') + 1);
		//
		var cacheFileName = filePath + "\\" + fileTitle + size.toString() + "." + ext;
		if (this.commonUI.PathFileExists(cacheFileName))
			return cacheFileName;
		//		
		if (!this.commonUI || !this.commonUI.ResizeImage)
			return fileName;
		var newFileName = this.commonUI.ResizeImage(fileName, size);
		//
		if (!this.commonUI.PathFileExists(newFileName))
			return fileName;
		//
		this.commonUI.CopyFile(newFileName, cacheFileName);
		//
		return cacheFileName;		
	}

	function getWizDocument() {
		return this.doc.doc;
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

function WizTodoReadCheckedQt () {

	this.initCss = initCss;
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

    function initCss() {
    }

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

function WizTodoReadCheckedAndroid () {

	this.initCss = initCss;
	this.getDocHtml = getDocHtml;
	this.setDocHtml = setDocHtml;
	this.canEdit = canEdit;
	this.getCheckedImageFileName = getCheckedImageFileName;
	this.getUnCheckedImageFileName = getUnCheckedImageFileName;
	this.isPersonalDocument = isPersonalDocument;
	this.getUserAlias = getUserAlias;
	this.getUserAvatarFileName = getUserAvatarFileName;
	this.getLocalDateTime = getLocalDateTime;
	this.onDocumentClose = onDocumentClose;
	this.onTodoImageClicked = onTodoImageClicked;
	this.getAvatarName = getAvatarName;	

	function initCss() {
	}

	function getDocHtml() {
		return window.WizNote.getDocHtml();
	}

	function setDocHtml(html, resources) {
		window.WizNote.setDocHtml(html, resources);
	}

	function canEdit() {
		return window.WizNote.canEdit();
	}

	function getCheckedImageFileName() {
		return window.WizNote.getCheckedImageFileName();
	}

	function getUnCheckedImageFileName() {
		return window.WizNote.getUnCheckedImageFileName();
	}

	function isPersonalDocument() {
		return window.WizNote.isPersonalDocument();
	}

	function getLocalDateTime(dt) {
		return window.WizNote.getLocalDateTime(dt);
	}

	function getUserAlias() {
		return window.WizNote.getUserAlias();
	}

	function getUserAvatarFileName(size) {
		return window.WizNote.getUserAvatarFileName(size);
	}

	function onDocumentClose() {
		window.WizNote.onWizTodoReadCheckedClose();
	}

	function onTodoImageClicked() {

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

function WizTodoReadCheckedIphone() {
	
	this.initCss = initCss;
	this.getUserAlias = getUserAlias;
	this.getUserAvatarFileName = getUserAvatarFileName;
	this.isPersonalDocument = isPersonalDocument;
	this.getLocalDateTime = getLocalDateTime;
	this.getCheckedImageFileName = getCheckedImageFileName;
	this.getUnCheckedImageFileName = getUnCheckedImageFileName;
	this.getDocHtml = getDocHtml;
	this.setDocHtml = setDocHtml;
	this.canEdit = canEdit;
	this.setUserAlias = setUserAlias;
	this.setUserAvatarFileName = setUserAvatarFileName;
	this.setCheckedImageFileName = setCheckedImageFileName;
	this.setUnCheckedImageFileName = setUnCheckedImageFileName;
	this.setIsPersonalDocument = setIsPersonalDocument;
	this.setCanEdit = setCanEdit;
	this.setDocOriginalHtml = setDocOriginalHtml;
	this.getAvatarName = getAvatarName;
	this.onAddTodoCompletedInfo = onAddTodoCompletedInfo;

	this.userAlias = null;
	this.avatarFileName = null;
	this.checkedFileName = null;
	this.unCheckedFileName = null;
	this.personalDocument = undefined;
	this.canedit = null;
	this.originalHtml = "";
    
	function initCss() {
	}

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

	function setCanEdit(canEdit) {
		this.canedit = canEdit;
	}

	function setDocOriginalHtml(html) {
		this.originalHtml = html;
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
    
	function getCheckedImageFileName() {
		return this.checkedFileName;
	}
    
	function getUnCheckedImageFileName() {
		return this.unCheckedFileName;
	}
    
	function canEdit() {
		return this.canedit === 'true';
	}

	function getDocHtml() {
		return this.originalHtml;
	}

	function setDocHtml(html, resources) {
		window.location.href="wiztodolist://setDocHtml"+"&*/" + html +"&*/"+ resources;
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

	function onAddTodoCompletedInfo(isChecked, id, dt, callBack) {
		window.location.href = "wiztodolist://onAddTodoCompletedInfo" + "&*/"+ isChecked +"&*/"+ id +"&*/"+ dt +"&*/"+ callBack;
	}
}
var WizTodoReadChecked = (function () {

	var WIZ_HTML_CLASS_WIZ_TODO = 'wiz-todo';
	var WIZ_HTML_CLASS_CANNOT_DRAG = 'wiz-img-cannot-drag';
	var WIZ_HTML_TODO_COMPLETED_INFO = 'wiz-todo-completed-info';
	var WIZ_HTML_TODO_AVATAR_SIZE = 40;

	var helper = null;
	var wizClient = null;
	var modifiedTodos = {};
	var editorDocument = null;

	function getHelper(wizClient) {
		switch(wizClient) {
			case 'windows':
				return new WizTodoReadCheckedWindows(external);
			case 'qt':
				return new WizTodoReadCheckedQt();
			case 'android':
				return new WizTodoReadCheckedAndroid();
			case 'iphone':
				return new WizTodoReadCheckedIphone();
			case 'web':
				break;
		}
	}

	function isIphone() {
		return 'iphone' === wizClient;
	}

	function isIpad() {
		return 'ipad' === wizClient;
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

	function getParentTodoLabelElement(node) {
		if (!node)
			return null;
		//
		var p = node;
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

	function isLabel(ele) {
		if (!ele)
			return false;
		//
		if (-1 != getClassValue(ele).indexOf('wiz-todo-label'))
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

	function isCompletedInfo (ele) {
		if (!ele)
			return false;
		if (!ele.getAttribute || -1 == getClassValue(ele).indexOf(WIZ_HTML_TODO_COMPLETED_INFO))
			return false;
		//
		return true;
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
			var userName = helper.getUserAlias();
			var avatar = helper.getUserAvatarFileName(WIZ_HTML_TODO_AVATAR_SIZE);
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
			// setCaret(info);
			//
			if (modifiedTodos[todoId] === undefined) {
				modifiedTodos[todoId] = {};
			}
			//
			modifiedTodos[todoId]['completedInfo'] = info.outerHTML;
		}
		else {
			if (modifiedTodos[todoId] === undefined) {
				modifiedTodos[todoId] = {};
			}			
			modifiedTodos[todoId]['completedInfo'] = "";
			//
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

	function onTodoClick(todoEle) {

		if (helper.onTodoImageClicked) {
			helper.onTodoImageClicked();
		}

		if (!helper.canEdit())
			return;
		//
		if (!todoEle)
			return;
		var label = getParentTodoLabelElement(todoEle);
		// todo img add a label parent.
		if (!label) {
			label = editorDocument.createElement('label');
			label.className = 'wiz-todo-label wiz-todo-label-unchecked';
			todoEle.parentElement.insertBefore(label, todoEle);
			var nextSib = todoEle;
			while (!!nextSib && !isBlockNode(nextSib)) {
				//
				label.appendChild(nextSib);
				//
				nextSib = nextSib.nextSibling;
			}
			// only for read mode
			label.setAttribute('no-exist', 'no-exist');
		}
		//
		var classValue = getClassValue(label);
		//
		var isChecked = !(todoEle.getAttribute('state') == 'checked');
		var imgSrc = isChecked ? helper.getCheckedImageFileName() : helper.getUnCheckedImageFileName();
		var state = isChecked ? 'checked' : 'unchecked';
		//
		if (isChecked) {
			//
			if (-1 != classValue.indexOf('wiz-todo-label-unchecked')) {
				classValue = classValue.replace('wiz-todo-label-unchecked', 'wiz-todo-label-checked');
			}
			else {
				classValue += ' wiz-todo-label-checked';
			}				
		} 
		else {
			if (-1 != classValue.indexOf('wiz-todo-label-checked')) {
				classValue = classValue.replace('wiz-todo-label-checked', 'wiz-todo-label-unchecked');
			}
			else {
				classValue += ' wiz-todo-label-unchecked';
			}
		}
		//
		todoEle.src = imgSrc;		
		todoEle.setAttribute('state', state);
		label.setAttribute('class', classValue);
		//
		if (!helper.isPersonalDocument()) {
			if (isIpad() || isIphone()) {
				helper.onAddTodoCompletedInfo(isChecked, todoEle.id, new Date(), 'addTodoCompletedInfo');
			}
			else {
				var dt = helper.getLocalDateTime(new Date());
				addCompletedInfo(label, isChecked, todoEle.id, dt);
			}		
		}
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
		//
		var parent = getBlockParentElement(label);
		if (!parent) {
			var div = editorDocument.createElement('div');
			label.parentElement.insertBefore(div, label.nextSibling);
			div.appendChild(label);
		}
		//
		var id = todoEle.id;
		//
		// changeWizDocument(id);
		if (modifiedTodos[todoEle.id] === undefined) {
			modifiedTodos[todoEle.id] = {};
		}
		//
		modifiedTodos[todoEle.id]['state'] = isChecked ? "checked" : "unchecked";
		// modifiedTodos[todoEle.id]['completedInfo'] = isChecked ? completedInfo : "";
	}

	function onDocumentClick(e) {
		var node = e.target;
		if (!node)
			return;
		if (!node.className)
			return;
		if (-1 != getClassValue(node).indexOf('wiz-todo-img')) {
			onTodoClick(node);
		}
	}

	function registerEvent() {
		editorDocument.removeEventListener('click', onDocumentClick);
		editorDocument.addEventListener('click', onDocumentClick);
	}

	function init(client) {

		wizClient = client;
		//
		var ueditor = null;
		if (wizClient == 'qt') {
			ueditor = document.getElementById('ueditor_0');
		}
		//
		editorDocument = ueditor ? ueditor.contentDocument : document;
		helper = getHelper(wizClient);
		helper.initCss();
		//
		registerEvent();
	}

	function extractTodoText(id, html) {
		var todoImg = editorDocument.getElementById(id);
		//
		if (!todoImg)
			return "";
		//
		var todoText = "";
		//
		var label = getParentTodoLabelElement(todoImg);
		if (label && !label.getAttribute('no-exist')) {
			var pos = html.indexOf(id);
			if (-1 == pos)
				return "";
			//
			var html1 = html.substr(0, pos);
			var pos1 = html1.lastIndexOf("<label");
			//
			var html2 = html.substr(pos);
			var pos2 = html2.indexOf("</label>");
			//
			if (-1 == pos1 || -1 == pos2)
				return "";
			//
			todoText = html.substring(pos1, pos + pos2 + "</label>".length);
		}
		else {
			var regText = "<img[^>]*?id[ ]*?=[ ]*?['\"]{id}['\"]*?.*?>.*?<\/{tagname}>";
			regText = regText.replace("{id}", id);
			var parent = getBlockParentElement(todoImg);
			var tagName = (!!parent && parent.tagName) ? parent.tagName.toLowerCase() : "div";
			regText = regText.replace("{tagname}", tagName);
			var reg = new RegExp(regText, "igm");
			var matchs = html.match(reg);

			if (matchs && matchs.length > 0)
			{
				var reg = new RegExp("<\\/" + tagName + ">$", 'igm');
				todoText = matchs[0].replace(reg, "");
			}
			else
			{
				var pos = html.indexOf(id);
				if (-1 == pos)
					return "";
				//
				var html1 = html.substr(0, pos);
				var pos1 = html1.lastIndexOf("<img");
				//
				var html2 = html.substr(pos);
				var pos2 = html2.indexOf(">");
				//
				if (-1 == pos1 || -1 == pos2)
					return "";
				//
				todoText = html.substring(pos1, pos + pos2 + ">".length);				
			}
		}
		//
		return todoText;
	}

	function changeWizDocument(id, state, completedInfo, html) {
		// var html = helper.getDocHtml();
		if (!html)
			return null;
		//
		var todoText = extractTodoText(id, html);
		//
		if (todoText === "")
			return null;
		//
		var pos = html.indexOf(todoText);
		var firstHtml = html.substr(0, pos);
		var lastHtml = html.substr(pos + todoText.length);
		//
		if (state == "checked" && todoText.match(/state[ ]*=[ ]*['\"]unchecked['\"]/ig)) {

			todoText = todoText.replace("wiz-todo-label-unchecked", "wiz-todo-label-checked");
			todoText = todoText.replace("unchecked.png", "checked.png");
			todoText = todoText.replace("state='unchecked'", "state='checked'");
			todoText = todoText.replace("state=\"unchecked\"", "state=\"checked\"");
		}
		else if (state == "unchecked" && todoText.match(/state[ ]*=[ ]*['\"]checked['\"]/ig)){

			todoText = todoText.replace("wiz-todo-label-checked", "wiz-todo-label-unchecked");
			todoText = todoText.replace("checked.png", "unchecked.png");
			todoText = todoText.replace("state='checked'", "state='unchecked'");
			todoText = todoText.replace("state=\"checked\"", "state=\"unchecked\"");
		}
		else {
			// on current user is not same as original one.
			if (state == "checked" && completedInfo) {
				var reg = new RegExp("<span[^>]*?wiz_todo_id[ ]*=[ ]*['\"]" + id + "['\"][\\w\\W]*?wiz-todo-dt[\\w\\W]*?<\\/span><\\/span>", "igm");
				lastHtml = lastHtml.replace(reg, "");
				//
				html = firstHtml + todoText + completedInfo + lastHtml;
				return html;
			}
			//
			return null;
		}
		//
		if (-1 == todoText.indexOf('<label') || -1 == todoText.indexOf('wiz-todo-label')) {
			//
			if (state == "checked") {
				todoText = "<label class='wiz-todo-label wiz-todo-label-checked'>" + todoText + "</label>";
			}
			else {
				todoText = "<label class='wiz-todo-label wiz-todo-label-unchecked>" + todoText + "</label>";
			}
		}
		//
		html = firstHtml + todoText + lastHtml;
		//
		if (!helper.isPersonalDocument()) {
			if (state == "checked") {
				if (completedInfo) {
					html = firstHtml + todoText + completedInfo + lastHtml;
				}
			}
			else if (state == "unchecked"){
				var reg = new RegExp("<span[^>]*?wiz_todo_id[ ]*=[ ]*['\"]" + id + "['\"][\\w\\W]*?wiz-todo-dt[\\w\\W]*?<\\/span><\\/span>", "igm");
				//
				if (html.match(reg)) {
					html = html.replace(reg, "");
				}
			}
		}
		//
		return html;
	}

	function getObjectLength(obj) {
	     var count = 0;
	     for(var i in obj){
	         count ++;
	     }
	     return count;		
	}

	function saveHtml() {
		
		if (getObjectLength(modifiedTodos) < 1)
			return;
		//
		var html = helper.getDocHtml();
		//
		for (var id in modifiedTodos) {
			newHtml = changeWizDocument(id, modifiedTodos[id].state, modifiedTodos[id].completedInfo, html);
			if (newHtml) {
				html = newHtml;
			}
		}
		//
		var resources = helper.getCheckedImageFileName() + "*" + helper.getUnCheckedImageFileName();
		//
		var hasCompletedInfo = false;
		for (var id in modifiedTodos) {
			if (modifiedTodos[id].state == "checked" && modifiedTodos[id].completedInfo) {
				hasCompletedInfo = true;
				//				
				break;
			}
		}
		if (hasCompletedInfo) {
			var userAvatar = helper.getUserAvatarFileName(WIZ_HTML_TODO_AVATAR_SIZE);
			var avatarName = helper.getAvatarName(userAvatar);
			//
			if (userAvatar && avatarName) {
				
				var userAvatar2 = userAvatar.replace(/\\/g, '\\\\');
				var reg = new RegExp(userAvatar2, 'ig');
				//
				html = html.replace(reg, "index_files/" + avatarName);
				//
				resources += "*" + userAvatar;
			}
		}
        //
        modifiedTodos = {};
        //
        if (!isIpad() && !isIphone()) {
			helper.setDocHtml(html, resources);
		}
		else {
			return html;	
		}
	}

	function onDocumentClose() {
		var html = saveHtml();
		//
		if (helper.onDocumentClose) {
			helper.onDocumentClose();
		}
		//
		if (isIpad() || isIphone()) {
			return html;
		}
	}
	
	function getWizDocument() {
		return helper.getWizDocument();
	}

	function setCanEdit(canEdit) {
		if (helper.setCanEdit) {
			helper.setCanEdit(canEdit);
		}
	}

	function setUserAlias(alias) {
		if (helper.setUserAlias) {
			helper.setUserAlias(alias);
		}
	}

	function setUserAvatarFileName(avatarFileName) {
		if (helper.setUserAvatarFileName) {
			helper.setUserAvatarFileName(avatarFileName);
		}
	}

	function setCheckedImageFileName(fileName) {
		if (helper.setCheckedImageFileName) {
			helper.setCheckedImageFileName(fileName);
		}
	}

	function setUnCheckedImageFileName(fileName) {
		if (helper.setUnCheckedImageFileName) {
			helper.setUnCheckedImageFileName(fileName);
		}
	}

	function setIsPersonalDocument(isPersonalDocument) {
		if (helper.setIsPersonalDocument) {
			helper.setIsPersonalDocument(isPersonalDocument);
		}
	}

	function setDocOriginalHtml(html) {
		if (helper.setDocOriginalHtml) {
			helper.setDocOriginalHtml(html);
		}
	}

	function addTodoCompletedInfo(isChecked, id, localDateTime) {
		var todoImg = editorDocument.getElementById(id);
		var label = getParentTodoLabelElement(todoImg);
		//
		addCompletedInfo(label, isChecked === 'true', id, localDateTime);
	}

	return {
		init: init,
		onDocumentClose: onDocumentClose,
		getWizDocument: getWizDocument,
		setCanEdit: setCanEdit,
		setUserAlias: setUserAlias,
		setUserAvatarFileName: setUserAvatarFileName,
		setCheckedImageFileName: setCheckedImageFileName,
		setUnCheckedImageFileName: setUnCheckedImageFileName,
		setIsPersonalDocument: setIsPersonalDocument,
		setDocOriginalHtml: setDocOriginalHtml,
		addTodoCompletedInfo: addTodoCompletedInfo
	}

})();
