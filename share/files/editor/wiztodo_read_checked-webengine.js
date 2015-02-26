// Copyright (c) 2014, Dan Kogai
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.

// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.

// * Neither the name of {{{project}}} nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
 * $Id: base64.js,v 2.15 2014/04/05 12:58:57 dankogai Exp dankogai $
 *
 *  Licensed under the MIT license.
 *    http://opensource.org/licenses/mit-license
 *
 *  References:
 *    http://en.wikipedia.org/wiki/Base64
 */

(function(global) {
    'use strict';
    // existing version for noConflict()
    var _Base64 = global.Base64;
    var version = "2.1.5";
    // if node.js, we use Buffer
    var buffer;
    if (typeof module !== 'undefined' && module.exports) {
        buffer = require('buffer').Buffer;
    }
    // constants
    var b64chars
        = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
    var b64tab = function(bin) {
        var t = {};
        for (var i = 0, l = bin.length; i < l; i++) t[bin.charAt(i)] = i;
        return t;
    }(b64chars);
    var fromCharCode = String.fromCharCode;
    // encoder stuff
    var cb_utob = function(c) {
        if (c.length < 2) {
            var cc = c.charCodeAt(0);
            return cc < 0x80 ? c
                : cc < 0x800 ? (fromCharCode(0xc0 | (cc >>> 6))
                                + fromCharCode(0x80 | (cc & 0x3f)))
                : (fromCharCode(0xe0 | ((cc >>> 12) & 0x0f))
                   + fromCharCode(0x80 | ((cc >>>  6) & 0x3f))
                   + fromCharCode(0x80 | ( cc         & 0x3f)));
        } else {
            var cc = 0x10000
                + (c.charCodeAt(0) - 0xD800) * 0x400
                + (c.charCodeAt(1) - 0xDC00);
            return (fromCharCode(0xf0 | ((cc >>> 18) & 0x07))
                    + fromCharCode(0x80 | ((cc >>> 12) & 0x3f))
                    + fromCharCode(0x80 | ((cc >>>  6) & 0x3f))
                    + fromCharCode(0x80 | ( cc         & 0x3f)));
        }
    };
    var re_utob = /[\uD800-\uDBFF][\uDC00-\uDFFFF]|[^\x00-\x7F]/g;
    var utob = function(u) {
        return u.replace(re_utob, cb_utob);
    };
    var cb_encode = function(ccc) {
        var padlen = [0, 2, 1][ccc.length % 3],
        ord = ccc.charCodeAt(0) << 16
            | ((ccc.length > 1 ? ccc.charCodeAt(1) : 0) << 8)
            | ((ccc.length > 2 ? ccc.charCodeAt(2) : 0)),
        chars = [
            b64chars.charAt( ord >>> 18),
            b64chars.charAt((ord >>> 12) & 63),
            padlen >= 2 ? '=' : b64chars.charAt((ord >>> 6) & 63),
            padlen >= 1 ? '=' : b64chars.charAt(ord & 63)
        ];
        return chars.join('');
    };
    var btoa = global.btoa ? function(b) {
        return global.btoa(b);
    } : function(b) {
        return b.replace(/[\s\S]{1,3}/g, cb_encode);
    };
    var _encode = buffer
        ? function (u) { return (new buffer(u)).toString('base64') } 
    : function (u) { return btoa(utob(u)) }
    ;
    var encode = function(u, urisafe) {
        return !urisafe 
            ? _encode(u)
            : _encode(u).replace(/[+\/]/g, function(m0) {
                return m0 == '+' ? '-' : '_';
            }).replace(/=/g, '');
    };
    var encodeURI = function(u) { return encode(u, true) };
    // decoder stuff
    var re_btou = new RegExp([
        '[\xC0-\xDF][\x80-\xBF]',
        '[\xE0-\xEF][\x80-\xBF]{2}',
        '[\xF0-\xF7][\x80-\xBF]{3}'
    ].join('|'), 'g');
    var cb_btou = function(cccc) {
        switch(cccc.length) {
        case 4:
            var cp = ((0x07 & cccc.charCodeAt(0)) << 18)
                |    ((0x3f & cccc.charCodeAt(1)) << 12)
                |    ((0x3f & cccc.charCodeAt(2)) <<  6)
                |     (0x3f & cccc.charCodeAt(3)),
            offset = cp - 0x10000;
            return (fromCharCode((offset  >>> 10) + 0xD800)
                    + fromCharCode((offset & 0x3FF) + 0xDC00));
        case 3:
            return fromCharCode(
                ((0x0f & cccc.charCodeAt(0)) << 12)
                    | ((0x3f & cccc.charCodeAt(1)) << 6)
                    |  (0x3f & cccc.charCodeAt(2))
            );
        default:
            return  fromCharCode(
                ((0x1f & cccc.charCodeAt(0)) << 6)
                    |  (0x3f & cccc.charCodeAt(1))
            );
        }
    };
    var btou = function(b) {
        return b.replace(re_btou, cb_btou);
    };
    var cb_decode = function(cccc) {
        var len = cccc.length,
        padlen = len % 4,
        n = (len > 0 ? b64tab[cccc.charAt(0)] << 18 : 0)
            | (len > 1 ? b64tab[cccc.charAt(1)] << 12 : 0)
            | (len > 2 ? b64tab[cccc.charAt(2)] <<  6 : 0)
            | (len > 3 ? b64tab[cccc.charAt(3)]       : 0),
        chars = [
            fromCharCode( n >>> 16),
            fromCharCode((n >>>  8) & 0xff),
            fromCharCode( n         & 0xff)
        ];
        chars.length -= [0, 0, 2, 1][padlen];
        return chars.join('');
    };
    var atob = global.atob ? function(a) {
        return global.atob(a);
    } : function(a){
        return a.replace(/[\s\S]{1,4}/g, cb_decode);
    };
    var _decode = buffer
        ? function(a) { return (new buffer(a, 'base64')).toString() }
    : function(a) { return btou(atob(a)) };
    var decode = function(a){
        return _decode(
            a.replace(/[-_]/g, function(m0) { return m0 == '-' ? '+' : '/' })
                .replace(/[^A-Za-z0-9\+\/]/g, '')
        );
    };
    var noConflict = function() {
        var Base64 = global.Base64;
        global.Base64 = _Base64;
        return Base64;
    };
    // export Base64
    global.Base64 = {
        VERSION: version,
        atob: atob,
        btoa: btoa,
        fromBase64: decode,
        toBase64: encode,
        utob: utob,
        encode: encode,
        encodeURI: encodeURI,
        btou: btou,
        decode: decode,
        noConflict: noConflict
    };
    // if ES5 is available, make Base64.extendString() available
    if (typeof Object.defineProperty === 'function') {
        var noEnum = function(v){
            return {value:v,enumerable:false,writable:true,configurable:true};
        };
        global.Base64.extendString = function () {
            Object.defineProperty(
                String.prototype, 'fromBase64', noEnum(function () {
                    return decode(this)
                }));
            Object.defineProperty(
                String.prototype, 'toBase64', noEnum(function (urisafe) {
                    return encode(this, urisafe)
                }));
            Object.defineProperty(
                String.prototype, 'toBase64URI', noEnum(function () {
                    return encode(this, true)
                }));
        };
    }
    // that's it!
})(this);

