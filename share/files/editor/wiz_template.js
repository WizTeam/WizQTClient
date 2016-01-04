(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
'use strict';

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonLang = require('./common/lang');

var _commonLang2 = _interopRequireDefault(_commonLang);

var _utilsUtilsJs = require('./utils/utils.js');

var _utilsUtilsJs2 = _interopRequireDefault(_utilsUtilsJs);

var _utilsInsertImgUtils = require('./utils/insertImgUtils');

var _utilsInsertImgUtils2 = _interopRequireDefault(_utilsInsertImgUtils);

var _templateBase = require('./template/base');

var _templateBase2 = _interopRequireDefault(_templateBase);

var WizTemplate = {
    /**
     * 初始化 模板组件
     * @param options
     * {
     *   document, //document
     *   clientType,  //客户端类型
     *   lang, //语言类型（en，zh-cn，zh-tw）
     * }
     */
    init: function init(options) {
        _commonEnv2['default'].setDoc(options.document || window.document);
        _commonEnv2['default'].client.setType(options.clientType);
        (0, _commonLang.initLang)(options.lang);
        return WizTemplate;
    },
    /**
     * 启动模板组件
     */
    on: function on(isRead) {
        if (_templateBase2['default'].getTemplate()) {
            _templateBase2['default'].on(isRead);
        }
        return WizTemplate;
    },
    /**
     * 关闭模板组件
     */
    off: function off() {
        _templateBase2['default'].off();
        return WizTemplate;
    },
    /**
     * 让 body 或 指定的 Dom 获取焦点
     */
    focus: function focus() {
        _templateBase2['default'].focus();
        return WizTemplate;
    },
    /**
     * 在笔记内隐藏模板 特殊的 Dom
     */
    hideTemplate: function hideTemplate() {
        _templateBase2['default'].hideElement();
        return WizTemplate;
    },
    /**
     * 从 html 字符串内隐藏模板 特殊的 Dom
     * @param html
     */
    hideTemplateFormHtml: function hideTemplateFormHtml(html) {
        return _templateBase2['default'].hideElementFromHtml(html);
    },
    img: {
        /**
         * 插入图片
         */
        insertByPath: function insertByPath(src, options) {
            _utilsInsertImgUtils2['default'].insertByPath(src, options);
        }
    },
    /**
     * 从笔记内 获取使用的模板信息
     */
    getTemplate: function getTemplate() {
        return _templateBase2['default'].getTemplate();
    },
    setContenteditable: function setContenteditable(enable) {
        _templateBase2['default'].setContenteditable(enable);
        return WizTemplate;
    },
    version: '0.1.0'
};

window.WizTemplate = WizTemplate;

},{"./common/const":2,"./common/env":3,"./common/lang":4,"./template/base":5,"./utils/insertImgUtils":7,"./utils/utils.js":10}],2:[function(require,module,exports){
/**
 * 内部使用的标准常量.
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});
var pre = 'wiz-template-';
var CONST = {
    Attr: {
        TemplateFormat: 'data-template-format',
        TemplateName: 'data-template-name',
        SelectBind: 'data-select-bind',
        SelectLayer: 'data-select-layer',
        TemplateVersion: 'data-template-version',
        OptionAction: 'data-option-action'
    },
    Class: {
        Active: 'active',
        Common: pre + 'node',
        Date: pre + 'date',
        Editable: pre + 'editable',
        FloatLayer: pre + 'floatLayer',
        Focus: pre + 'focus',
        Hide: pre + 'hide',
        InsertImg: pre + 'insert-img',
        Option: pre + 'option',
        Select: pre + 'select',
        SelectBtn: pre + 'select-btn',
        Diary: {
            blue: 'mood-blue',
            purple: 'mood-purple',
            green: 'mood-green',
            red: 'mood-red',
            sun: 'weather-sun',
            wind: 'weather-wind',
            rain: 'weather-rain',
            snow: 'weather-snow',
            mood: 'mood',
            weather: 'weather'
        },
        TodoImg: "wiz-todo-img"
    },
    Id: {
        TemplateInfo: pre + 'info',
        TemplateInsertImgPre: pre + 'insert-img'
    },
    EVENT: {
        InsertImg: 'wizTemplateInsertImg',
        setStyle: 'wizTemplateSetStyle'
    },
    DiaryStyle: {
        bgColor: {
            blue: "#e0f5fc",
            purple: "#eee6f5",
            green: "#e3f6e3",
            red: "#fae7ea"
        }
    }
};

exports['default'] = CONST;
module.exports = exports['default'];

},{}],3:[function(require,module,exports){
/**
 * 环境参数
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

var WizNotCmdInditify = 'wiznotecmd://';

var ENV = {
    win: null,
    doc: null,
    setDoc: function setDoc(doc) {
        if (doc) {
            ENV.doc = doc;
            ENV.win = ENV.doc.defaultView;
        }
    },
    /**
     * 客户端类型 & 功能设置
     */
    client: {
        type: {
            isWeb: (function () {
                return location && location.protocol.indexOf('http') === 0;
            })(),
            isWin: false,
            isMac: false,
            isIOS: false,
            isAndroid: false,
            isPad: false,
            isPhone: false
        },
        sendCmdToWiznote: function sendCmdToWiznote(cmd, options) {},
        setType: function setType(type) {
            if (!type) {
                return;
            }
            type = type.toLowerCase();
            if (type.indexOf('windows') > -1) {
                ENV.client.type.isWin = true;
            } else if (type.indexOf('ios') > -1) {
                ENV.client.type.isIOS = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    var url = '';
                    if (cmd == _const2['default'].EVENT.InsertImg) {
                        url = WizNotCmdInditify + cmd + '?userInfo=' + encodeURIComponent(options.userInfo) + '&message=' + encodeURIComponent(options.message) + '&cameraOnly=' + options.cameraOnly;
                    } else if (cmd == _const2['default'].EVENT.setStyle) {
                        url = WizNotCmdInditify + cmd + '?bgColor=' + encodeURIComponent(options.bgColor) + '&titleColor=' + encodeURIComponent(options.titleColor) + '&folderColor=' + encodeURIComponent(options.folderColor) + '&lineColor=' + encodeURIComponent(options.lineColor);
                    }

                    var iframe = ENV.doc.createElement("iframe");
                    iframe.setAttribute("src", url);
                    ENV.doc.documentElement.appendChild(iframe);
                    iframe.parentNode.removeChild(iframe);
                    iframe = null;
                };
            } else if (type.indexOf('android') > -1) {
                ENV.client.type.isAndroid = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    //if (cmd == CONST.EVENT.wizReaderClickImg) {
                    //    window.WizNote.onClickImg(options.src, options.imgList);
                    //}
                };
            } else if (type.indexOf('mac') > -1) {
                    ENV.client.type.isMac = true;
                }

            if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
                if (type.indexOf('pad') > -1) {
                    ENV.client.type.isPad = true;
                } else {
                    ENV.client.type.isPhone = true;
                }
            }
        }
    }
};

