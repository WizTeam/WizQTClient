
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

function WizTodoReadCheckedWindows (wizApp) {

	if (wizApp) {
		this.app = wizApp;
		this.doc = new WizDoc(wizApp.Window.CurrentDocument);
		this.personalDB = wizApp.PersonalDatabase;
		this.database = wizApp.Database;
		this.commonUI = wizApp.CreateWizObject('WizKMControls.WizCommonUI');

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

function WizTodoReadCheckedAndroid () {
	this.getDocHtml = getDocHtml;
	this.setDocHtml = setDocHtml;
	this.canEdit = canEdit;
	this.getCheckedImageFileName = getCheckedImageFileName;
	this.getUnCheckedImageFileName = getUnCheckedImageFileName;
	this.isPersonalDocument = isPersonalDocument;
	this.getUserAlias = getUserAlias;
	this.getUserAvatarFileName = getUserAvatarFileName;
	this.getLocalDateTime = getLocalDateTime;

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
}


var WizTodoReadChecked = (function () {

	var WIZ_HTML_CLASS_WIZ_TODO = 'wiz-todo';
	var WIZ_HTML_CLASS_CANNOT_DRAG = 'wiz-img-cannot-drag';
	var WIZ_HTML_TODO_COMPLETED_INFO = 'wiz-todo-completed-info';
	var WIZ_HTML_TODO_AVATAR_SIZE = 24;

	var editorDocument = null;
	var helper = null;
	var wizClient = null;
	var modifiedTodos = {};

	function getHelper(wizClient) {
		switch(wizClient) {
			case 'windows':
				return new WizTodoReadCheckedWindows(external);
			case 'qt':
				return new WizTodoReadCheckedQt();
			case 'android':
				return new WizTodoReadCheckedAndroid();
				break;
			case 'iphone':
				break;
			case 'web':
				break;
		}
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

	function isCompletedInfo (ele) {
		if (!ele)
			return false;
		if (!ele.getAttribute || -1 == getClassValue(ele).indexOf(WIZ_HTML_TODO_COMPLETED_INFO))
			return false;
		//
		return true;
	}

	function addCompletedInfo(label, isChecked, todoId) {
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
			var dt = helper.getLocalDateTime(new Date());
			var userName = helper.getUserAlias();
			var avatar = helper.getUserAvatarFileName(WIZ_HTML_TODO_AVATAR_SIZE);
			//
			html = html.replace('%1', avatar);
			html = html.replace('%2', WIZ_HTML_CLASS_CANNOT_DRAG + ' ' + 'wiz-todo-avatar');
			html = html.replace('%3', userName);
			html = html.replace('%4', dt);
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
			return info.outerHTML;
		}
		else {
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
			while (nextSib) {
				//
				label.appendChild(nextSib);
				//
				nextSib = nextSib.nextSibling;
			}
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
		var completedInfo = "";
		if (!helper.isPersonalDocument()) {
			completedInfo = addCompletedInfo(label, isChecked, todoEle.id);
		}
		//
		var nextSib = label.nextSibling;
		while (nextSib) {
			//
			var tmpNext = nextSib;
			nextSib = nextSib.nextSibling;
			//
			if (isLabel(tmpNext) || isCompletedInfo(tmpNext))
				break;
			//
			label.appendChild(tmpNext);
		}
		var id = todoEle.id;
		//
		// changeWizDocument(id);
		modifiedTodos[todoEle.id] = {};
		modifiedTodos[todoEle.id]['state'] = isChecked ? "checked" : "unchecked";
		modifiedTodos[todoEle.id]['completedInfo'] = isChecked ? completedInfo : "";
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
		//
		registerEvent();
	}

	function changeWizDocument(id, html, state, completedInfo) {
		// var html = helper.getDocHtml();
		if (!html)
			return null;
		//
		var pos = html.indexOf(id);
		if (-1 == pos)
			return null;
		//
		var html1 = html.substr(0, pos);
		var pos1 = html1.lastIndexOf("<label");
		//
		var html2 = html.substr(pos);
		var pos2 = html2.indexOf("</label>");
		//
		if (-1 == pos1 || -1 == pos2)
			return null;
		//
		var html3 = html.substring(pos1, pos + pos2 + "</label>".length);
		//
		if (state == "checked" && -1 != html3.indexOf("wiz-todo-label-unchecked")) {
			html3 = html3.replace("wiz-todo-label-unchecked", "wiz-todo-label-checked");
			html3 = html3.replace("unchecked.png", "checked.png");
			html3 = html3.replace("state='unchecked'", "state='checked'");
			html3 = html3.replace("state=\"unchecked\"", "state=\"checked\"");
		}
		else if (state == "unchecked" && -1 != html3.indexOf("wiz-todo-label-checked")) {
			html3 = html3.replace("wiz-todo-label-checked", "wiz-todo-label-unchecked");
			html3 = html3.replace("checked.png", "unchecked.png");
			html3 = html3.replace("state='checked'", "state='unchecked'");
			html3 = html3.replace("state=\"checked\"", "state=\"unchecked\"");
		}
		else return null;
		//
		var firstHtml = html.substr(0, pos1);
		var lastHtml = html.substr(pos + pos2 + "</label>".length);
		html = firstHtml + html3 + lastHtml;
		//
		if (!helper.isPersonalDocument()) {
			if (state == "checked") {
				if (completedInfo) {
					html = firstHtml + html3 + completedInfo + lastHtml;
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
			newHtml = changeWizDocument(id, html, modifiedTodos[id].state, modifiedTodos[id].completedInfo);
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
			var userAvatar2 = userAvatar.replace(/\\/g, '\\\\');
			var reg = new RegExp(userAvatar2, 'ig');
			//
			html = html.replace(reg, "index_files/" + avatarName);
			//
			resources += "*" + userAvatar;
		}
        //
		helper.setDocHtml(html, resources);
		//
		modifiedTodos = {};
	}

	function onDocumentClose() {
		saveHtml();
	}
	
	function getWizDocument() {
		return helper.getWizDocument();
	}

	return {
		init: init,
		onDocumentClose: onDocumentClose,
		getWizDocument: getWizDocument
	}

})();