if (this['Meteor']) {
    Base64 = global.Base64; // for normal export in Meteor.js
}
////////////////////////////////////////////////////////////////////////////////

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
	this.onClickingTodo = onClickingTodo;
    this.onBeforeSave = onBeforeSave;
	
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

	function onClickingTodo(callback) {
        this.app.ExecuteCommand("OnClickingChecklist",
         "WizTodoReadChecked." + callback + "({cancel}, {needCallAgain});", "readingnote");
	}

    function onBeforeSave(isModified) {
        // this.app.ExecuteCommand("onBeforeSaveChecklist", isModified, "readingnote");
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
    this.onClickingTodo = onClickingTodo;
    this.setUserAlias = setUserAlias;
    this.setUserAvatarFileName = setUserAvatarFileName;
    this.setCheckedImageFileName = setCheckedImageFileName;
    this.setUnCheckedImageFileName = setUnCheckedImageFileName;
    this.setIsPersonalDocument = setIsPersonalDocument;
    this.setCanEdit = setCanEdit;
    this.setDocOriginalHtml = setDocOriginalHtml;


    this.userAlias = null;
    this.avatarFileName = null;
    this.checkedFileName = null;
    this.unCheckedFileName = null;
    this.personalDocument = false;
    this.canedit = false;
    this.originalHtml = "";

    // WizEditor.setDocOriginalHtml.connect(function (strHtml) {
    //     this.originalHtml = strHtml;
    //     console.log("after set original html , html : " + getDocHtml());
    // });
    
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

    function setDocOriginalHtml (strHtml) {
        console.log("set doc originalHtml called , html : " + strHtml);
        this.originalHtml = strHtml;
    }

    function getDocHtml() {
        return this.originalHtml;
    }

    function setDocHtml(html, resources) {
        WizEditor.saveHtmlToCurrentNote(html, resources);
    }

    function canEdit() {
        return this.canedit;
    }

    function getCheckedImageFileName() {
        return this.checkedFileName;
    }

    function getUnCheckedImageFileName() {
        return this.unCheckedFileName;
    }

    function isPersonalDocument() {
        return this.personalDocument;
    }

    function getLocalDateTime(dt) {
        return "";
    }

    function getUserAlias() {
        return this.userAlias;
    }

    function getUserAvatarFileName(size) {
        return this.avatarFileName;
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

    function onClickingTodo(callback) {
        WizEditor.clickingTodoCallBack.connect(WizTodoReadChecked[callback]);
        return WizEditor.checkListClickable();
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
	this.onClickingTodo = onClickingTodo;
    this.onBeforeSave = onBeforeSave;

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

	function onDocumentClose(isModified) {
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

    function onClickingTodo(callback) {
        window.WizNote.onClickingTodo(callback);
    }

    function onBeforeSave(modified) {

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
	this.onClickingTodo = onClickingTodo;
    this.onBeforeSave = onBeforeSave;

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
		window.location.href="wiztodolist://setDocHtml/"+"?html=" + html +"&resource="+ resources;
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
        var href = "wiztodolist://onAddTodoCompletedInfo/" + "?checked="+ isChecked +"&id="+ id +"&dt="+ dt +"&callback="+ callBack;
        window.location.href = href;
	}

	function onClickingTodo(callback) {
		window.location.href="wiztodolist://tryLockDocument/"+"?callback=" + callback;
	}

    function onBeforeSave(isModified) {
        
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
	var needCallHelperClicking = true;
	var beingClickedTodoEle = null;
    var modified = false;
    var bodyOriginalCursor = undefined;

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

    function isQt () {
        return 'qt' == wizClient;
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

	function clickTodo(todoEle) {

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
				helper.onAddTodoCompletedInfo(isChecked, todoEle.id, Date.now(), 'addTodoCompletedInfo');
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
		if (modifiedTodos[todoEle.id] === undefined) {
			modifiedTodos[todoEle.id] = {};
		}
        //
        if (modifiedTodos[todoEle.id]['state'] === undefined) {
            modifiedTodos[todoEle.id]['state'] = isChecked ? "checked" : "unchecked";    
        }
        else {
            delete modifiedTodos[todoEle.id];
        }
		//
	}

	function onTodoClick(todoEle) {

        if (beingClickedTodoEle != null)
            return;
        //
		if (!needCallHelperClicking) {
			clickTodo(todoEle);
		}
		else {
			if (helper.onClickingTodo) {
				beingClickedTodoEle = todoEle;
				helper.onClickingTodo("onClickingTodoCallback");
			}
		}
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

    function unregisterEvent() {
        if (editorDocument) {
            editorDocument.removeEventListener('click', onDocumentClick);
        }
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

    function clear() {

        unregisterEvent();
        //
        helper = null;
        wizClient = null;
        modifiedTodos = {};
        editorDocument = null;
        needCallHelperClicking = true;
        beingClickedTodoEle = null;
        modified = false;
        bodyOriginalCursor = undefined;
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
	        if(obj[i] !== undefined) {
                count ++;
            }
	     }
	     return count;		
	}

    function _isModified() {
        return getObjectLength(modifiedTodos) >= 1;
    }

	function saveHtml() {
		
		if (!_isModified())
			return;
        //
        modified = true;
		//
		var html = helper.getDocHtml();
        console.log("get save html , original html : " + html);
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
        if (!isIpad() && !isIphone() && !isQt()) {
			helper.setDocHtml(html, resources);
		}
		else {
			return html;	
		}
	}

	function onDocumentClose() {
        var modified = _isModified();
        //
        if (helper.onBeforeSave) {
            helper.onBeforeSave(modified);
        }
        var html = saveHtml();
        //
        if (helper.onDocumentClose) {
            helper.onDocumentClose();
        }
		//
		if (isIpad() || isIphone() || isQt()) {
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
		
			html = Base64.decode(html);
			//
			helper.setDocOriginalHtml(html);
		}
	}

	function addTodoCompletedInfo(isChecked, id, localDateTime) {
		var todoImg = editorDocument.getElementById(id);
		var label = getParentTodoLabelElement(todoImg);
		//
		addCompletedInfo(label, isChecked === 'true', id, localDateTime);
	}
  
	function onClickingTodoCallback(cancel, needCallAgain) {
		needCallHelperClicking = needCallAgain;
		if (cancel) {
            beingClickedTodoEle = null;
        }
        else {
			clickTodo(beingClickedTodoEle);
            beingClickedTodoEle = null;
		}
	}

    function isModified() {
        return modified || _isModified();
    }

    function _setChecklistCursor(cursor) {
        var labels = editorDocument.getElementsByClassName('wiz-todo-label');
        for (var i = 0; i < labels.length; i++) {
            var label  = labels[i];
            label.style.cursor = cursor;
            //
            var imgs = label.getElementsByClassName('wiz-todo-img');
            for (var j = 0; j < imgs.length; j++) {
                imgs[j].style.cursor = cursor;
            }
        }
    }

    function setChecklistCursorWait() {
        _setChecklistCursor("wait");
        //
        bodyOriginalCursor = editorDocument.body.style.cursor;
        editorDocument.body.style.cursor = "wait";
    }

    function restoreChecklistCursor() {
        if (undefined === bodyOriginalCursor) {
            console.log("restoreChecklistCursor, undefined == bodyOriginalCursor.");
            return;
        }
        //
        _setChecklistCursor("");
        //
        editorDocument.body.style.cursor = bodyOriginalCursor;
    }

	return {
		init: init,
        clear: clear,
		onDocumentClose: onDocumentClose,
		getWizDocument: getWizDocument,
		setCanEdit: setCanEdit,
		setUserAlias: setUserAlias,
		setUserAvatarFileName: setUserAvatarFileName,
		setCheckedImageFileName: setCheckedImageFileName,
		setUnCheckedImageFileName: setUnCheckedImageFileName,
		setIsPersonalDocument: setIsPersonalDocument,
		setDocOriginalHtml: setDocOriginalHtml,
		addTodoCompletedInfo: addTodoCompletedInfo,
		onClickingTodoCallback: onClickingTodoCallback,
        isModified: isModified,
        setChecklistCursorWait: setChecklistCursorWait,
        restoreChecklistCursor: restoreChecklistCursor
	}

})();