exports['default'] = ENV;
module.exports = exports['default'];

},{"./const":2}],4:[function(require,module,exports){
/**
 * Created by ZQG on 2015/3/11.
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});
var LANG = {},
    userLangType = 'en',
    userLang = {};
LANG['en'] = {
    weeks: ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat']
};
LANG['zh-cn'] = {
    weeks: ['星期日', '星期一', '星期二', '星期三', '星期四', '星期五', '星期六']
};
LANG['zh-tw'] = {
    weeks: ['星期日', '星期一', '星期二', '星期三', '星期四', '星期五', '星期六']
};

function setLang(type) {
    if (!type) {
        type = 'en';
    }
    type = type.toLowerCase();
    if (LANG[type]) {
        userLangType = type;
    } else {
        type = 'en';
    }

    var k;
    for (k in LANG[type]) {
        if (LANG[type].hasOwnProperty(k)) {
            userLang[k] = LANG[type][k];
        }
    }
}

exports['default'] = userLang;

/**
 * 初始化语言文件
 * @param lang
 */
var initLang = function initLang(type) {
    setLang(type);
};
exports.initLang = initLang;

},{}],5:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _utilsUtils = require('../utils/utils');

var _utilsUtils2 = _interopRequireDefault(_utilsUtils);

var _utilsInsertImgUtilsJs = require('../utils/insertImgUtils.js');

var _utilsInsertImgUtilsJs2 = _interopRequireDefault(_utilsInsertImgUtilsJs);

var _utilsDateUtilsJs = require('../utils/dateUtils.js');

var _utilsDateUtilsJs2 = _interopRequireDefault(_utilsDateUtilsJs);

var _utilsSelectUtilsJs = require('../utils/selectUtils.js');

var _utilsSelectUtilsJs2 = _interopRequireDefault(_utilsSelectUtilsJs);

var _utilsTodoPatchJs = require('../utils/todoPatch.js');

var _utilsTodoPatchJs2 = _interopRequireDefault(_utilsTodoPatchJs);

var template = {
    on: function on(isRead) {
        if (isRead) {
            //针对阅读状态设置特效
            return;
        }

        template.showElement();
        template.bind();
        _utilsInsertImgUtilsJs2['default'].on();
        _utilsDateUtilsJs2['default'].on();
        _utilsSelectUtilsJs2['default'].on();
        _utilsTodoPatchJs2['default'].on();
    },
    off: function off() {
        template.hideElement();
        template.unbind();
        _utilsInsertImgUtilsJs2['default'].off();
        _utilsDateUtilsJs2['default'].off();
        _utilsSelectUtilsJs2['default'].off();
        _utilsTodoPatchJs2['default'].off();
    },
    bind: function bind() {},
    unbind: function unbind() {},
    focus: function focus() {
        var n = _commonEnv2['default'].doc.querySelector('.' + _commonConst2['default'].Class.Focus);
        if (n) {
            n.focus();
        } else {
            _commonEnv2['default'].doc.body.focus();
        }
    },
    getTemplate: function getTemplate() {
        var meta = _commonEnv2['default'].doc.querySelector('#' + _commonConst2['default'].Id.TemplateInfo);
        if (!meta) {
            return null;
        }
        return JSON.stringify({
            name: meta.getAttribute(_commonConst2['default'].Attr.TemplateName),
            version: meta.getAttribute(_commonConst2['default'].Attr.TemplateVersion)
        });
    },
    hideElement: function hideElement() {
        var elements = _utilsUtils2['default'].getTemplateElements(),
            i,
            j;
        if (elements) {
            for (i = 0, j = elements.length; i < j; i++) {
                _utilsUtils2['default'].addClass(elements[i], _commonConst2['default'].Class.Hide);
            }
        }
    },
    hideElementFromHtml: function hideElementFromHtml(html) {
        //做 RegExp 做 test 的时候 不能使用 g 全局设置， 否则会影响 索引
        var regexForTest = /(<[^<> ]*[^<>]* class[ ]*=[ ]*['"])([^'"]*)(['"])/i;
        var regex = /(<[^<> ]*[^<>]* class[ ]*=[ ]*['"])([^'"]*)(['"])/ig;
        if (!regexForTest.test(html)) {
            return html;
        }

        var result = [],
            m,
            lastIndex = 0,
            str,
            reg;
        while (m = regex.exec(html)) {
            str = m[2];

            //先处理 float layer
            if ((' ' + str + ' ').indexOf(' ' + _commonConst2['default'].Class.FloatLayer + ' ') > -1) {
                reg = new RegExp(' ' + _commonConst2['default'].Class.Active + ' ', 'ig');
                str = _utilsUtils2['default'].trim((' ' + str + ' ').replace(reg, ''));
            } else if ((' ' + str + ' ').indexOf(' ' + _commonConst2['default'].Class.Common + ' ') === -1) {
                //其它情况 如果不是 template common 功能 dom 则不进行处理
                result.push(html.substr(lastIndex, m.index - lastIndex), m[0]);
                lastIndex = m.index + m[0].length;
                continue;
            } else if ((' ' + str + ' ').indexOf(' ' + _commonConst2['default'].Class.Hide + ' ') === -1) {
                //如果是 则添加 Hide ClassName
                str += ' ' + _commonConst2['default'].Class.Hide;
            }
            result.push(html.substr(lastIndex, m.index - lastIndex), m[1], str, m[3]);

            lastIndex = m.index + m[0].length;
            //console.log(m);
        }
        result.push(html.substr(lastIndex));
        return result.join('');
    },
    setContenteditable: function setContenteditable(enable) {
        var contents = _commonEnv2['default'].doc.querySelectorAll('.' + _commonConst2['default'].Class.Editable),
            i,
            j;
        if (contents.length === 0) {
            _commonEnv2['default'].doc.body.setAttribute('contenteditable', enable ? 'true' : 'false');
        } else {
            if (enable) {
                _commonEnv2['default'].doc.body.setAttribute('contenteditable', 'false');
            }
            for (i = 0, j = contents.length; i < j; i++) {
                contents[i].setAttribute('contenteditable', enable ? 'true' : 'false');
            }
        }
    },
    showElement: function showElement() {
        var elements = _utilsUtils2['default'].getTemplateElements(),
            i,
            j;
        if (elements) {
            for (i = 0, j = elements.length; i < j; i++) {
                _utilsUtils2['default'].removeClass(elements[i], _commonConst2['default'].Class.Hide);
            }
        }
    }
};

//var handler = {
//    onclick: function(event) {
//    }
//};

exports['default'] = template;
module.exports = exports['default'];

},{"../common/const":2,"../common/env":3,"../utils/dateUtils.js":6,"../utils/insertImgUtils.js":7,"../utils/selectUtils.js":8,"../utils/todoPatch.js":9,"../utils/utils":10}],6:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonLang = require('../common/lang');

var _commonLang2 = _interopRequireDefault(_commonLang);

var _utils = require('./utils');

var _utils2 = _interopRequireDefault(_utils);

var dateUtils = {
    on: function on() {
        var dateList = _commonEnv2['default'].doc.querySelectorAll('.' + _commonConst2['default'].Class.Date),
            i,
            j,
            d;
        for (i = 0, j = dateList.length; i < j; i++) {
            d = dateList[i];
            d.innerHTML = getDate(d.getAttribute(_commonConst2['default'].Attr.TemplateFormat));
            d.removeAttribute(_commonConst2['default'].Attr.TemplateFormat);
            _utils2['default'].removeClass(d, _commonConst2['default'].Class.Date);
        }
    },
    off: function off() {}
};

function getDate(format) {
    if (!format) {
        return '';
    }

    var dd = new Date(),
        y = patch(dd.getFullYear(), 4),
        m = patch(dd.getMonth() + 1, 2),
        d = patch(dd.getDate(), 2),
        w = _commonLang2['default'].weeks[dd.getDay()];

    return format.replace(/yyyy/ig, y).replace(/mm/ig, m).replace(/dd/ig, d).replace(/ww/ig, w);
}
function patch(str, length) {
    str = str + '';
    while (str.length < length) {
        str = '0' + str;
    }
    return str;
}
exports['default'] = dateUtils;
module.exports = exports['default'];

},{"../common/const":2,"../common/env":3,"../common/lang":4,"./utils":10}],7:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnvJs = require('../common/env.js');

var _commonEnvJs2 = _interopRequireDefault(_commonEnvJs);

var _commonConstJs = require('../common/const.js');

var _commonConstJs2 = _interopRequireDefault(_commonConstJs);

var _utilsJs = require('./utils.js');

var _utilsJs2 = _interopRequireDefault(_utilsJs);

var insertImgUtils = {
    on: function on() {
        //测试 插入图片按钮
        //var x = ENV.doc.createElement('button');
        //utils.addClass(x, CONST.Class.InsertImg);
        //x.innerHTML = 'test insert image';
        //x.setAttribute('contenteditable', 'false');
        //ENV.doc.body.insertBefore(x, null);
        insertImgUtils.off();
        _utilsJs2['default'].bind(_commonEnvJs2['default'].doc.body, "click", handler);
    },
    off: function off() {
        _utilsJs2['default'].unbind(_commonEnvJs2['default'].doc.body, "click", handler);
    },
    insertByPath: function insertByPath(src, options) {
        if (!options) {
            options = {};
        } else if (typeof options == 'string') {
            options = decodeURIComponent(options);
            options = JSON.parse(options);
        }
        var target = _commonEnvJs2['default'].doc.getElementById(options.id);
        var parent = !target || target == _commonEnvJs2['default'].doc.body ? _commonEnvJs2['default'].doc.body : target.parentNode;
        insertImage(makeDom(src), parent, target || null);
    }
};

function filterFn(node) {
    return _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.InsertImg);
}

function handler(event) {
    var src = event.target || event.srcElement;

    var node = _utilsJs2['default'].getParentByFilter(src, filterFn),
        userInfo;

    if (node) {
        if (!src.id) {
            src.id = _commonConstJs2['default'].Id.TemplateInsertImgPre + new Date().valueOf();
        }

        userInfo = {
            id: src.id
        };

        _commonEnvJs2['default'].client.sendCmdToWiznote(_commonConstJs2['default'].EVENT.InsertImg, {
            userInfo: JSON.stringify(userInfo),
            message: '模板插入图片',
            cameraOnly: 'true'
        });
    }
}

function insertImage(image, parent, target) {
    if (!image) {
        return;
    }
    var i, j;

    if (_utilsJs2['default'].isArray(image)) {
        for (i = 0, j = image.length; i < j; i++) {
            parent.insertBefore(image[i], target);
        }
    } else {
        parent.insertBefore(image, target);
    }
}

function makeDom(src) {
    var result = [],
        paths = [],
        main,
        img,
        i,
        j;
    if (src.indexOf('*')) {
        paths = src.split("*");
    } else {
        paths.push(src);
    }

    for (i = 0, j = paths.length; i < j; i++) {
        main = _commonEnvJs2['default'].doc.createElement("div");
        result.push(main);

        img = _commonEnvJs2['default'].doc.createElement("img");
        img.src = paths[i];
        img.style.maxWidth = '100%';
        main.insertBefore(img, null);
    }

    main = _commonEnvJs2['default'].doc.createElement("div");
    main.insertBefore(_commonEnvJs2['default'].doc.createElement("br"), null);
    result.push(main);
    return result;
}

exports['default'] = insertImgUtils;
module.exports = exports['default'];

},{"../common/const.js":2,"../common/env.js":3,"./utils.js":10}],8:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnvJs = require('../common/env.js');

var _commonEnvJs2 = _interopRequireDefault(_commonEnvJs);

var _commonConstJs = require('../common/const.js');

var _commonConstJs2 = _interopRequireDefault(_commonConstJs);

var _utilsJs = require('./utils.js');

var _utilsJs2 = _interopRequireDefault(_utilsJs);

var curRange = null;
var selectUtils = {
    on: function on() {
        selectUtils.off();
        _utilsJs2['default'].bind(_commonEnvJs2['default'].doc.body, _commonEnvJs2['default'].client.type.isPhone ? "touchstart" : "mousedown", handlerGetRange);
        _utilsJs2['default'].bind(_commonEnvJs2['default'].doc.body, "click", handler);
    },
    off: function off() {
        _utilsJs2['default'].unbind(_commonEnvJs2['default'].doc.body, _commonEnvJs2['default'].client.type.isPhone ? "touchstart" : "mousedown", handlerGetRange);
        _utilsJs2['default'].unbind(_commonEnvJs2['default'].doc.body, "click", handler);
    }
};

function filterFn(node) {
    return _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.SelectBtn) || _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.Option);
}
function filterSelectFn(node) {
    return _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.Select);
}
function filterLayerFn(node) {
    return _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.FloatLayer);
}

function handlerGetRange(event) {
    var src = event.target || event.srcElement;
    var node = _utilsJs2['default'].getParentByFilter(src, filterFn),
        select,
        layerId,
        layer;

    if (node && _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.SelectBtn)) {
        layerId = node.getAttribute(_commonConstJs2['default'].Attr.SelectLayer);
        if (!layerId) {
            return;
        }

        layer = _commonEnvJs2['default'].doc.getElementById(layerId);
        if (!layer) {
            return;
        }

        if (!_utilsJs2['default'].hasClass(layer, _commonConstJs2['default'].Class.Active)) {
            curRange = _utilsJs2['default'].getTemplateRange();
        }
    }
}

function handler(event) {
    var src = event.target || event.srcElement;

    var node = _utilsJs2['default'].getParentByFilter(src, filterFn),
        select,
        layerId,
        layer;

    if (node && _utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.SelectBtn)) {
        //选择 按钮
        layerId = node.getAttribute(_commonConstJs2['default'].Attr.SelectLayer);
        if (!layerId) {
            return;
        }

        layer = _commonEnvJs2['default'].doc.getElementById(layerId);
        if (!layer) {
            return;
        }

        if (_utilsJs2['default'].hasClass(layer, _commonConstJs2['default'].Class.Active)) {
            _utilsJs2['default'].removeClass(layer, _commonConstJs2['default'].Class.Active);
            _utilsJs2['default'].removeClass(_commonEnvJs2['default'].doc.querySelector('.layer-mask'), _commonConstJs2['default'].Class.Active);
            _utilsJs2['default'].setTemplateRange(curRange);
            unbindClose();
        } else {
            _utilsJs2['default'].addClass(layer, _commonConstJs2['default'].Class.Active);
            _utilsJs2['default'].addClass(_commonEnvJs2['default'].doc.querySelector('.layer-mask'), _commonConstJs2['default'].Class.Active);
            bindClose();
        }
    } else if (node) {
        //选择 项目

        select = _utilsJs2['default'].getParentByFilter(src, filterSelectFn);
        if (!select) {
            return;
        }

        if (_utilsJs2['default'].hasClass(node, _commonConstJs2['default'].Class.Active)) {
            return;
        }

        if (_utilsJs2['default'].hasClass(node.parentNode, _commonConstJs2['default'].Class.Diary.mood)) {
            _commonEnvJs2['default'].doc.body.className = node.querySelector("i").className;
            clickMood(node.querySelector("i").className);
        } else if (_utilsJs2['default'].hasClass(node.parentNode, _commonConstJs2['default'].Class.Diary.weather)) {
            _commonEnvJs2['default'].doc.querySelector("i." + _commonConstJs2['default'].Class.Diary.weather).className = _commonConstJs2['default'].Class.Diary.weather + " " + node.querySelector("i").className;
            clickWeather(node.querySelector("i").className);
        }
    }
}

function closeLayer(event) {
    var src = event.target || event.srcElement;
    var node = _utilsJs2['default'].getParentByFilter(src, filterFn);
    if (!node) {
        node = _utilsJs2['default'].getParentByFilter(src, filterLayerFn);
    }

    var layers, i, j;
    if (!node) {
        layers = _commonEnvJs2['default'].doc.querySelectorAll('.' + _commonConstJs2['default'].Class.FloatLayer + '.' + _commonConstJs2['default'].Class.Active);
        for (i = 0, j = layers.length; i < j; i++) {
            _utilsJs2['default'].removeClass(layers[i], _commonConstJs2['default'].Class.Active);
        }
        _utilsJs2['default'].removeClass(_commonEnvJs2['default'].doc.querySelector('.layer-mask'), _commonConstJs2['default'].Class.Active);
        unbindClose();
    }
}
function bindClose() {
    if (_commonEnvJs2['default'].client.type.isPhone) {
        _utilsJs2['default'].bind(_commonEnvJs2['default'].doc.body, "touchstart", closeLayer);
    } else {
        _utilsJs2['default'].bind(_commonEnvJs2['default'].doc.body, "click", closeLayer);
    }
}
function unbindClose() {
    if (_commonEnvJs2['default'].client.type.isPhone) {
        _utilsJs2['default'].unbind(_commonEnvJs2['default'].doc.body, "touchstart", closeLayer);
    } else {
        _utilsJs2['default'].unbind(_commonEnvJs2['default'].doc.body, "click", closeLayer);
    }
}
function clickWeather(className) {
    var weather, arr;
    arr = className.split("-");
    if (arr.length && arr.length > 0) {
        weather = arr[arr.length - 1];
        _commonEnvJs2['default'].doc.getElementById(_commonConstJs2['default'].Id.TemplateInfo).setAttribute("data-template-weather", weather);
    }
}
function clickMood(className) {
    var colors = _commonConstJs2['default'].DiaryStyle.bgColor,
        mood,
        bgColor,
        arr;
    arr = className.split("-");
    if (arr.length && arr.length > 0) {
        mood = arr[arr.length - 1];
        for (var key in colors) {
            if (mood == key) {
                bgColor = colors[key];
                _commonEnvJs2['default'].doc.getElementById(_commonConstJs2['default'].Id.TemplateInfo).setAttribute("data-template-bgcolor", bgColor);
                break;
            }
        }
        _commonEnvJs2['default'].client.sendCmdToWiznote(_commonConstJs2['default'].EVENT.setStyle, {
            bgColor: bgColor,
            titleColor: "",
            folderColor: "",
            lineColor: ""
        });
    }
}
exports['default'] = selectUtils;
module.exports = exports['default'];

},{"../common/const.js":2,"../common/env.js":3,"./utils.js":10}],9:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonConstJs = require('../common/const.js');

var _commonConstJs2 = _interopRequireDefault(_commonConstJs);

var _commonEnvJs = require('../common/env.js');

var _commonEnvJs2 = _interopRequireDefault(_commonEnvJs);

var todoList = {
    on: function on() {
        var todoImgs = _commonEnvJs2['default'].doc.querySelectorAll("." + _commonConstJs2['default'].Class.TodoImg),
            arr,
            random;
        if (!todoImgs) {
            return;
        }
        for (var i = 0, j = todoImgs.length; i < j; i++) {
            arr = ['wiz_todo'];
            random = [];
            for (var a = 0; a < 6; a++) {
                random.push(Math.floor(Math.random() * 10));
            }
            arr.push(new Date().getTime() + i);
            arr.push(random.join(""));
            arr.push(_commonEnvJs2['default'].doc.getElementById(_commonConstJs2['default'].Id.TemplateInfo).getAttribute("data-templatename"));
            //console.log(arr.join("_"));
            if (!todoImgs[i].id) {
                todoImgs[i].id = arr.join("_");
            }
        }
    },
    off: function off() {}
};
exports['default'] = todoList;
module.exports = exports['default'];

},{"../common/const.js":2,"../common/env.js":3}],10:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonConstJs = require('../common/const.js');

var _commonConstJs2 = _interopRequireDefault(_commonConstJs);

var _commonEnvJs = require('../common/env.js');

var _commonEnvJs2 = _interopRequireDefault(_commonEnvJs);

var utils = {
    //绑定事件方法
    bind: function bind(el, type, fn) {
        if (el.addEventListener) {
            el.addEventListener(type, fn, false);
        } else {
            el['e' + fn] = function () {
                fn.call(el, window.event);
            };
            el.attachEvent('on' + type, el['e' + fn]);
        }
    },
    //解绑事件方法
    unbind: function unbind(el, type, fn) {
        if (el.removeEventListener) {
            el.removeEventListener(type, fn, false);
        } else if (el.detachEvent) {
            el.detachEvent('on' + type, el['e' + fn]);
        }
    },
    getTemplateElements: function getTemplateElements() {
        return _commonEnvJs2['default'].doc.querySelectorAll('.' + _commonConstJs2['default'].Class.Common);
    },
    hasClass: function hasClass(obj, className) {
        if (obj) {
            return (' ' + obj.className + ' ').indexOf(' ' + className + ' ') > -1;
        }
    },
    addClass: function addClass(obj, className) {
        if (utils.hasClass(obj, className)) {
            return;
        }
        obj.className = utils.trim(obj.className + ' ' + className);
    },
    removeClass: function removeClass(obj, className) {
        obj.className = utils.trim((" " + obj.className + " ").replace(' ' + className + ' ', ' '));
    },
    isArray: function isArray(obj) {
        return Object.prototype.toString.apply(obj) === "[object Array]";
    },
    /**
     * 阻止默认事件
     * @param e
     */
    stopEvent: function stopEvent(e) {
        e.stopPropagation();
        e.preventDefault();
    },
    trim: function trim(str) {
        return str.replace(/^\s+|\s+$/g, '');
    },
    //根据 filterFn 函数设置的 自定义规则 查找 Dom 的父节点
    getParentByFilter: function getParentByFilter(node, filterFn) {
        if (!filterFn) {
            return node;
        }
        if (node && !(node == _commonEnvJs2['default'].doc.body)) {
            while (node) {
                if (filterFn(node)) {
                    return node;
                }
                if (node == _commonEnvJs2['default'].doc.body) {
                    return null;
                }
                node = node.parentNode;
            }
        }
        return null;
    },
    getTemplateRange: function getTemplateRange() {
        var s = document.getSelection();
        if (!s.rangeCount) {
            return null;
        }

        var r = s.getRangeAt(0);
        return {
            start: r.startContainer,
            startOffset: r.startOffset,
            end: r.endContainer,
            endOffset: r.endOffset
        };
    },
    setTemplateRange: function setTemplateRange(range) {
        if (!range) {
            return;
        }
        var s = document.getSelection();
        s.collapse(range.start, range.startOffset);
        s.extend(range.end, range.endOffset);
    }
};
exports['default'] = utils;
module.exports = exports['default'];

},{"../common/const.js":2,"../common/env.js":3}]},{},[1]);
