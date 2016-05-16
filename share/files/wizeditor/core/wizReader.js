(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
/**
 * 修订信息显示图层 相关对象
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _commonLang = require('../common/lang');

var _commonLang2 = _interopRequireDefault(_commonLang);

var _domUtilsDomBase = require('../domUtils/domBase');

var _domUtilsDomBase2 = _interopRequireDefault(_domUtilsDomBase);

var _amendUtilsAmendBase = require('./amendUtils/amendBase');

var _amendUtilsAmendBase2 = _interopRequireDefault(_amendUtilsAmendBase);

var _amendUser = require('./amendUser');

var _amendUser2 = _interopRequireDefault(_amendUser);

var _commonWizUserAction = require('../common/wizUserAction');

var _commonWizUserAction2 = _interopRequireDefault(_commonWizUserAction);

var callback = {
    onAccept: null,
    onRefuse: null
};
//暂停显示的标志
var pause = false,

//记录最后一次鼠标移动的位置
lastMousePos = { x: null, y: null };

var amendInfo = {
    cur: null,
    curPos: null,
    isMulti: false,
    isSelection: false,
    template: null,
    main: null,
    img: null,
    name: null,
    content: null,
    time: null,
    btnAccept: null,
    btnRefuse: null,

    /**
     * 修订信息 显示图层 初始化
     * @param options  {readonly: boolean,  cb: {onAccept: function, onRefuse: function}}
     * @param cb
     */
    init: function init(options, cb) {
        amendInfo.template = _commonEnv2['default'].doc.createElement('div');
        amendInfo.main = createAmendInfo();
        amendInfo.readonly = !!(options && options.readonly);

        _domUtilsDomBase2['default'].setContenteditable(amendInfo.main, false);

        if (cb && cb.onAccept) {
            callback.onAccept = cb.onAccept;
        }
        if (cb && cb.onRefuse) {
            callback.onRefuse = cb.onRefuse;
        }
        _event.unbind();
        _event.bind();
    },
    /**
     * 删除 修订信息 图层
     */
    remove: function remove() {
        _event.unbind();
        removeAmendInfo();
        amendInfo.main = null;
        amendInfo.img = null;
        amendInfo.name = null;
        amendInfo.content = null;
        amendInfo.time = null;
        amendInfo.btnAccept = null;
        amendInfo.btnRefuse = null;
    },
    /**
     * 显示 修订信息
     * @param dom
     * @param pos
     */
    show: function show(dom, pos) {
        clearTimeout(amendInfo.showTimer);
        clearTimeout(amendInfo.hideTimer);

        var isSelection = _commonUtils2['default'].isArray(dom),
            isMulti = isSelection && dom.length > 1,
            cur = !isSelection ? dom : isMulti ? null : dom[0],
            showFlag = false;

        amendInfo.isSelection = isSelection;
        if (amendInfo.isMulti !== isMulti || cur !== amendInfo.cur) {

            //移动到不同的 dom 时，立刻隐藏当前标签， 等待固定时间后再显示信息
            amendInfo.hide(true);

            showFlag = true;
        } else if (!amendInfo.curPos || Math.abs(amendInfo.curPos.left - pos.left) > 75 || Math.abs(amendInfo.curPos.top - pos.top) > 24) {
            //在同一个 dom 内移动距离较远后， 更换信息图层位置
            showFlag = true;
        }

        if (showFlag) {
            amendInfo.showTimer = setTimeout(function () {
                amendInfo.isMulti = isMulti;
                amendInfo.cur = cur;
                showInfo(pos);
            }, _commonConst2['default'].AMEND.INFO_TIMER * 2);
        }
    },
    /**
     * 隐藏 修订信息
     * @param quick
     */
    hide: function hide(quick) {
        clearTimeout(amendInfo.showTimer);
        clearTimeout(amendInfo.hideTimer);
        if (!amendInfo.cur && !amendInfo.isMulti) {
            return;
        }

        if (quick) {
            hideInfo();
        } else {
            amendInfo.hideTimer = setTimeout(hideInfo, _commonConst2['default'].AMEND.INFO_TIMER);
        }
    },
    /**
     * 判断 dom 是否 amendInfo layer 内的元素（包括layer）
     * @param dom
     */
    isInfo: function isInfo(dom) {
        var amendInfoMain = _domUtilsDomBase2['default'].getParentByFilter(dom, function (node) {
            return node == amendInfo.main;
        }, true);
        return !!amendInfoMain;
    },
    /**
     * 恢复 info 的显示
     */
    start: function start() {
        pause = false;
    },
    /**
     * 暂停 info 的显示
     */
    stop: function stop() {
        amendInfo.hide(true);
        pause = true;
    }
};

var _event = {
    bind: function bind() {
        if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isAndroid) {
            _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_TOUCH_START, _event.handler.onTouchstart);
            _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_TOUCH_END, _event.handler.onMouseMove);
        } else {
            _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        }
    },
    unbind: function unbind() {
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_TOUCH_START, _event.handler.onTouchstart);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_TOUCH_END, _event.handler.onMouseMove);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
    },
    bindInfoBtn: function bindInfoBtn() {
        _event.unbindInfoBtn();
        if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isAndroid) {
            amendInfo.main.addEventListener('touchend', _event.handler.onClick);
        } else {
            amendInfo.main.addEventListener('click', _event.handler.onClick);
        }
    },
    unbindInfoBtn: function unbindInfoBtn() {
        if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isAndroid) {
            amendInfo.main.removeEventListener('touchend', _event.handler.onClick);
        } else {
            amendInfo.main.removeEventListener('click', _event.handler.onClick);
        }
    },
    getEventPos: function getEventPos(e) {
        return {
            x: e.changedTouches ? e.changedTouches[0].clientX : e.clientX,
            y: e.changedTouches ? e.changedTouches[0].clientY : e.clientY
        };
    },
    handler: {
        /**
         * 检测 鼠标移动到的 dom 对象，是否需要显示 或 隐藏 amendInfo
         * @param e
         */
        onMouseMove: function onMouseMove(e) {
            //console.log('onMouseMove....')
            var curMousePos = _event.getEventPos(e);
            //如果鼠标没有移动， 仅仅输入文字导致触发mousemove事件时，不弹出信息框
            if (lastMousePos.x === curMousePos.x && lastMousePos.y === curMousePos.y) {
                return;
            }
            lastMousePos = curMousePos;
            if (pause) {
                return;
            }
            var target = e.target,
                isInfo = amendInfo.isInfo(target),
                scroll,
                pos = {
                width: 20,
                height: 20
            };

            //在 修订信息图层内移动， 不进行任何操作
            if (isInfo) {
                clearTimeout(amendInfo.showTimer);
                clearTimeout(amendInfo.hideTimer);
                return;
            }

            var sel = _commonEnv2['default'].doc.getSelection(),
                selectedDoms,
                targetDom = _amendUtilsAmendBase2['default'].getWizDeleteParent(target) || _amendUtilsAmendBase2['default'].getWizInsertParent(target);

            if (!sel.isCollapsed && targetDom && sel.containsNode(targetDom, true)) {
                //有选择区域， 且 target 在选择区域内
                selectedDoms = sel.isCollapsed ? null : _amendUtilsAmendBase2['default'].getAmendDoms({
                    selection: true,
                    selectAll: false
                });
            }

            //校验选择区域内是否有多个dom
            if (selectedDoms) {
                selectedDoms = selectedDoms.deletedInsertList.concat(selectedDoms.insertList, selectedDoms.deleteList);
                //选择多个修订内容时，不显示详细信息
                if (selectedDoms.length === 0) {
                    selectedDoms = null;
                    //} else if (selectedDoms.length == 1) {
                    //    targetDom = selectedDoms[0];
                    //    selectedDoms = null;
                }
            }
            var fontSize;
            if (selectedDoms || targetDom) {
                fontSize = parseInt(_commonEnv2['default'].win.getComputedStyle(targetDom)['font-size']);
                if (isNaN(fontSize)) {
                    fontSize = 14;
                }
                scroll = _domUtilsDomBase2['default'].getPageScroll();
                pos.left = curMousePos.x + scroll.left;
                pos.top = curMousePos.y + scroll.top - fontSize;
                if (pos.top < targetDom.offsetTop) {
                    pos.top = targetDom.offsetTop;
                }
                amendInfo.show(selectedDoms || targetDom, pos);
            } else {
                amendInfo.hide(false);
            }
        },
        onTouchstart: function onTouchstart(e) {
            //console.log('onTouchstart....')
            var target = e.target,
                isInfo = amendInfo.isInfo(target);
            if (isInfo) {
                return;
            }
            amendInfo.hide(false);
        },

        onClick: function onClick(e) {
            var target;
            if (e.changedTouches) {
                target = e.changedTouches[0].target;
            } else {
                target = e.target;
            }
            if (target.id == _commonConst2['default'].ID.AMEND_INFO_ACCEPT) {
                _event.handler.onAccept(e);
            } else if (target.id == _commonConst2['default'].ID.AMEND_INFO_REFUSE) {
                _event.handler.onRefuse(e);
            }
            _commonUtils2['default'].stopEvent(e);
        },
        onAccept: function onAccept(e) {
            if (callback.onAccept) {
                callback.onAccept(getCallbackParams());
            }
            amendInfo.hide(true);
            _commonWizUserAction2['default'].save(_commonWizUserAction2['default'].ActionId.ClickAcceptFromAmendInfo);
        },
        onRefuse: function onRefuse(e) {
            if (callback.onRefuse) {
                callback.onRefuse(getCallbackParams());
            }
            amendInfo.hide(true);
            _commonWizUserAction2['default'].save(_commonWizUserAction2['default'].ActionId.ClickRefuseFromAmendInfo);
        }
    }
};

/**
 * 创建 修订信息 图层
 */
function createAmendInfo() {
    var mask = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO),
        container;
    if (!mask) {
        mask = _commonEnv2['default'].doc.createElement('div');
        container = _commonEnv2['default'].doc.createElement('div');
        _domUtilsDomBase2['default'].setContenteditable(container, false);
        mask.appendChild(container);
        mask.id = _commonConst2['default'].ID.AMEND_INFO;
        _domUtilsDomBase2['default'].css(mask, {
            'position': 'absolute',
            'z-index': _commonConst2['default'].CSS.Z_INDEX.amendInfo,
            'display': 'none',
            'padding': '6px',
            'font-family': '"Microsoft Yahei","微软雅黑",Helvetica,SimSun,SimHei'
        }, false);
        container.innerHTML = getInfoTemplate();

        _domUtilsDomBase2['default'].css(container, {
            'background-color': 'white',
            'padding': '0px',
            'font-size': '12px',
            'border': '1px solid #D8D8D8',
            '-webkit-border-radius': '4px',
            '-moz-border-radius': '4px',
            '-border-radius': '4px',
            '-webkit-box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
            '-moz-box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
            'box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
            'min-width': '160px',
            'max-width': '280px',
            'min-height': '50px'
        }, false);

        amendInfo.template.appendChild(mask);
    }
    return mask;
}

function getInfoTemplate() {
    if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isMac || _commonEnv2['default'].client.type.isAndroid) {
        return '<div id="' + _commonConst2['default'].ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' + '<img id="' + _commonConst2['default'].ID.AMEND_INFO_IMG + '" class="' + _commonConst2['default'].CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute; -webkit-border-radius: 40px;-moz-border-radius:40px;border-radius:40px;">' + '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' + '</li><li style="line-height: 18px;text-align: right;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' + '<p style="margin: 4px 16px;">' + _commonLang2['default'].Amend.MultiInfo + '</p>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_TOOLS + '" style="padding:0;margin:0;box-sizing: border-box;">' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnRefuse + '</a></div>' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnAccept + '</a></div>' + '</div>';
    }

    //if (ENV.client.type.isWeb || ENV.client.type.isWin) {
    return '<div id="' + _commonConst2['default'].ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' + '<img id="' + _commonConst2['default'].ID.AMEND_INFO_IMG + '" class="' + _commonConst2['default'].CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute;">' + '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' + '</li><li style="line-height: 18px;text-align: right;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' + '<p style="margin: 4px 16px;">' + _commonLang2['default'].Amend.MultiInfo + '</p>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_TOOLS + '" style="padding:0;margin:0;box-sizing:border-box;border-top:1px solid #D8D8D8">' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;border-right: 1px solid #D8D8D8">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnAccept + '</a></div>' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnRefuse + '</a></div>' + '</div>';
    //}
}

function getCallbackParams() {
    return {
        dom: amendInfo.cur,
        isSelection: !!amendInfo.isSelection
    };
}

function initUserInfo(pos) {
    var dom = amendInfo.cur,
        guid = dom.getAttribute(_commonConst2['default'].ATTR.SPAN_USERID),
        user = _amendUser2['default'].getUserByGuid(guid),
        name = user ? user.name : _commonLang2['default'].Amend.UserNameDefault,
        time = dom.getAttribute(_commonConst2['default'].ATTR.SPAN_TIMESTAMP),
        isDelete = !!dom.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE),
        user = _amendUser2['default'].getUserByGuid(guid);
    time = time.substring(0, time.length - 3);

    amendInfo.curPos = pos;
    amendInfo.img.src = user ? user.imgUrl : '';
    amendInfo.name.innerText = name;
    amendInfo.name.setAttribute('title', name);
    amendInfo.content.innerText = isDelete ? _commonLang2['default'].Amend.Delete : _commonLang2['default'].Amend.Edit;
    amendInfo.time.innerText = time;

    amendInfo.multiUser.style.display = 'none';
    amendInfo.singleUser.style.display = 'block';
}

function initMultiInfo(pos) {
    amendInfo.curPos = pos;
    amendInfo.singleUser.style.display = 'none';
    amendInfo.multiUser.style.display = 'block';
}

function showInfo(pos) {

    if (amendInfo.main.parentNode == amendInfo.template) {
        _commonEnv2['default'].doc.body.appendChild(amendInfo.main);
        amendInfo.singleUser = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_SINGLE);
        amendInfo.multiUser = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_MULTI);
        amendInfo.img = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_IMG);
        amendInfo.name = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_NAME);
        amendInfo.content = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_CONTENT);
        amendInfo.time = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_TIME);
        amendInfo.tools = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_TOOLS);
        amendInfo.btnAccept = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_ACCEPT);
        amendInfo.btnRefuse = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO_REFUSE);
    }

    if (amendInfo.cur) {
        initUserInfo(pos);
    } else {
        initMultiInfo(pos);
    }

    _event.bindInfoBtn();

    if (amendInfo.readonly) {
        amendInfo.tools.style.display = 'none';
    } else {
        amendInfo.tools.style.display = 'block';
    }

    _domUtilsDomBase2['default'].css(amendInfo.main, {
        'top': '0px',
        'left': '0px',
        'display': 'block',
        'visibility': 'hidden'
    }, false);
    _domUtilsDomBase2['default'].setLayout({
        layerObj: amendInfo.main,
        target: pos,
        layout: _commonConst2['default'].TYPE.POS.upLeft,
        fixed: false,
        noSpace: false,
        reverse: true
        //reverse: !ENV.client.type.isPhone
    });
    _domUtilsDomBase2['default'].css(amendInfo.main, {
        'display': 'block',
        'visibility': 'visible'
    }, false);
}

function hideInfo() {
    if (amendInfo.main) {
        _event.unbindInfoBtn();
        amendInfo.cur = null;
        amendInfo.curPos = null;
        amendInfo.isMulti = false;
        amendInfo.isSelection = false;
        amendInfo.img.src = '';
        amendInfo.name.innerText = '';
        amendInfo.name.setAttribute('title', '');
        amendInfo.content.innerText = '';
        _domUtilsDomBase2['default'].css(amendInfo.main, {
            'display': 'none'
        }, false);
        amendInfo.template.appendChild(amendInfo.main);
    }
}

/**
 * 删除 修订信息 图层
 */
function removeAmendInfo() {
    var d = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_INFO);
    if (!!d) {
        d.parentNode.removeChild(d);
    }
}

exports['default'] = amendInfo;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/lang":8,"../common/utils":10,"../common/wizUserAction":12,"../domUtils/domBase":14,"./amendUser":2,"./amendUtils/amendBase":3}],2:[function(require,module,exports){
/**
 * 用于记录 当前操作者的信息
 * @type {{guid: string, hash: string, name: string, color: string, init: Function}}
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var DefaultImg = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAC0AAAAtCAYAAAA6GuKaAAAExUlEQVRYR9WZz08bRxTHvya1CHbUSHXCiZzCIaaBBOUUEveQSORWYwwRxRCpqtIqjZL8XQkEUppAbQMX6hgh5UetRiFV4MABQXoK2AgUx1vNzs7u7Hpmf3m3Vc3Blm2Gz/vOd95784goiqLgf/aIhAVdrVaxu7sLKomC7u7uwKQJBPrFi5eoVvfx6VMdZONOnIhjf78KBQoINQEnrwcuD+DkyS9bhvcNfXR0hFKppEKqWqp8BE0l1N7XgPXvKOjq6kJfX29L4L6hCfDh4aEnYBbQjRuD+PhxD4nEV77gfUEvLy/rXm1S2GIJQ3VtJ9TdoFtDfuKxOK5fv+YJ3jM0BfZmCd4y5tcUPp3+Njzo1dVV1Go1X5Yw+Z1oTOLWVI91HMfg4KBrcE9KLy0tq+mrVUvwwGSxzs5ODAxcDgt6yXWWcLIEVd5QezgzFA704iKBdk5rQmCLJdRzwYEPD2fCgS4WF408LMkSooOmhsl5mBYcTmlFQTY7HA50oVi0LRxeLcHDj4QGXShKK50jsGpfsyWY2uR5dCQbjtL5QsHUSzgVDidL8AcxNOiVlRXUagdccTEfSqGfLVmCt4RRpBTcHB0JR+lS6Tn29va04iIBdsgSvCVoxlMQ/SKKTCYdPPT29jb+qFSEB5HvJbxYgnqctq2958+jpyfpCtx1RVxY+M1eYUsKYyrqAWmf85ZgwAx+bOxm8ND0D4rLOK+wXZbQg+F6bALfQAOj2Syi0agjuGul5+cXTA1+EJbg7UFeZ4bS6OjoCA762fy87udWsoTMHkT43PiYIzD5gmul19bW8OHD36Yy7tUSVg/T8q6VeCjIjX8XLDRZ/OkzojZNdX6zhNUSzOPEGrFYLFhotlpxcQkHBwdaZTS3l7LCYWeJeDyGoTBvLgy8XC5jl1jFppeQZQm/luC3wLWn+V9qNBqYm/u1qb3kewmZDXjVe5JJ9PdfdGWJlqHJAk9+mQOBl1nC6dCRTZrIuTt41qh8Kc0WmZmZ1dXmFbTzMCtOly71I3nunGeVPaU80eo7Ozv4vfRcvzY1WULrK8wBqW7HRG7cF3DL0GSBqenHRk9iKc3WQ8eGka0ABwZtpzADNZ4VTE7kfKscCPSjqWm9vZRnDGoJVv1uTf7H0A8fTen24DOGSGH23q3JiX9f6Xq9jnfv/sKfb954BmbTqYsX+nDmTBcSiYTnAFynvK2tLZDhef3zZ60S0huHLNXxHrZTnXwWiUTQffYsUqkrrgKQQpMLbD6fF+ZhN4WD9zB/cTC/ZsN3YwysN1CZNE6fOiUMwgT99u061tfXjRmb4IrkpnCIgOVBNAObdg/A1z1JfJO6qgegQ5fLqyDFwnqnY/M2L4WD75GZh5nCIqWlFqOyq7sdaWvDnZ9uq+A69Ozsk1AUtt4rhd2fNkpg4wWzULT9JZ/d/uF7tLe3U+iNjU1tPGAMBu1uzbJKZ1W4FUuIhpQX+nqRSl2l0O/fb6BSqZhbTX32xmUJm14iVGCN5VjbMfx850cK/erVa2xuburb4LaBl6U1mSVE6zpZwphhE4s0cP/eXQqdzxewX61Kcy6bS9ComPLm0ixKa34PXXMyoMDkoUNPP56Rt5fcITGUEgMH6mGLPdl/fh/cv4t/ANultPKz243RAAAAAElFTkSuQmCC';

var AmendUser = function AmendUser(userInfo) {
    if (!userInfo) {
        userInfo = {};
    }
    this.guid = userInfo.user_guid || '';
    this.hash = getHash(this.guid);
    this.name = userInfo.user_name || '';
    this.imgUrl = userInfo.img_url ? userInfo.img_url : getImgUrl(this.guid);
    this.color = userInfo.color || createUserColor(this);
};
var curAmendUser = null;
var userDom = null;
var users = null,
    //用户 AmendUser 对象集合，用于 修订功能
usersForSave = null; //用户 保存数据，用于保存到 meta 内

var amendUserUtils = {
    initUser: function initUser(userInfo) {
        //初始化用户信息， 保证第一个修订用户的信息能被正常保存
        loadUsers();

        if (!userInfo) {
            return null;
        }

        curAmendUser = new AmendUser(userInfo);
        addUser(curAmendUser);
    },
    getCurUser: function getCurUser() {
        saveUser();
        return curAmendUser;
    },
    getUserByGuid: function getUserByGuid(guid) {
        if (curAmendUser && guid === curAmendUser.guid) {
            return curAmendUser;
        }
        if (users && users[guid]) {
            return users[guid];
        }
        loadUsers();
        return users[guid];
    },
    /**
     * 删除 修订颜色数据（用于确认修订）
     */
    removeAllUserInfo: function removeAllUserInfo() {
        var d = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_USER_INFO);
        if (!!d) {
            d.parentNode.removeChild(d);
        }
        userDom = null;
        users = null;
        usersForSave = null;
    },
    setUsersData: function setUsersData(_usersData) {
        var i, j, u, u1, u2;
        if (!_usersData) {
            return;
        }
        for (i = 0, j = _usersData.length; i < j; i++) {
            u = _usersData[i];
            u1 = users[u.user_guid];
            u2 = usersForSave[u.user_guid];
            if (u1 && u.user_name) {
                u1.name = u.user_name;
            }
            if (u1 && u.img_url) {
                u1.imgUrl = u.img_url;
            }
            if (u2 && u.user_name) {
                u2.name = u.user_name;
            }
        }
    }
};

function getHash(guid) {
    //hash = util.getHash(userInfo.user_guid);
    //hash = '_w' + hash;
    return guid;
}

function getImgUrl(guid) {
    if (_commonEnv2['default'].client.type.isWeb) {
        return '/wizas/a/users/avatar/' + guid + '?default=true&_' + new Date().valueOf();
    } else if (_commonEnv2['default'].client.type.isWin) {
        try {
            var avatarFileName = external.GetAvatarByUserGUID(guid);
            return avatarFileName ? avatarFileName : DefaultImg;
        } catch (e) {
            console.log(e);
        }
    } else if (_commonEnv2['default'].client.type.isMac) {} else if (_commonEnv2['default'].client.type.isIOS) {} else if (_commonEnv2['default'].client.type.isAndroid) {}

    return DefaultImg;
}
/**
 * 从客户端根据 guid 获取最新的用户昵称， 保证显示最新的用户昵称
 * @param guid
 * @returns {*}
 */
function getUserNameFromClient(guid) {
    if (_commonEnv2['default'].client.type.isWeb) {} else if (_commonEnv2['default'].client.type.isWin) {
        try {
            var userName = external.GetAliasByUserGUID(guid);
            return userName;
        } catch (e) {
            console.log(e);
        }
    } else if (_commonEnv2['default'].client.type.isMac) {} else if (_commonEnv2['default'].client.type.isIOS) {} else if (_commonEnv2['default'].client.type.isAndroid) {}

    return null;
}

function getUserDom() {
    if (userDom) {
        return userDom;
    }
    userDom = _commonEnv2['default'].doc.getElementById(_commonConst2['default'].ID.AMEND_USER_INFO);
    return userDom;
}
function createUserDom() {
    userDom = _commonEnv2['default'].doc.createElement('meta');
    userDom.id = _commonConst2['default'].ID.AMEND_USER_INFO;
    userDom.name = _commonConst2['default'].ID.AMEND_USER_INFO;
    _commonEnv2['default'].doc.getElementsByTagName('HEAD')[0].insertBefore(userDom, null);
}

function loadUsers() {
    if (users) {
        return;
    }
    var i, u, tmpName;

    users = {};
    usersForSave = {};

    userDom = getUserDom();
    if (!userDom) {
        return;
    }

    try {
        //根据已有数据获取曾经修订过的用户信息
        usersForSave = JSON.parse(userDom.content);

        for (i in usersForSave) {
            if (usersForSave.hasOwnProperty(i)) {
                u = usersForSave[i];
                u.user_guid = i;
                tmpName = getUserNameFromClient(i);
                if (tmpName) {
                    u.user_name = tmpName;
                } else {
                    u.user_name = u.name;
                }

                users[i] = new AmendUser(u);
            }
        }
    } catch (e) {}
}

/**
 * 根据 user 信息生成 修订颜色
 * @param user
 */
function createUserColor(user) {
    var userKey = user.hash,
        colorCount = _commonConst2['default'].COLOR.length,
        tmpColors = {},
        i,
        c;

    loadUsers();
    //如果该用户已有修订记录，直接使用
    if (users[userKey]) {
        return users[userKey].color;
    }

    //初始化 颜色列表，确认哪些颜色已被使用
    for (i in users) {
        if (users.hasOwnProperty(i)) {
            c = users[i].color;
            tmpColors[c] = true;
        }
    }

    for (i = 0; i < colorCount; i++) {
        c = _commonConst2['default'].COLOR[i];
        if (!tmpColors[c]) {
            return c;
        }
    }
    //如果所有颜色都被使用， 则直接使用第一种颜色
    return _commonConst2['default'].COLOR[0];
}

function addUser(user) {
    //如果已经存在，则替换数据
    users[user.guid] = user;
    usersForSave[user.guid] = {
        color: user.color,
        name: user.name
    };
}
function saveUser() {
    if (!userDom) {
        createUserDom();
    }

    userDom.content = JSON.stringify(usersForSave);
}

exports['default'] = amendUserUtils;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/utils":10}],3:[function(require,module,exports){
/**
 * amend 中通用的基本方法集合（基础操作，以读取为主）
 *
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomBase = require('../../domUtils/domBase');

var _domUtilsDomBase2 = _interopRequireDefault(_domUtilsDomBase);

var _rangeUtilsRangeBase = require('../../rangeUtils/rangeBase');

var _rangeUtilsRangeBase2 = _interopRequireDefault(_rangeUtilsRangeBase);

var _amendUser = require('./../amendUser');

var _amendUser2 = _interopRequireDefault(_amendUser);

var amendUtils = {
    /**
     * 根据条件 获取 修订的 dom 集合
     * @param options  {{[selection]: Boolean, [domList]: Array, [selectAll]: Boolean}}
     * @returns {{insertList: Array, deleteList: Array, deletedInsertList: Array}}
     */
    getAmendDoms: function getAmendDoms(options) {
        var i,
            j,
            d,
            insertAttr = {},
            deleteAttr = {},
            result = {
            insertList: [],
            deleteList: [],
            deletedInsertList: []
        },
            tmp = [];
        if (options.selection) {
            insertAttr[_commonConst2['default'].ATTR.SPAN_INSERT] = '';
            result.insertList = amendUtils.getWizSpanFromRange(options.selectAll, insertAttr);
            //清理出 删除&新增内容
            result.deletedInsertList = _domUtilsDomBase2['default'].removeListFilter(result.insertList, function (dom) {
                return dom.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE);
            });
            deleteAttr[_commonConst2['default'].ATTR.SPAN_DELETE] = '';
            result.deleteList = amendUtils.getWizSpanFromRange(options.selectAll, deleteAttr);
            //清理出 删除&新增内容
            tmp = _domUtilsDomBase2['default'].removeListFilter(result.deleteList, function (dom) {
                return dom.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT);
            });
            //合并从 insert & delete 集合中 清理出来的 删除&新增内容
            result.deletedInsertList = _commonUtils2['default'].removeDup(result.deletedInsertList.concat(tmp));
        } else {
            for (i = 0, j = options.domList.length; i < j; i++) {
                d = options.domList[i];
                if (d.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE) && d.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT)) {
                    result.deletedInsertList.push(d);
                } else if (d.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                    result.deleteList.push(d);
                } else if (d.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT)) {
                    result.insertList.push(d);
                }
            }
        }
        return result;
    },
    /**
     * 获取 与目标 连续且时间戳相近的 修订 dom 集合(必须同一用户的操作)
     * @param dom
     * @returns {Array}
     */
    getSameTimeStampDom: function getSameTimeStampDom(dom) {
        if (!dom || dom.nodeType != 1) {
            return [];
        }
        var result = [];

        findWizSibling(dom, true, result);
        result.push(dom);
        findWizSibling(dom, false, result);
        return result;

        function findWizSibling(target, isPrev, result) {
            var wizAmend,
                tmp,
                amendTypeTmp,
                amendType = getAmendType(target),
                time = target.getAttribute(_commonConst2['default'].ATTR.SPAN_TIMESTAMP),
                userId = target.getAttribute(_commonConst2['default'].ATTR.SPAN_USERID);
            if (!time) {
                return;
            }
            var sibling = getSibling(target, isPrev);
            while (sibling) {
                wizAmend = amendUtils.getWizInsertParent(sibling) || amendUtils.getWizDeleteParent(sibling);
                sibling = wizAmend;
                //首先判断是否同一用户
                if (sibling && sibling.getAttribute(_commonConst2['default'].ATTR.SPAN_USERID) !== userId) {
                    sibling = null;
                } else if (sibling) {
                    tmp = sibling.getAttribute(_commonConst2['default'].ATTR.SPAN_TIMESTAMP);
                    amendTypeTmp = getAmendType(sibling);
                    //时间相近的算法必须要考虑 删除其他用户新增的情况， 如果目标是（delete & insert）的情况，则相邻的也必须满足
                    if (amendType === amendTypeTmp && _commonUtils2['default'].isSameAmendTime(sibling.getAttribute(_commonConst2['default'].ATTR.SPAN_TIMESTAMP), time)) {
                        if (isPrev) {
                            result.splice(0, 0, sibling);
                        } else {
                            result.push(sibling);
                        }
                        sibling = getSibling(sibling, isPrev);
                    } else {
                        sibling = null;
                    }
                }
            }
        }

        function getAmendType(obj) {
            if (obj.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE) && obj.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT)) {
                return 1;
            } else if (obj.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT)) {
                return 2;
            } else if (obj.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                return 3;
            }
            return 0;
        }

        function getSibling(target, isPrev) {
            return isPrev ? _domUtilsDomBase2['default'].getPreviousNode(target, false, null) : _domUtilsDomBase2['default'].getNextNode(target, false, null);
        }
    },
    /**
     * 获取选择范围内 修订 dom 集合
     * @returns {*}
     */
    getSelectedAmendDoms: function getSelectedAmendDoms() {
        var sel = _commonEnv2['default'].doc.getSelection(),
            range = sel.getRangeAt(0),
            startDom,
            endDom,
            startOffset,
            endOffset;

        var amends = amendUtils.getAmendDoms({
            selection: true,
            selectAll: false
        });
        if (amends.insertList.length === 0 && amends.deleteList.length === 0 && amends.deletedInsertList.length === 0) {
            return null;
        }

        if (sel.isCollapsed) {
            //光标折叠状态时，不需要对 span 进行拆分
            return amends;
        }

        startDom = range.startContainer;
        startOffset = range.startOffset;
        endDom = range.endContainer;
        endOffset = range.endOffset;

        var start = checkStart(amends.deleteList, startDom, startOffset);
        if (!start) {
            start = checkStart(amends.insertList, startDom, startOffset);
            if (!start) {
                start = checkStart(amends.deletedInsertList, startDom, startOffset);
            }
        }
        var end = {};
        if (endDom === startDom && !!start) {
            end.dom = start.dom;
            end.offset = endOffset;
        } else {
            end = checkEnd(amends.deleteList, endDom, endOffset);
            if (!end) {
                end = checkEnd(amends.insertList, endDom, endOffset);
                if (!end) {
                    end = checkEnd(amends.deletedInsertList, endDom, endOffset);
                }
            }
        }

        amends.start = start;
        amends.end = end;

        return amends;

        function checkStart(list, startDom, startOffset) {
            if (list.length === 0 || startOffset === 0) {
                return null;
            }
            var s = list[0];
            if (s === startDom || _domUtilsDomBase2['default'].contains(s, startDom)) {
                list.splice(0, 1);
                return {
                    dom: startDom,
                    offset: startOffset
                };
            }
            return null;
        }

        function checkEnd(list, endDom, endOffset) {
            if (list.length === 0) {
                return null;
            }
            var maxLength = endDom.nodeType === 3 ? endDom.length : endDom.childNodes.length;
            if (endOffset === maxLength) {
                return null;
            }
            var e = list[list.length - 1];
            if (e === endDom || _domUtilsDomBase2['default'].contains(e, endDom)) {
                list.splice(list.length - 1, 1);
                return {
                    dom: endDom,
                    offset: endOffset
                };
            }
            return null;
        }
    },
    /**
     * 获取 wiz 编辑操作中 已标注的 Img 父节点
     * @param dom
     * @returns {*}
     */
    getWizAmendImgParent: function getWizAmendImgParent(dom) {
        return _domUtilsDomBase2['default'].getParentByFilter(dom, function (node) {
            return node && node.nodeType === 1 && node.getAttribute(_commonConst2['default'].ATTR.IMG);
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注为编辑的 父节点
     * @param dom
     * @returns {*}
     */
    getWizAmendParent: function getWizAmendParent(dom) {
        return _domUtilsDomBase2['default'].getParentByFilter(dom, function (node) {
            return node && node.nodeType === 1 && (node.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT) || node.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE));
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注删除的 父节点
     * @param dom
     * @returns {*}
     */
    getWizDeleteParent: function getWizDeleteParent(dom) {
        return _domUtilsDomBase2['default'].getParentByFilter(dom, function (node) {
            return node && node.nodeType === 1 && node.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE);
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注新增的 父节点
     * @param dom
     * @returns {*}
     */
    getWizInsertParent: function getWizInsertParent(dom) {
        return _domUtilsDomBase2['default'].getParentByFilter(dom, function (node) {
            //node.childNodes.length == 0 时，键盘敲入的字符加在 span 外面
            return node && node.nodeType === 1 && node.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT) && !node.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE) && node.childNodes.length > 0;
        }, true);
    },
    /**
     * 获取 鼠标选择范围内（Range）满足条件的 Wiz Span
     * @param isAll
     * @param options
     * @returns {*}
     */
    getWizSpanFromRange: function getWizSpanFromRange(isAll, options) {
        var exp = 'span',
            i,
            j,
            d;
        if (!options) {
            return [];
        }
        //根据 options 生成 dom 查询表达式
        for (i in options) {
            if (options.hasOwnProperty(i)) {
                if (options[i]) {
                    exp += '[' + i + '="' + options[i] + '"]';
                } else {
                    exp += '[' + i + ']';
                }
            }
        }

        var sel = _commonEnv2['default'].doc.getSelection(),
            range,
            startDom,
            startOffset,
            endDom,
            endOffset,
            startSpan,
            endSpan,
            parent,
            domList,
            startIndex,
            endIndex,
            dIdx,
            result = [];

        if (isAll) {
            //在 document.body 内进行查找
            var tmp = _commonEnv2['default'].doc.querySelectorAll(exp);
            for (i = 0, j = tmp.length; i < j; i++) {
                result.push(tmp[i]);
            }
            return result;
        }

        if (sel.rangeCount === 0) {
            return [];
        }

        if (sel.isCollapsed) {
            endDom = _rangeUtilsRangeBase2['default'].getRangeAnchor(false);
            startDom = _domUtilsDomBase2['default'].getPreviousNode(endDom, false, null);

            if (endDom) {
                endDom = _domUtilsDomBase2['default'].getParentByFilter(endDom, spanFilter, true);
                if (endDom) {
                    result.push(endDom);
                }
            }

            //TODO 对于换行的处理有问题，需要待定，暂时屏蔽
            //if (!endDom && startDom) {
            //    startDom = domUtils.getParentByFilter(startDom, spanFilter, true);
            //    if (startDom) {
            //        result.push(startDom);
            //    }
            //}

            return result;
        }
        startDom = _rangeUtilsRangeBase2['default'].getRangeAnchor(true);
        endDom = _rangeUtilsRangeBase2['default'].getRangeAnchor(false);

        if (!startDom || !endDom) {
            return [];
        }

        //获取 startDom, endDom 所在的 WizSpan 节点
        startSpan = _domUtilsDomBase2['default'].getParentByFilter(startDom, spanFilter, true);
        endSpan = _domUtilsDomBase2['default'].getParentByFilter(endDom, spanFilter, true);
        if (startSpan && startSpan == endSpan) {
            //startDom 和 endDom 所在同一个 WizSpan
            return [startSpan];
        }

        //在 startDom, endDom 共同的 parent 内根据查询表达式 查找 WizSpan
        parent = _domUtilsDomBase2['default'].getParentRoot([startDom, endDom]);
        domList = parent.querySelectorAll(exp);
        startIndex = _domUtilsDomBase2['default'].getIndexListByDom(startDom);
        endIndex = _domUtilsDomBase2['default'].getIndexListByDom(endDom);
        //startDom 是 TextNode 时，其父节点的 index 肯定要小于 startDom， 所以必须强行加入
        if (startSpan) {
            result.push(startSpan);
        }
        //根据 起始节点的 index 数据筛选 在其范围内的 WizSpan
        for (i = 0, j = domList.length; i < j; i++) {
            d = domList[i];
            dIdx = _domUtilsDomBase2['default'].getIndexListByDom(d);
            if (_domUtilsDomBase2['default'].compareIndexList(startIndex, dIdx) <= 0 && _domUtilsDomBase2['default'].compareIndexList(endIndex, dIdx) >= 0) {
                result.push(d);
            }
        }
        return result;

        /**
         * 查找 attribute 满足 options 的 Dom 节点过滤器
         * @param node
         * @returns {boolean}
         */
        function spanFilter(node) {
            if (!node || node.nodeType !== 1) {
                return false;
            }
            var i;
            for (i in options) {
                //option[i] == '' 表示 只看某属性是否存在，但不比较具体的value
                if (options.hasOwnProperty(i) && (!node.getAttribute(i) || options[i] && node.getAttribute(i) != options[i])) {
                    return false;
                }
            }
            return true;
        }
    },
    /**
     * 判断 是否为修订编辑的 笔记
     */
    isAmendEdited: function isAmendEdited() {
        var amendDoms = amendUtils.getAmendDoms({
            selection: true,
            selectAll: true
        });
        return !!amendDoms && (amendDoms.deleteList.length > 0 || amendDoms.insertList.length > 0 || amendDoms.deletedInsertList.length > 0);
    },
    /**
     * 判断 是否为 修订的 dom
     * @param dom
     * @returns {*|boolean}
     */
    isWizAmend: function isWizAmend(dom) {
        return amendUtils.getWizAmendParent(dom);
    },
    /**
     * 判断 是否为 删除内容
     * @param dom
     * @returns {boolean}
     */
    isWizDelete: function isWizDelete(dom) {
        return !!amendUtils.getWizDeleteParent(dom);
    },
    /**
     * 判断 是否为 新增内容
     * @param dom
     * @returns {boolean}
     */
    isWizInsert: function isWizInsert(dom) {
        return !!amendUtils.getWizInsertParent(dom);
    }
};

exports['default'] = amendUtils;
module.exports = exports['default'];

},{"../../common/const":4,"../../common/env":6,"../../common/utils":10,"../../domUtils/domBase":14,"../../rangeUtils/rangeBase":22,"./../amendUser":2}],4:[function(require,module,exports){
/**
 * 内部使用的标准常量.
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});
var FILL_CHAR = '​';
var CONST = {
    //String.fromCharCode(8203)
    FILL_CHAR: FILL_CHAR,
    FILL_CHAR_REG: new RegExp(FILL_CHAR, 'ig'),
    //在此间隔内修订，不生成新的 span，只修改修订时间
    AMEND_TIME_SPACE: 3 * 60 * 1000, // 3分钟
    //在此间隔内修订的内容， 被当作同一批次修订，批量拒绝或接受
    AMEND_BATCH_TIME_SPACE: 30 * 1000, // 30秒
    //判断是否正在进行中文输入法的标识（true 为正在进行中...）
    COMPOSITION_START: false,
    CLASS: {
        IMG_NOT_DRAG: 'wiz-img-cannot-drag',
        IMG_RESIZE_ACTIVE: 'wiz-img-resize-active',
        IMG_RESIZE_CONTAINER: 'wiz-img-resize-container',
        IMG_RESIZE_HANDLE: 'wiz-img-resize-handle',
        SELECTED_CELL: 'wiz-selected-cell',
        TABLE_CONTAINER: 'wiz-table-container',
        TABLE_TOOLS: 'wiz-table-tools',
        TABLE_BODY: 'wiz-table-body',
        TABLE_MENU_BUTTON: 'wiz-table-menu-button',
        TABLE_MENU_ITEM: 'wiz-table-menu-item',
        TABLE_MENU_SUB: 'wiz-table-menu-sub',
        TABLE_MOVING: 'wiz-table-moving'
    },
    ATTR: {
        IMG: 'data-wiz-img',
        IMG_MASK: 'data-wiz-img-mask',
        IMG_RATE: 'data-wiz-img-rate',
        SPAN: 'data-wiz-span',
        SPAN_USERID: 'data-wiz-user-id',
        SPAN_INSERT: 'data-wiz-insert',
        SPAN_DELETE: 'data-wiz-delete',
        SPAN_PASTE: 'data-wiz-paste',
        SPAN_PASTE_TYPE: 'data-wiz-paste-type',
        SPAN_PASTE_ID: 'data-wiz-paste-id',
        SPAN_TIMESTAMP: 'data-wiz-amend-time'
    },
    ID: {
        AMEND_INFO: 'wiz-amend-info',
        AMEND_INFO_SINGLE: 'wiz-amend-info-single',
        AMEND_INFO_MULTI: 'wiz-amend-info-multi',
        AMEND_INFO_NAME: 'wiz-amend-info-name',
        AMEND_INFO_IMG: 'wiz-amend-info-image',
        AMEND_INFO_CONTENT: 'wiz-amend-info-content',
        AMEND_INFO_TIME: 'wiz-amend-info-time',
        AMEND_INFO_TOOLS: 'wiz-amend-info-tools',
        AMEND_INFO_ACCEPT: 'wiz-amend-info-accept',
        AMEND_INFO_REFUSE: 'wiz-amend-info-refuse',
        AMEND_USER_INFO: 'wiz-amend-user',
        TABLE_RANGE_BORDER: 'wiz-table-range-border',
        TABLE_ROW_LINE: 'wiz-table-row-line',
        TABLE_COL_LINE: 'wiz-table-col-line'
    },
    NAME: {
        // NO_ABSTRACT_START: 'Document-Abstract-Start',
        // NO_ABSTRACT_END: 'Document-Abstract-End',
        TMP_STYLE: 'wiz_tmp_editor_style'
    },
    TAG: {
        TMP_TAG: 'wiz_tmp_tag'
    },
    TYPE: {
        IMG_DELETE: 'delete',
        IMG_INSERT: 'insert',
        PASTE: {
            START: 'start',
            END: 'end',
            CONTENT: 'content'
        },
        POS: {
            upLeft: 'up-left',
            downLeft: 'down-left',
            leftUp: 'left-up',
            rightUp: 'right-up',
            upRight: 'up-right',
            downRight: 'down-right',
            leftDown: 'left-down',
            rightDown: 'right-down'
        },
        TABLE: {
            COPY: 'copy',
            PASTE: 'paste',
            CLEAR_CELL: 'clearCell',
            MERGE_CELL: 'mergeCell',
            SPLIT_CELL: 'splitCell',
            INSERT_ROW_UP: 'insertRowUp',
            INSERT_ROW_DOWN: 'insertRowDown',
            INSERT_COL_LEFT: 'insertColLeft',
            INSERT_COL_RIGHT: 'insertColRight',
            DELETE_ROW: 'deleteRow',
            DELETE_COL: 'deleteCol',
            SET_CELL_BG: 'setCellBg',
            SET_CELL_ALIGN: 'setCellAlign',
            DISTRIBUTE_COLS: 'distributeCols',
            DELETE_TABLE: 'deleteTable'
        }
    },
    COLOR: ['#CB3C3C', '#0C9460', '#FF3399', '#FF6005', '#8058BD', '#009999', '#8AA725', '#339900', '#CC6600', '#3BBABA', '#D4CA1A', '#2389B0', '#006699', '#FF8300', '#2C6ED5', '#FF0000', '#B07CFF', '#CC3399', '#EB4847', '#3917E6'],
    CSS: {
        IMG: {
            SPAN: {
                position: 'relative',
                display: 'inline-block'
            },
            MASK: {
                position: 'absolute',
                width: '100% !important',
                height: '100% !important',
                top: '0',
                left: '0',
                opacity: '.5',
                filter: 'alpha(opacity=50)',
                border: '2px solid',
                'box-sizing': 'border-box',
                '-webkit-box-sizing': 'border-box',
                '-moz-box-sizing': 'border-box'
            }
        },
        IMG_DELETED: {
            background: '#fdc6c6 url(data:image/gif;base64,R0lGODlhDwAPAIABAIcUFP///yH5BAEKAAEALAAAAAAPAA8AAAIajI8IybadHjxyhjox1I0zH1mU6JCXCSpmUAAAOw==)',
            'border-color': '#E47070'
        },
        IMG_INSERT: {
            background: '#ccffcc',
            'border-color': '#00AA00'
        },
        Z_INDEX: {
            amendInfo: 150,
            tableBorder: 105,
            tableColRowLine: 120,
            tableRangeDot: 110,
            tableTDBefore: 100,
            tableTools: 130,
            tableToolsArrow: 10
        }
    },
    //客户端相关的事件定义
    CLIENT_EVENT: {
        WizEditorPaste: 'wizEditorPaste',
        wizReaderClickImg: 'wizReaderClickImg',
        wizMarkdownRender: 'wizMarkdownRender',
        wizEditorTrackEvent: 'wizEditorTrackEvent'
    },
    //全局事件 id 集合
    EVENT: {
        BEFORE_GET_DOCHTML: 'BEFORE_GET_DOCHTML',
        BEFORE_SAVESNAP: 'BEFORE_SAVESNAP',
        AFTER_RESTORE_HISTORY: 'AFTER_RESTORE_HISTORY',

        ON_COMPOSITION_START: 'ON_COMPOSITION_START',
        ON_COMPOSITION_END: 'ON_COMPOSITION_END',
        ON_COPY: 'ON_COPY',
        ON_CUT: 'ON_CUT',
        ON_DRAG_START: 'ON_DRAG_START',
        ON_DRAG_ENTER: 'ON_DRAG_ENTER',
        ON_DROP: 'ON_DROP',
        ON_KEY_DOWN: 'ON_KEY_DOWN',
        ON_KEY_UP: 'ON_KEY_UP',
        ON_MOUSE_DOWN: 'ON_MOUSE_DOWN',
        ON_MOUSE_MOVE: 'ON_MOUSE_MOVE',
        ON_MOUSE_OVER: 'ON_MOUSE_OVER',
        ON_MOUSE_UP: 'ON_MOUSE_UP',
        ON_PASTE: 'ON_PASTE',
        ON_SCROLL: 'ON_SCROLL',
        ON_SELECT_CHANGE: 'ON_SELECT_CHANGE',
        ON_SELECT_START: 'ON_SELECT_START',
        ON_TOUCH_START: 'ON_TOUCH_START',
        ON_TOUCH_END: 'ON_TOUCH_END',
        UPDATE_RENDER: 'UPDATE_RENDER'
    },
    AMEND: {
        INFO_SPACE: 0, //修订信息图层与目标间隔
        INFO_TIMER: 300 //修订timer 间隔
    }
};

exports['default'] = CONST;
module.exports = exports['default'];

},{}],5:[function(require,module,exports){
/**
 * 依赖的 css && 非可打包的 js 文件加载控制
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _utils = require('./utils');

var _utils2 = _interopRequireDefault(_utils);

var _scriptLoader = require('./scriptLoader');

var _scriptLoader2 = _interopRequireDefault(_scriptLoader);

function loadGroup(doc, group, callback) {
    _scriptLoader2['default'].load(doc, group, callback);
}

function makeCallback(doc, loadFiles, callback) {
    var count = 0,
        max = loadFiles.length;

    var cb = function cb() {
        if (count < max) {
            loadGroup(doc, loadFiles[count++], cb);
        } else if (callback) {
            callback();
        }
    };

    return cb;
}

var dependLoader = {
    loadJs: function loadJs(doc, loadFiles, callback) {
        var cb = makeCallback(doc, loadFiles, callback);
        cb();
    },
    loadCss: function loadCss(doc, loadFiles) {
        var i, j;
        for (i = 0, j = loadFiles.length; i < j; i++) {
            _utils2['default'].loadSingleCss(doc, loadFiles[i]);
        }
    }
};

exports['default'] = dependLoader;
module.exports = exports['default'];

},{"./scriptLoader":9,"./utils":10}],6:[function(require,module,exports){
/**
 * wizEditor 环境参数，保存当前 document 等
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

var GlobalEvent = {};
var WizNotCmdInditify = 'wiznotecmd://';

var ENV = {
    win: null,
    doc: null,
    dependency: {
        files: {
            css: {
                fonts: '',
                github2: '',
                wizToc: ''
            },
            js: {
                jquery: '',
                prettify: '',
                raphael: '',
                underscore: '',
                flowchart: '',
                sequence: '',
                mathJax: 'http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-MML-AM_CHTML'
            },
            init: function init(cssFiles, jsFiles) {
                _append('fonts', cssFiles, ENV.dependency.files.css);
                _append('github2', cssFiles, ENV.dependency.files.css);
                _append('wizToc', cssFiles, ENV.dependency.files.css);

                _append('jquery', jsFiles, ENV.dependency.files.js);
                _append('prettify', jsFiles, ENV.dependency.files.js);
                _append('raphael', jsFiles, ENV.dependency.files.js);
                _append('underscore', jsFiles, ENV.dependency.files.js);
                _append('flowchart', jsFiles, ENV.dependency.files.js);
                _append('sequence', jsFiles, ENV.dependency.files.js);
                _append('mathJax', jsFiles, ENV.dependency.files.js);

                function _append(id, src, target) {
                    if (!src || !target) {
                        return;
                    }

                    if (src[id]) {
                        target[id] = src[id];
                    }
                }
            }
        },
        css: {
            fons: ['fonts'],
            markdown: ['github2', 'wizToc']
        },
        js: {
            markdown: [['jquery'], ['prettify', 'raphael', 'underscore'], ['flowchart', 'sequence']],
            mathJax: [['jquery'], ['mathJax']]
        }
    },
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
        sendCmdToWiznote: function sendCmdToWiznote() {},
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
                    var url;
                    if (cmd == _const2['default'].CLIENT_EVENT.wizReaderClickImg) {
                        url = WizNotCmdInditify + cmd + '?src=' + encodeURIComponent(options.src);
                    } else if (cmd == _const2['default'].CLIENT_EVENT.wizEditorTrackEvent) {
                        url = WizNotCmdInditify + cmd + '?id=' + encodeURIComponent(options.id) + '&e=' + encodeURIComponent(options.event);
                    } else {
                        url = WizNotCmdInditify + cmd;
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
                    if (cmd == _const2['default'].CLIENT_EVENT.wizReaderClickImg) {
                        ENV.win.WizNote.onClickImg(options.src, options.imgList);
                    }
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
    },
    event: {
        add: function add(eventId, fun) {
            if (!eventId || !fun || checkFun(eventId, fun)) {
                return;
            }
            var eList = GlobalEvent[eventId];
            if (!eList) {
                eList = [];
            }
            eList.push(fun);
            GlobalEvent[eventId] = eList;

            function checkFun(eventId, fun) {
                if (!eventId || !fun) {
                    return false;
                }
                var i,
                    j,
                    eList = GlobalEvent[eventId];

                if (!eList || eList.length === 0) {
                    return false;
                }
                for (i = 0, j = eList.length; i < j; i++) {
                    if (eList[i] === fun) {
                        return true;
                    }
                }
                return false;
            }
        },
        call: function call(eventId) {
            var i,
                j,
                args = [],
                eList = GlobalEvent[eventId];

            if (!eList || eList.length === 0) {
                return;
            }
            for (i = 1, j = arguments.length; i < j; i++) {
                args.push(arguments[i]);
            }
            for (i = 0, j = eList.length; i < j; i++) {
                eList[i].apply(this, args);
            }
        },
        remove: function remove(eventId, fun) {
            if (!eventId || !fun) {
                return;
            }
            var i,
                j,
                eList = GlobalEvent[eventId];

            if (!eList || eList.length === 0) {
                return;
            }
            for (i = 0, j = eList.length; i < j; i++) {
                if (eList[i] === fun) {
                    eList.splice(i, 1);
                }
            }
        }
    }
};

exports['default'] = ENV;
module.exports = exports['default'];

},{"./const":4}],7:[function(require,module,exports){
/**
 * undo、redo 工具包
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _env = require('./env');

var _env2 = _interopRequireDefault(_env);

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

var _utils = require('./utils');

var _utils2 = _interopRequireDefault(_utils);

var _domUtilsDomExtend = require('./../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('./../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

var MaxRedo = 100;
var historyUtils = {
    enable: false,
    /**
     * 执行 undo 操作时，触发的回调函数， 返回 history 的缓存集合数量，以及当前 undo、redo 操作游标的所在位置——用于控制 undo、redo 按钮的 disabled
     */
    callback: null,
    /**
     * undo 集合
     */
    stack: [],
    /**
     * undo 集合当前游标位置
     */
    stackIndex: 0,
    /**
     * 初始化 historyUtils 工具包
     */
    init: function init() {
        historyUtils.stack = [];
        historyUtils.stackIndex = 0;
    },
    /**
     * 开启 history 功能
     * @param maxRedo
     * @param callback
     */
    start: function start(maxRedo, callback) {
        if (maxRedo && maxRedo > 0) {
            MaxRedo = maxRedo;
        }
        historyUtils.enable = true;
        historyUtils.init();
        historyEvent.bind();
        if (callback) {
            historyUtils.callback = callback;
        }
    },
    /**
     * 关闭 history 功能
     */
    stop: function stop() {
        historyUtils.enable = false;
        historyUtils.init();
        historyEvent.unbind();
    },
    /**
     * 触发 callback
     */
    applyCallback: function applyCallback() {
        if (historyUtils.callback) {
            historyUtils.callback(historyUtils.getUndoState());
        }
    },
    getUndoState: function getUndoState() {
        return {
            'undoCount': historyUtils.stack.length,
            'undoIndex': historyUtils.stackIndex
        };
    },
    /**
     * undo 操作
     */
    undo: function undo() {
        //console.log('.....undo....');
        if (!historyUtils.enable || historyUtils.stackIndex <= 0 || historyUtils.stack.length === 0) {
            historyUtils.stackIndex = 0;
            return;
        }
        if (historyUtils.stackIndex >= historyUtils.stack.length) {
            historyUtils.saveSnap(true);
        }
        //console.log('.....restore.....' + historyUtils.stack.length + ',' + historyUtils.stackIndex);
        historyUtils.restore(historyUtils.stack[--historyUtils.stackIndex]);
        historyUtils.applyCallback();
        _domUtilsDomExtend2['default'].focus();
        //            console.log('undo: ' + historyUtils.stackIndex);
    },
    /**
     * redo 操作
     */
    redo: function redo() {
        //console.log('.....redo....');
        if (!historyUtils.enable || historyUtils.stackIndex >= historyUtils.stack.length - 1) {
            return;
        }
        historyUtils.restore(historyUtils.stack[++historyUtils.stackIndex]);
        historyUtils.applyCallback();
        _domUtilsDomExtend2['default'].focus();
        //            console.log('redo: ' + historyUtils.stackIndex);
    },
    /**
     * 保存当前内容的快照
     * @param keepIndex （是否保存快照时不移动游标， 主要用于 undo 操作时保存最后的快照）
     */
    saveSnap: function saveSnap(keepIndex) {
        if (!historyUtils.enable || _const2['default'].COMPOSITION_START) {
            return;
        }

        _env2['default'].event.call(_const2['default'].EVENT.BEFORE_SAVESNAP);

        var canSave = { add: true, replace: false, direct: 0 },
            snap = historyUtils.snapshot();
        if (!keepIndex && historyUtils.stack.length > 0 && historyUtils.stackIndex > 0) {
            canSave = historyUtils.canSave(snap, historyUtils.stack[historyUtils.stackIndex - 1]);
        }
        if (canSave.add || canSave.replace) {
            //console.log('save snap.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
            //记录 光标移动方向，用于判断是删除还是添加字符
            snap.direct = canSave.direct;

            if (historyUtils.stackIndex >= 0) {
                historyUtils.stack.splice(historyUtils.stackIndex, historyUtils.stack.length - historyUtils.stackIndex);
            }
            //                console.log(snap.content);
            if (canSave.add) {
                //console.log('save snap.add.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
                historyUtils.stack.push(snap);
                if (!keepIndex) {
                    historyUtils.stackIndex++;
                }
            } else if (canSave.replace) {
                //console.log('save snap.replace.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
                historyUtils.stack[historyUtils.stackIndex - 1] = snap;
                if (keepIndex) {
                    historyUtils.stackIndex--;
                }
            }
        }

        if (historyUtils.stack.length > MaxRedo) {
            historyUtils.stack.shift();
            historyUtils.stackIndex--;
        }
        historyUtils.applyCallback();
    },
    /**
     * 根据指定的 快照 恢复页面内容
     * @param snap
     */
    restore: function restore(snap) {
        if (!historyUtils.enable || !snap) {
            return;
        }
        var sel = _env2['default'].doc.getSelection(),
            start,
            end;
        _env2['default'].doc.body.innerHTML = snap.content;
        try {
            start = _domUtilsDomExtend2['default'].getDomByIndexList(snap.focus.start);
            sel.collapse(start.dom, start.offset);
            if (!snap.focus.isCollapsed) {
                end = _domUtilsDomExtend2['default'].getDomByIndexList(snap.focus.end);
                _rangeUtilsRangeExtend2['default'].setRange(start.dom, start.offset, end.dom, end.offset);
            } else {
                _rangeUtilsRangeExtend2['default'].setRange(start.dom, start.offset, start.dom, start.offset);
            }
            _rangeUtilsRangeExtend2['default'].caretFocus();
        } catch (e) {}
        _env2['default'].event.call(_const2['default'].EVENT.AFTER_RESTORE_HISTORY);
    },
    /**
     * 判断 当前快照是否可以保存
     * @param s1
     * @param s2
     * @returns {{add: boolean, replace: boolean}}
     */
    canSave: function canSave(s1, s2) {
        var result = { add: false, replace: false, direct: 0 };
        if (s1.content.length != s2.content.length || !!s1.content.localeCompare(s2.content)) {
            result.direct = compareFocus(s1.focus, s2.focus);
            if (result.direct === 0 || result.direct !== s2.direct) {
                result.add = true;
            } else {
                result.replace = true;
            }
        }
        //console.log(' ..... can Save .....')
        //console.log(s1)
        //console.log(s2)
        //console.log(result)
        return result;

        function compareFocus(f1, f2) {
            if (f1.isCollapsed != f2.isCollapsed) {
                return 0;
            }
            if (f1.start.length != f2.start.length || f1.end.length != f2.end.length) {
                return 0;
            }
            var result = compareIndexList(f1.start, f2.start);
            if (result < 1) {
                return result;
            }
            result = compareIndexList(f1.end, f2.end);
            return result;
        }

        function compareIndexList(index1, index2) {
            var isSame = 1,
                i,
                j;
            for (i = 0, j = index1.length - 1; i < j; i++) {
                if (index1[i] != index2[i]) {
                    isSame = 0;
                    break;
                }
            }
            if (isSame && index1[j] < index2[j]) {
                isSame = -1;
            }
            //console.log('.....compareIndexList.....')
            //console.log(index1)
            //console.log(index2)
            //console.log(isSame)
            return isSame;
        }
    },
    /**
     * 生成快照
     * @returns {{content: string, focus: {isCollapsed: boolean, start: Array, end: Array}}}
     */
    snapshot: function snapshot() {
        var sel = _env2['default'].doc.getSelection(),
            content = _env2['default'].doc.body.innerHTML,
            focus = {
            isCollapsed: true,
            start: [],
            end: []
        },
            snap = {
            content: content,
            focus: focus
        };

        if (sel.rangeCount === 0) {
            focus.start.push(0);
            return snap;
        }

        var range = sel.getRangeAt(0);
        focus.start = _domUtilsDomExtend2['default'].getIndexListByDom(range.startContainer);
        focus.start.push(range.startOffset);
        focus.isCollapsed = sel.isCollapsed;
        if (!sel.isCollapsed) {
            focus.end = _domUtilsDomExtend2['default'].getIndexListByDom(range.endContainer);
            focus.end.push(range.endOffset);
        }
        return snap;
    }
};

/**
 * 历史记录功能的 事件处理
 */
var historyEvent = {
    /**
     * 初始化时， 绑定历史记录相关的必要事件
     */
    bind: function bind() {
        historyEvent.unbind();
        _env2['default'].event.add(_const2['default'].EVENT.ON_KEY_DOWN, historyEvent.onKeyDown);
    },
    /**
     * 解绑历史记录相关的必要事件
     */
    unbind: function unbind() {
        _env2['default'].event.remove(_const2['default'].EVENT.ON_KEY_DOWN, historyEvent.onKeyDown);
    },
    /**
     * 快捷键 监控
     * @param e
     */
    onKeyDown: function onKeyDown(e) {

        var keyCode = e.keyCode || e.which;
        //console.log('history keydown.....' + keyCode);

        /**
         * Ctrl + Z
         */
        if (e.ctrlKey && keyCode == 90 || e.metaKey && keyCode == 90 && !e.shiftKey) {
            historyUtils.undo();
            _utils2['default'].stopEvent(e);
            return;
        }
        /**
         * Ctrl + Y
         */
        if (e.ctrlKey && keyCode == 89 || e.metaKey && keyCode == 89 || e.metaKey && keyCode == 90 && e.shiftKey) {
            historyUtils.redo();
            _utils2['default'].stopEvent(e);
        }
    }
};

exports['default'] = historyUtils;
module.exports = exports['default'];

},{"./../domUtils/domExtend":15,"./../rangeUtils/rangeExtend":23,"./const":4,"./env":6,"./utils":10}],8:[function(require,module,exports){
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
    Amend: {
        Edit: 'Inserted contents',
        Delete: 'Deleted contents',
        BtnAccept: 'Accept',
        BtnRefuse: 'Reject',
        Accept: 'Accept all changes? Or partially select the changes which need to be accepted.',
        Refuse: 'Reject all changes? Or partially select the changes which need to be rejected.',
        MultiInfo: 'Multiple changes are selected',
        UserNameDefault: 'someone'
    },
    Table: {
        Copy: 'Copy',
        Paste: 'Paste',
        ClearCell: 'Clear',
        MergeCell: 'Merge Cells',
        SplitCell: 'Unmerge Cells',
        InsertRowUp: 'Add Row Above',
        InsertRowDown: 'Add Row Below',
        InsertColLeft: 'Add Column Before',
        InsertColRight: 'Add Column After',
        DeleteRow: 'Delete Row',
        DeleteCol: 'Delete Column',
        SetCellBg: 'Color Fill',
        CellAlign: 'Arrange',
        DeleteTable: 'Delete Table',
        DistrbuteCols: 'Average Column Width'
    },
    Err: {
        Copy_Null: 'Copy of deleted changes not allowed',
        Cut_Null: 'Cut of deleted changes not allowed'
    }
};
LANG['zh-cn'] = {
    Amend: {
        Edit: '插入了内容',
        Delete: '删除了内容',
        BtnAccept: '接受修订',
        BtnRefuse: '拒绝修订',
        Accept: '是否确认接受全部修订内容？ 如需接受部分内容请使用鼠标进行选择',
        Refuse: '是否确认拒绝全部修订内容？ 如需拒绝部分内容请使用鼠标进行选择',
        MultiInfo: '您选中了多处修订',
        UserNameDefault: '有人'
    },
    Table: {
        Copy: '复制',
        Paste: '粘贴',
        ClearCell: '清空单元格',
        MergeCell: '合并单元格',
        SplitCell: '拆分单元格',
        InsertRowUp: '上插入行',
        InsertRowDown: '下插入行',
        InsertColLeft: '左插入列',
        InsertColRight: '右插入列',
        DeleteRow: '删除当前行',
        DeleteCol: '删除当前列',
        SetCellBg: '单元格底色',
        CellAlign: '单元格对齐方式',
        DeleteTable: '删除表格',
        DistrbuteCols: '平均分配各列'
    },
    Err: {
        Copy_Null: '无法复制已删除的内容',
        Cut_Null: '无法剪切已删除的内容'
    }
};
LANG['zh-tw'] = {
    Amend: {
        Edit: '插入了內容',
        Delete: '刪除了內容',
        BtnAccept: '接受修訂',
        BtnRefuse: '拒絕修訂',
        Accept: '是否確認接受全部修訂內容？ 如需接受部分內容請使用滑鼠進行選擇',
        Refuse: '是否確認拒絕全部修訂內容？ 如需拒絕部分內容請使用滑鼠進行選擇',
        MultiInfo: '您選中了多處修訂',
        UserNameDefault: '有人'
    },
    Table: {
        Copy: '複製',
        Paste: '粘貼',
        ClearCell: '清空儲存格',
        MergeCell: '合併儲存格',
        SplitCell: '拆分儲存格',
        InsertRowUp: '上插入行',
        InsertRowDown: '下插入行',
        InsertColLeft: '左插入列',
        InsertColRight: '右插入列',
        DeleteRow: '刪除當前行',
        DeleteCol: '刪除當前列',
        SetCellBg: '儲存格底色',
        CellAlign: '儲存格對齊方式',
        DeleteTable: '刪除表格',
        DistrbuteCols: '平均分配各列'
    },
    Err: {
        Copy_Null: '無法複製已刪除的內容',
        Cut_Null: '無法剪切已刪除的內容'
    }
};

function setLang(type) {
    if (!type) {
        type = 'en';
    }
    //同时支持 zh-cn & zh_cn
    type = type.toLowerCase().replace('_', '-');
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

},{}],9:[function(require,module,exports){
/*
 *用于加载js
 *options是数组，值有
 *  字符串：js地址
 *  对象(js需保存到localStorage)：
 *      {
 *         id:"",
 *         version:"",
 *         link:""
 *      }
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

var _utils = require('./utils');

var _utils2 = _interopRequireDefault(_utils);

var scriptLoader = {
    appendJsCode: function appendJsCode(doc, jsStr, type) {
        var s = doc.createElement('script');
        s.type = type;
        s.text = jsStr;
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].appendChild(s);
    },
    load: function load(doc, options, callback) {
        if (!doc || !options) {
            return;
        }
        var i,
            j,
            s,
            c,
            id = new Date().valueOf(),
            allLoaded = true;
        for (i = 0, j = options.length; i < j; i++) {
            if (typeof options[i] == "string") {
                s = this.loadSingleJs(doc, options[i]);
                if (s !== true) {
                    s.onload = makeLoadHandle(id, callback);
                    allLoaded = false;
                }
            } else {
                var jsUrl = options[i].link,
                    jsId = createJsId(options[i].id),
                    jsVersion = options[i].version;
                if (window.localStorage) {
                    var jsInfo = JSON.parse(localStorage.getItem(jsId));
                    if (jsInfo && jsInfo.version == jsVersion) {
                        s = this.inject(doc, jsInfo.jsStr, jsId);
                        if (s !== true) {
                            c = makeLoadHandle(id, callback);
                            setTimeout(function () {
                                c();
                            }, 10);
                            allLoaded = false;
                        }
                    } else {
                        allLoaded = false;
                        c = makeLoadHandle(id, callback);
                        $.ajax({
                            url: jsUrl,
                            context: { id: jsId, version: jsVersion },
                            success: function success(data) {
                                save({ id: this.id, version: this.version, jsStr: data });
                                s = wizUI.scriptLoader.inject(doc, data, this.id);
                                if (s !== true) {
                                    setTimeout(function () {
                                        c();
                                    }, 10);
                                }
                            },
                            error: function error() {
                                c();
                            }
                        });
                    }
                } else {
                    s = this.loadSingleJs(doc, options[i].link);
                    if (s !== true) {
                        s.onload = makeLoadHandle(id, callback);
                        allLoaded = false;
                    }
                }
            }
        }
        if (allLoaded) {
            callback();
        }
    },
    loadSingleJs: function loadSingleJs(doc, path) {
        var jsId = 'wiz_' + path;
        if (doc.getElementById(jsId)) {
            return true;
        }
        var s = doc.createElement('script');
        s.type = 'text/javascript';
        s.setAttribute('charset', "utf-8");
        s.src = path.replace(/\\/g, '/');
        s.id = jsId;
        //s.className = utils.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    inject: function inject(doc, jsStr, jsId) {
        if (!doc || doc.getElementById(jsId)) {
            return true;
        }
        var s = doc.createElement("script");
        s.type = 'text/javascript';
        s.id = jsId;
        s.text = jsStr;
        //s.className = utils.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    }
};
var loadCount = {};

function makeLoadHandle(id, loadCallback) {
    if (!loadCount[id]) {
        loadCount[id] = 0;
    }
    loadCount[id]++;
    return function () {
        loadCount[id]--;
        if (loadCount[id] === 0) {
            loadCount[id] = null;
            if (loadCallback) {
                loadCallback();
            }
        }
    };
}

function createJsId(jsId) {
    return "wiz_js_" + jsId;
}

function save(options) {
    if (!options) {
        return;
    }
    var jsInfo = {
        version: options.version,
        jsStr: options.jsStr
    };
    localStorage.setItem(options.id, JSON.stringify(jsInfo));
}

exports['default'] = scriptLoader;
module.exports = exports['default'];

},{"./const":4,"./utils":10}],10:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

if (!String.prototype.trim) {
    String.prototype.trim = function () {
        return this.replace(/^\s+|\s+$/g, '');
    };
}
if (!Array.prototype.indexOf) {
    Array.prototype.indexOf = function (n) {
        for (var i = 0; i < this.length; i++) {
            if (this[i] == n) {
                return i;
            }
        }
        return -1;
    };
}

/**
 * 常用基本工具包
 */
var utils = {
    /**
     * 判断 obj 是否为 数组
     * @param obj
     * @returns {boolean}
     */
    isArray: function isArray(obj) {
        return Object.prototype.toString.apply(obj) === "[object Array]";
    },
    /**
     * 判断字符串是否为空， 空格 不认为空
     * @param str
     * @returns {boolean}
     */
    isEmpty: function isEmpty(str) {
        if (!str) {
            return true;
        }
        var enter = /\r?\n/ig,
            r = new RegExp('[\r\n' + _const2['default'].FILL_CHAR + ']', 'ig'),
            hasEnter = enter.test(str),
            _str = str.replace(r, ''),
            isNone = str.replace(r, '').trim().length === 0;
        //避免 正常标签只存在 一个空格时，也被误判
        return _str.length === 0 || hasEnter && isNone;
    },
    /**
     * 判断两个 修订时间是否近似相同
     * @param time1
     * @param time2
     * @returns {boolean}
     */
    isSameAmendTime: function isSameAmendTime(time1, time2) {
        if (!time1 || !time2) {
            return false;
        }
        var t1 = utils.getDateForTimeStr(time1),
            t2 = utils.getDateForTimeStr(time2);
        return Math.abs(t1 - t2) <= _const2['default'].AMEND_BATCH_TIME_SPACE;
    },
    /**
     * 获取字符串 的 hash 值
     * @param str
     * @returns {number}
     */
    getHash: function getHash(str) {
        var hash = 1315423911,
            i,
            ch;
        for (i = str.length - 1; i >= 0; i--) {
            ch = str.charCodeAt(i);
            hash ^= (hash << 5) + ch + (hash >> 2);
        }
        return hash & 0x7FFFFFFF;
    },
    /**
     * 生成当前时间戳，用于 修订的时间
     * @returns {string}
     */
    getTime: function getTime() {
        var d = new Date();
        return d.getFullYear() + '-' + to2(d.getMonth() + 1) + '-' + to2(d.getDate()) + ' ' + to2(d.getHours()) + ':' + to2(d.getMinutes()) + ':' + to2(d.getSeconds());

        function to2(num) {
            var str = num.toString();
            return str.length == 1 ? '0' + str : str;
        }
    },
    /**
     * 根据 日期字符串 返回 Date 对象（用于修订编辑，所以只支持 yyyy-mm-hh HH:MM:SS 格式）
     * @param str
     * @returns {Date}
     */
    getDateForTimeStr: function getDateForTimeStr(str) {
        return new Date(Date.parse(str.replace(/-/g, "/")));
    },
    /**
     * 将 list 转换为 Map （主要用于处理 tagNames）
     * @param list
     * @returns {{}}
     */
    listToMap: function listToMap(list) {
        if (!list) {
            return {};
        }
        list = utils.isArray(list) ? list : list.split(',');
        var i,
            j,
            ci,
            obj = {};
        for (i = 0, j = list.length; i < j; i++) {
            ci = list[i];
            obj[ci.toUpperCase()] = obj[ci] = 1;
        }
        return obj;
    },
    rgb2Hex: function rgb2Hex(str) {
        if (!str) {
            return '';
        }
        var rgb = str.replace(/.*\((.*)\)/ig, '$1').split(',');
        if (rgb.length < 3) {
            return '';
        }
        var r = parseInt(rgb[0], 10),
            g = parseInt(rgb[1], 10),
            b = parseInt(rgb[2], 10),
            a = rgb.length === 4 ? parseFloat(rgb[3]) : 1;
        if (a === 0) {
            return '';
        }
        return '#' + getHex(getColor(r, a)) + getHex(getColor(g, a)) + getHex(getColor(b, a));

        function getColor(color, colorA) {
            return color + Math.floor((255 - color) * (1 - colorA));
        }
        function getHex(n) {
            var h = n.toString(16);
            return h.length == 1 ? '0' + h : h;
        }
    },
    /**
     * 删除 数组中重复的数据
     * @param arr
     * @returns {Array}
     */
    removeDup: function removeDup(arr) {
        var result = [],
            i,
            j,
            a;
        for (i = 0, j = arr.length; i < j; i++) {
            a = arr[i];
            if (result.indexOf(a) < 0) {
                result.push(a);
            }
        }
        return result;
    },
    /**
     * 阻止默认事件
     * @param e
     */
    stopEvent: function stopEvent(e) {
        if (!e) {
            return;
        }
        e.stopPropagation();
        e.preventDefault();
        //这个会阻止其他同event 的触发，过于野蛮
        //e.stopImmediatePropagation();
    },
    //-------------------- 以下内容修改需要 保证与 wizUI 中的 utils 内 对应方法一致 start ----------------------
    //PcCustomTagClass: 'wiz-html-render-unsave', //此 class 专门用于 pc 端将 markdown 笔记选然后发email 或微博等处理
    loadSingleCss: function loadSingleCss(doc, path) {
        var cssId = 'wiz_' + path;
        if (doc.getElementById(cssId)) {
            return true;
        }

        var s = doc.createElement('link');
        s.rel = 'stylesheet';
        s.setAttribute('charset', "utf-8");
        s.href = path.replace(/\\/g, '/');
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    appendCssCode: function appendCssCode(doc, jsStr, type) {
        var s = doc.createElement('style');
        s.type = type;
        s.text = jsStr;
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].appendChild(s);
    },
    /**
     * FF下无法获取innerText，通过解析DOM树来解析innerText，来渲染markdown
     * @param ele 需要解析的节点元素
     * @returns {string}
     */
    getInnerText: function getInnerText(ele) {

        var t = '';

        var normalize = function normalize(a) {
            if (!a) {
                return "";
            }
            return a.replace(/ +/gm, " ").replace(/[\t]+/gm, "").replace(/[ ]+$/gm, "").replace(/^[ ]+/gm, "").replace(/\n+/gm, "\n").replace(/\n+$/, "").replace(/^\n+/, "").replace(/NEWLINE/gm, '\n');
            //return a.replace(/ +/g, " ")
            //    .replace(/[\t]+/gm, "")
            //    .replace(/[ ]+$/gm, "")
            //    .replace(/^[ ]+/gm, "")
            //    .replace(/\n+/g, "\n")
            //    .replace(/\n+$/, "")
            //    .replace(/^\n+/, "")
        };
        var removeWhiteSpace = function removeWhiteSpace(node) {
            // 去掉空的文本节点
            var isWhite = function isWhite(node) {
                return !/[^\t\n\r ]/.test(node.nodeValue);
            };
            var ws = [];
            var findWhite = function findWhite(node) {
                for (var i = 0; i < node.childNodes.length; i++) {
                    var n = node.childNodes[i];
                    if (n.nodeType == 3 && isWhite(n)) {
                        ws.push(n);
                    } else if (n.hasChildNodes()) {
                        findWhite(n);
                    }
                }
            };
            findWhite(node);
            for (var i = 0; i < ws.length; i++) {
                ws[i].parentNode.removeChild(ws[i]);
            }
        };
        var sty = function sty(n, prop) {
            // 获取节点的style
            if (n.style[prop]) {
                return n.style[prop];
            }
            var s = n.currentStyle || n.ownerDocument.defaultView.getComputedStyle(n, null);
            if (n.tagName == "SCRIPT") {
                return "none";
            }
            if (!s[prop]) {
                return "LI,P,TR".indexOf(n.tagName) > -1 ? "block" : n.style[prop];
            }
            if (s[prop] == "block" && n.tagName == "TD") {
                return "feaux-inline";
            }
            return s[prop];
        };

        var blockTypeNodes = "table-row,block,list-item";
        var isBlock = function isBlock(n) {
            // 判断是否为block元素
            var s = sty(n, "display") || "feaux-inline";
            return blockTypeNodes.indexOf(s) > -1;
        };
        // 遍历所有子节点，收集文本内容，注意需要空格和换行
        var recurse = function recurse(n) {
            // 处理pre元素
            if (/pre/.test(sty(n, "whiteSpace"))) {
                t += n.innerHTML.replace(/\t/g, " ");
                return "";
            }
            var s = sty(n, "display");
            if (s == "none") {
                return "";
            }
            var gap = isBlock(n) ? "\n" : " ";
            t += gap;
            for (var i = 0; i < n.childNodes.length; i++) {
                var c = n.childNodes[i];
                if (c.nodeType == 3) {
                    t += c.nodeValue;
                }

                if (c.childNodes.length) {
                    recurse(c);
                }
            }
            t += gap;
            return t;
        };

        var node = ele.cloneNode(true);
        // br转换成会忽略换行, 会出现 <span>aaa</span><br><span>bbb</span> 的情况，因此用一个特殊字符代替，而不是直接替换成 \n
        node.innerHTML = node.innerHTML.replace(/<br[\/]?>/gi, 'NEWLINE');

        // p元素会多一个换行，暂时用NEWLINE进行占位，markdown中不考虑p元素
        //var paras = node.getElementsByTagName('p');
        //for(var i = 0; i < paras.length; i++) {
        //    paras[i].innerHTML += 'NEWLINE';
        //}
        removeWhiteSpace(node);
        return normalize(recurse(node));
    },

    /**
     * 对markdown的html内容进行预处理，已显示图片，todoList等等
     * @param dom 传入的dom对象
     */
    markdownPreProcess: function markdownPreProcess(dom) {
        function htmlUnEncode(input) {
            return String(input).replace(/\&amp;/g, '&').replace(/\&gt;/g, '>').replace(/\&lt;/g, '<').replace(/\&quot;/g, '"').replace(/\&&#39;/g, "'");
        }

        var el = $(dom);
        el.find('label.wiz-todo-label').each(function (index) {
            //检测如果是遗留的 label 则不进行特殊处理
            var img = $('.wiz-todo-img', this);
            if (img.length === 0) {
                return;
            }

            var span = $("<span></span>");
            //避免 父节点是 body 时导致笔记阅读异常
            span.text(htmlUnEncode($(this)[0].outerHTML));
            span.insertAfter($(this));
            $(this).remove();
        });
        el.find('img').each(function (index) {
            var span = $("<span></span>");
            span.text(htmlUnEncode($(this)[0].outerHTML));
            span.insertAfter($(this));
            $(this).remove();
        });
        el.find('a').each(function (index, link) {
            var linkObj = $(link);
            var href = linkObj.attr('href');
            if (href && /^(wiz|wiznote):/.test(href)) {
                var span = $("<span></span>");
                span.text(htmlUnEncode(linkObj[0].outerHTML));
                span.insertAfter(linkObj);
                linkObj.remove();
            }
        });
        el.find('p').each(function () {
            $(this).replaceWith($('<div>' + this.innerHTML + '</div>'));
        });
    }
    //-------------------- 以上内容修改需要 保证与 wizUI 中的 utils 内 对应方法一致 end ----------------------
};

exports['default'] = utils;
module.exports = exports['default'];

},{"./const":4}],11:[function(require,module,exports){
/**
 * 默认的样式集合
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _env = require('./env');

var _env2 = _interopRequireDefault(_env);

var _const = require('./const');

var _const2 = _interopRequireDefault(_const);

var TmpEditorStyle = {
    phone: 'body {' + 'overflow-y:scroll;' + '-webkit-overflow-scrolling: touch;' + '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' + '}',
    pad: 'body {' + 'min-width: 90%;' + 'max-width: 100%;' + 'min-height: 100%;' + 'background: #ffffff;' + 'overflow-y:scroll;' + '-webkit-overflow-scrolling: touch;' + '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' + '}'
},
    TmpReaderStyle = {
    phone: 'img {' + 'max-width: 100%;' + 'height: auto !important;' + 'margin: 0px auto;' + 'cursor: pointer;' + //专门用于 ios 点击 img 触发 click 事件
    '}' + 'a {' + 'word-wrap: break-word;' + '}' + 'body {' + 'word-wrap: break-word;' + '}'
},
    DefaultStyleId = 'wiz_custom_css',
    DefaultFont = 'Helvetica, "Hiragino Sans GB", "微软雅黑", "Microsoft YaHei UI", SimSun, SimHei, arial, sans-serif;',
    DefaultStyle = {
    common: 'html, body {' + 'font-size: 15px;' + '}' + 'body {' + 'font-family: ' + DefaultFont + 'line-height: 1.6;' + 'margin: 0;padding: 20px 15px;padding: 1.33rem 1rem;' + '}' + 'h1, h2, h3, h4, h5, h6 {margin:20px 0 10px;margin:1.33rem 0 0.667rem;padding: 0;font-weight: bold;}' + 'h1 {font-size:21px;font-size:1.4rem;}' + 'h2 {font-size:20px;font-size:1.33rem;}' + 'h3 {font-size:18px;font-size:1.2rem;}' + 'h4 {font-size:17px;font-size:1.13rem;}' + 'h5 {font-size:15px;font-size:1rem;}' + 'h6 {font-size:15px;font-size:1rem;color: #777777;margin: 1rem 0;}' + 'div, p, ul, ol, dl, li {margin:0;}' + 'blockquote, table, pre, code {margin:8px 0;}' + 'ul, ol {padding-left:32px;padding-left:2.13rem;}' + 'blockquote {padding:0 12px;padding:0 0.8rem;}' + 'blockquote > :first-child {margin-top:0;}' + 'blockquote > :last-child {margin-bottom:0;}' + 'img {border:0;max-width:100%;height:auto !important;margin:2px 0;}' + 'table {border-collapse:collapse;border:1px solid #bbbbbb;}' + 'td, th {padding:4px 8px;border-collapse:collapse;border:1px solid #bbbbbb;height:28px;word-break:break-all;box-sizing: border-box;position: relative;}' + '@media only screen and (-webkit-max-device-width: 1024px), only screen and (-o-max-device-width: 1024px), only screen and (max-device-width: 1024px), only screen and (-webkit-min-device-pixel-ratio: 3), only screen and (-o-min-device-pixel-ratio: 3), only screen and (min-device-pixel-ratio: 3) {' + 'html,body {font-size:17px;}' + 'body {line-height:1.7;padding:0.75rem 0.9375rem;color:#353c47;}' + 'h1 {font-size:2.125rem;}' + 'h2 {font-size:1.875rem;}' + 'h3 {font-size:1.625rem;}' + 'h4 {font-size:1.375rem;}' + 'h5 {font-size:1.125rem;}' + 'h6 {color: inherit;}' + 'ul, ol {padding-left:2.5rem;}' + 'blockquote {padding:0 0.9375rem;}' + '}'
},
    ImageResizeStyle = '.wiz-img-resize-handle {position: absolute;z-index: 1000;border: 1px solid black;background-color: white;}' + '.wiz-img-resize-handle {width:5px;height:5px;}' + '.wiz-img-resize-handle.lt {cursor: nw-resize;}' + '.wiz-img-resize-handle.tm {cursor: n-resize;}' + '.wiz-img-resize-handle.rt {cursor: ne-resize;}' + '.wiz-img-resize-handle.lm {cursor: w-resize;}' + '.wiz-img-resize-handle.rm {cursor: e-resize;}' + '.wiz-img-resize-handle.lb {cursor: sw-resize;}' + '.wiz-img-resize-handle.bm {cursor: s-resize;}' + '.wiz-img-resize-handle.rb {cursor: se-resize;}',
    TableContainerStyle = '.' + _const2['default'].CLASS.TABLE_CONTAINER + ' {}' + '.' + _const2['default'].CLASS.TABLE_BODY + ' {position:relative;padding:0 0 10px;overflow-x:auto;-webkit-overflow-scrolling:touch;}' + '.' + _const2['default'].CLASS.TABLE_BODY + ' table {margin:0;outline:none;}' + 'td,th {height:28px;word-break:break-all;box-sizing:border-box;position:relative;outline:none;}',
    TableEditStyle = '.' + _const2['default'].CLASS.TABLE_BODY + '.' + _const2['default'].CLASS.TABLE_MOVING + ' *,' + ' .' + _const2['default'].CLASS.TABLE_BODY + '.' + _const2['default'].CLASS.TABLE_MOVING + ' *:before,' + ' .' + _const2['default'].CLASS.TABLE_BODY + '.' + _const2['default'].CLASS.TABLE_MOVING + ' *:after {cursor:default !important;}' + '#wiz-table-range-border {display: none;width: 0;height: 0;position: absolute;top: 0;left: 0; z-index:' + _const2['default'].CSS.Z_INDEX.tableBorder + '}' + '#wiz-table-col-line, #wiz-table-row-line {display: none;background-color: #448aff;position: absolute;z-index:' + _const2['default'].CSS.Z_INDEX.tableColRowLine + ';}' + '#wiz-table-col-line {width: 1px;cursor:col-resize;}' + '#wiz-table-row-line {height: 1px;cursor:row-resize;}' + '#wiz-table-range-border_start, #wiz-table-range-border_range {display: none;width: 0;height: 0;position: absolute;}' + '#wiz-table-range-border_start_top, #wiz-table-range-border_range_top {height: 2px;background-color: #448aff;position: absolute;top: 0;left: 0;}' + '#wiz-table-range-border_range_top {height: 1px;}' + '#wiz-table-range-border_start_right, #wiz-table-range-border_range_right {width: 2px;background-color: #448aff;position: absolute;top: 0;}' + '#wiz-table-range-border_range_right {width: 1px;}' + '#wiz-table-range-border_start_bottom, #wiz-table-range-border_range_bottom {height: 2px;background-color: #448aff;position: absolute;top: 0;}' + '#wiz-table-range-border_range_bottom {height: 1px;}' + '#wiz-table-range-border_start_left, #wiz-table-range-border_range_left {width: 2px;background-color: #448aff;position: absolute;top: 0;left: 0;}' + '#wiz-table-range-border_range_left {width: 1px;}' + '#wiz-table-range-border_start_dot, #wiz-table-range-border_range_dot {width: 5px;height: 5px;border: 2px solid rgb(255, 255, 255);background-color: #448aff;cursor: crosshair;position: absolute;z-index:' + _const2['default'].CSS.Z_INDEX.tableRangeDot + ';}' + '.wiz-table-tools {display: block;background-color:#fff;position: absolute;left: 0px;border: 1px solid #ddd;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;z-index:' + _const2['default'].CSS.Z_INDEX.tableTools + ';}' + '.wiz-table-tools ul {list-style: none;padding: 0;}' + '.wiz-table-tools .wiz-table-menu-item {position: relative;float: left;margin:5px 2px 5px 8px;}' + '.wiz-table-tools .wiz-table-menu-item .wiz-table-menu-button {width: 20px;height: 20px;cursor: pointer;position:relative;}' + '.wiz-table-tools i.editor-icon{font-size: 15px;color: #455a64;}' + '.wiz-table-tools .wiz-table-menu-item .wiz-table-menu-button i#wiz-menu-bg-demo{position: absolute;top:3px;left:0;}' + '.wiz-table-tools .wiz-table-menu-sub {position: absolute;display: none;width: 125px;padding: 5px 0;background: #fff;border-radius: 3px;border: 1px solid #E0E0E0;top:28px;left:-9px;box-shadow: 1px 1px 5px #d0d0d0;}' + '.wiz-table-tools .wiz-table-menu-item.active .wiz-table-menu-sub {display: block}' + '.wiz-table-tools .wiz-table-menu-sub:before, .wiz-table-tools .wiz-table-menu-sub:after {position: absolute;content: " ";border-style: solid;border-color: transparent;border-bottom-color: #cccccc;left: 22px;margin-left: -14px;top: -8px;border-width: 0 8px 8px 8px;z-index:' + _const2['default'].CSS.Z_INDEX.tableToolsArrow + ';}' + '.wiz-table-tools .wiz-table-menu-sub:after {border-bottom-color: #ffffff;top: -7px;}' + '.wiz-table-tools .wiz-table-menu-sub-item {padding: 4px 12px;font-size: 14px;}' + '.wiz-table-tools .wiz-table-menu-sub-item.split {border-top: 1px solid #E0E0E0;}' + '.wiz-table-tools .wiz-table-menu-sub-item:hover {background-color: #ececec;}' + '.wiz-table-tools .wiz-table-menu-sub-item.disabled {color: #bbbbbb;cursor: default;}' + '.wiz-table-tools .wiz-table-menu-sub-item.disabled:hover {background-color: transparent;}' + '.wiz-table-tools .wiz-table-menu-item.wiz-table-cell-bg:hover .wiz-table-color-pad {display: block;}' + '.wiz-table-tools .wiz-table-color-pad {display: none;padding: 10px;box-sizing: border-box;width: 85px;height: 88px;background-color: #fff;cursor: default;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item {display: inline-block;width: 15px;height: 15px;margin-right: 9px;position: relative;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item i.pad-demo {position: absolute;top:3px;left:0;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item .icon-oblique_line{color: #cc0000;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item:last-child {margin-right: 0;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item.active i.editor-icon.icon-box {color: #448aff;}' + '.wiz-table-tools .wiz-table-cell-align {display: none;padding: 10px;box-sizing: border-box;width: 85px;height: 65px;background-color: #fff;cursor: default;}' + '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item {display: inline-block;width: 15px;height: 15px;margin-right: 9px;position: relative;}' + '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item:last-child {margin-right:0}' + '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item i.valign{position: absolute;top:3px;left:0;color: #d2d2d2;}' + '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.valign {color: #a1c4ff;}' + '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.icon-box,' + '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.align {color: #448aff;}' + '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item:last-child,' + '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item:last-child {margin-right: 0;}' + 'th.wiz-selected-cell, td.wiz-selected-cell {background: rgba(0,102,255,.05);}' + 'th:before,td:before,#wiz-table-col-line:before,#wiz-table-range-border_start_right:before,#wiz-table-range-border_range_right:before{content: " ";position: absolute;top: 0;bottom: 0;right: -5px;width: 9px;cursor: col-resize;background: transparent;z-index:' + _const2['default'].CSS.Z_INDEX.tableTDBefore + ';}' + 'th:after,td:after,#wiz-table-row-line:before,#wiz-table-range-border_start_bottom:before,#wiz-table-range-border_range_bottom:before{content: " ";position: absolute;left: 0;right: 0;bottom: -5px;height: 9px;cursor: row-resize;background: transparent;z-index:' + _const2['default'].CSS.Z_INDEX.tableTDBefore + ';}';

function replaceStyleById(id, css, isReplace) {
    //isReplace = true 则 只进行替换， 如无同id 的元素，不进行任何操作
    isReplace = !!isReplace;

    var s = _env2['default'].doc.getElementById(id);
    if (!s && !isReplace) {
        s = _env2['default'].doc.createElement('style');
        s.id = id;
        _env2['default'].doc.getElementsByTagName('HEAD')[0].insertBefore(s, null);
    }
    if (s) {
        s.innerHTML = css;
    }
}

var WizStyle = {
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        replaceStyleById(DefaultStyleId, DefaultStyle.common, isReplace);
        if (!customCss) {
            return;
        }
        var css,
            k,
            hasCustomCss = false;
        if (typeof customCss == 'string') {
            css = customCss;
            hasCustomCss = true;
        } else {
            css = 'html, body{';
            for (k in customCss) {
                if (customCss.hasOwnProperty(k)) {
                    if (k.toLowerCase() == 'font-family') {
                        css += k + ':' + customCss[k] + ',' + DefaultFont + ';';
                    } else {
                        css += k + ':' + customCss[k] + ';';
                    }
                    hasCustomCss = true;
                }
            }
            css += '}';
        }

        if (hasCustomCss) {
            WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, css);
        }
    },
    insertStyle: function insertStyle(options, css) {
        var s = _env2['default'].doc.createElement('style');
        if (options.name) {
            s.setAttribute('name', options.name);
        }
        if (options.id) {
            s.setAttribute('id', options.id);
        }
        _env2['default'].doc.getElementsByTagName('HEAD')[0].insertBefore(s, null);
        s.innerHTML = css;
        return s;
    },
    insertTmpEditorStyle: function insertTmpEditorStyle() {
        WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, ImageResizeStyle + TableEditStyle + TableContainerStyle);

        if (_env2['default'].client.type.isIOS && _env2['default'].client.type.isPhone) {
            WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, TmpEditorStyle.phone);
        } else if (_env2['default'].client.type.isIOS && _env2['default'].client.type.isPad) {
            WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, TmpEditorStyle.pad);
        }
    },
    insertTmpReaderStyle: function insertTmpReaderStyle() {
        WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, TableContainerStyle);
        if (_env2['default'].client.type.isIOS) {
            WizStyle.insertStyle({ name: _const2['default'].NAME.TMP_STYLE }, TmpReaderStyle.phone);
        }
    }
};

exports['default'] = WizStyle;
module.exports = exports['default'];

},{"./const":4,"./env":6}],12:[function(require,module,exports){
/**
 * 专门用于记录用户行为的 log
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var ActionId = {
    ClickAcceptFromAmendInfo: 'ClickAcceptFromAmendInfo',
    ClickRefuseFromAmendInfo: 'ClickRefuseFromAmendInfo'
};

var wizUserAction = {
    save: function save(id) {
        if (_commonEnv2['default'].client.type.isWin) {
            try {
                if (external && external.LogAction) {
                    external.LogAction(id);
                }
            } catch (e) {
                console.log(e.toString());
            }
        }
    }
};

var UserAction = {
    ActionId: ActionId,
    save: wizUserAction.save
};

exports['default'] = UserAction;
module.exports = exports['default'];

},{"../common/env":6}],13:[function(require,module,exports){
/**
 * 兼容 ES6 将 require 替换为 _require
 */
"use strict";

Object.defineProperty(exports, "__esModule", {
  value: true
});
(function e(t, n, r) {
  function s(o, u) {
    if (!n[o]) {
      if (!t[o]) {
        var a = typeof _require == "function" && _require;if (!u && a) return a(o, !0);if (i) return i(o, !0);var f = new Error("Cannot find module '" + o + "'");throw (f.code = "MODULE_NOT_FOUND", f);
      }var l = n[o] = { exports: {} };t[o][0].call(l.exports, function (e) {
        var n = t[o][1][e];return s(n ? n : e);
      }, l, l.exports, e, t, n, r);
    }return n[o].exports;
  }var i = typeof _require == "function" && _require;for (var o = 0; o < r.length; o++) s(r[o]);return s;
})({ 1: [function (_require, module, exports) {
    /**
     * 默认配置
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var FilterCSS = _require('cssfilter').FilterCSS;
    var _ = _require('./util');

    // 默认白名单
    var whiteList = {
      a: ['target', 'href', 'title'],
      abbr: ['title'],
      address: [],
      area: ['shape', 'coords', 'href', 'alt'],
      article: [],
      aside: [],
      audio: ['autoplay', 'controls', 'loop', 'preload', 'src'],
      b: [],
      bdi: ['dir'],
      bdo: ['dir'],
      big: [],
      blockquote: ['cite'],
      br: [],
      caption: [],
      center: [],
      cite: [],
      code: [],
      col: ['align', 'valign', 'span', 'width'],
      colgroup: ['align', 'valign', 'span', 'width'],
      dd: [],
      del: ['datetime'],
      details: ['open'],
      div: [],
      dl: [],
      dt: [],
      em: [],
      font: ['color', 'size', 'face'],
      footer: [],
      h1: [],
      h2: [],
      h3: [],
      h4: [],
      h5: [],
      h6: [],
      header: [],
      hr: [],
      i: [],
      img: ['src', 'alt', 'title', 'width', 'height'],
      ins: ['datetime'],
      li: [],
      mark: [],
      nav: [],
      ol: [],
      p: [],
      pre: [],
      s: [],
      section: [],
      small: [],
      span: [],
      sub: [],
      sup: [],
      strong: [],
      table: ['width', 'border', 'align', 'valign'],
      tbody: ['align', 'valign'],
      td: ['width', 'colspan', 'align', 'valign'],
      tfoot: ['align', 'valign'],
      th: ['width', 'colspan', 'align', 'valign'],
      thead: ['align', 'valign'],
      tr: ['rowspan', 'align', 'valign'],
      tt: [],
      u: [],
      ul: [],
      video: ['autoplay', 'controls', 'loop', 'preload', 'src', 'height', 'width']
    };

    // 默认CSS Filter
    var defaultCSSFilter = new FilterCSS();

    /**
     * 匹配到标签时的处理方法
     *
     * @param {String} tag
     * @param {String} html
     * @param {Object} options
     * @return {String}
     */
    function onTag(tag, html, options) {}
    // do nothing

    /**
     * 匹配到不在白名单上的标签时的处理方法
     *
     * @param {String} tag
     * @param {String} html
     * @param {Object} options
     * @return {String}
     */
    function onIgnoreTag(tag, html, options) {}
    // do nothing

    /**
     * 匹配到标签属性时的处理方法
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @return {String}
     */
    function onTagAttr(tag, name, value) {}
    // do nothing

    /**
     * 匹配到不在白名单上的标签属性时的处理方法
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @return {String}
     */
    function onIgnoreTagAttr(tag, name, value) {}
    // do nothing

    /**
     * HTML转义
     *
     * @param {String} html
     */
    function escapeHtml(html) {
      return html.replace(REGEXP_LT, '&lt;').replace(REGEXP_GT, '&gt;');
    }

    /**
     * 安全的标签属性值
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @param {Object} cssFilter
     * @return {String}
     */
    function safeAttrValue(tag, name, value, cssFilter) {
      cssFilter = cssFilter || defaultCSSFilter;
      // 转换为友好的属性值，再做判断
      value = friendlyAttrValue(value);

      if (name === 'href' || name === 'src') {
        // 过滤 href 和 src 属性
        // 仅允许 http:// | https:// | mailto: | / | # 开头的地址
        value = _.trim(value);
        if (value === '#') return '#';
        if (!(value.substr(0, 7) === 'http://' || value.substr(0, 8) === 'https://' || value.substr(0, 7) === 'mailto:' || value[0] === '#' || value[0] === '/')) {
          return '';
        }
      } else if (name === 'background') {
        // 过滤 background 属性 （这个xss漏洞较老了，可能已经不适用）
        // javascript:
        REGEXP_DEFAULT_ON_TAG_ATTR_4.lastIndex = 0;
        if (REGEXP_DEFAULT_ON_TAG_ATTR_4.test(value)) {
          return '';
        }
      } else if (name === 'style') {
        // /*注释*/
        /*REGEXP_DEFAULT_ON_TAG_ATTR_3.lastIndex = 0;
         if (REGEXP_DEFAULT_ON_TAG_ATTR_3.test(value)) {
         return '';
         }*/
        // expression()
        REGEXP_DEFAULT_ON_TAG_ATTR_7.lastIndex = 0;
        if (REGEXP_DEFAULT_ON_TAG_ATTR_7.test(value)) {
          return '';
        }
        // url()
        REGEXP_DEFAULT_ON_TAG_ATTR_8.lastIndex = 0;
        if (REGEXP_DEFAULT_ON_TAG_ATTR_8.test(value)) {
          REGEXP_DEFAULT_ON_TAG_ATTR_4.lastIndex = 0;
          if (REGEXP_DEFAULT_ON_TAG_ATTR_4.test(value)) {
            return '';
          }
        }
        value = cssFilter.process(value);
      }

      // 输出时需要转义<>"
      value = escapeAttrValue(value);
      return value;
    }

    // 正则表达式
    var REGEXP_LT = /</g;
    var REGEXP_GT = />/g;
    var REGEXP_QUOTE = /"/g;
    var REGEXP_QUOTE_2 = /&quot;/g;
    var REGEXP_ATTR_VALUE_1 = /&#([a-zA-Z0-9]*);?/img;
    var REGEXP_ATTR_VALUE_COLON = /&colon;?/img;
    var REGEXP_ATTR_VALUE_NEWLINE = /&newline;?/img;
    var REGEXP_DEFAULT_ON_TAG_ATTR_3 = /\/\*|\*\//mg;
    var REGEXP_DEFAULT_ON_TAG_ATTR_4 = /((j\s*a\s*v\s*a|v\s*b|l\s*i\s*v\s*e)\s*s\s*c\s*r\s*i\s*p\s*t\s*|m\s*o\s*c\s*h\s*a)\:/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_5 = /^[\s"'`]*(d\s*a\s*t\s*a\s*)\:/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_6 = /^[\s"'`]*(d\s*a\s*t\s*a\s*)\:\s*image\//ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_7 = /e\s*x\s*p\s*r\s*e\s*s\s*s\s*i\s*o\s*n\s*\(.*/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_8 = /u\s*r\s*l\s*\(.*/ig;

    /**
     * 对双引号进行转义
     *
     * @param {String} str
     * @return {String} str
     */
    function escapeQuote(str) {
      return str.replace(REGEXP_QUOTE, '&quot;');
    }

    /**
     * 对双引号进行转义
     *
     * @param {String} str
     * @return {String} str
     */
    function unescapeQuote(str) {
      return str.replace(REGEXP_QUOTE_2, '"');
    }

    /**
     * 对html实体编码进行转义
     *
     * @param {String} str
     * @return {String}
     */
    function escapeHtmlEntities(str) {
      return str.replace(REGEXP_ATTR_VALUE_1, function replaceUnicode(str, code) {
        return code[0] === 'x' || code[0] === 'X' ? String.fromCharCode(parseInt(code.substr(1), 16)) : String.fromCharCode(parseInt(code, 10));
      });
    }

    /**
     * 对html5新增的危险实体编码进行转义
     *
     * @param {String} str
     * @return {String}
     */
    function escapeDangerHtml5Entities(str) {
      return str.replace(REGEXP_ATTR_VALUE_COLON, ':').replace(REGEXP_ATTR_VALUE_NEWLINE, ' ');
    }

    /**
     * 清除不可见字符
     *
     * @param {String} str
     * @return {String}
     */
    function clearNonPrintableCharacter(str) {
      var str2 = '';
      for (var i = 0, len = str.length; i < len; i++) {
        str2 += str.charCodeAt(i) < 32 ? ' ' : str.charAt(i);
      }
      return _.trim(str2);
    }

    /**
     * 将标签的属性值转换成一般字符，便于分析
     *
     * @param {String} str
     * @return {String}
     */
    function friendlyAttrValue(str) {
      str = unescapeQuote(str); // 双引号
      str = escapeHtmlEntities(str); // 转换HTML实体编码
      str = escapeDangerHtml5Entities(str); // 转换危险的HTML5新增实体编码
      str = clearNonPrintableCharacter(str); // 清除不可见字符
      return str;
    }

    /**
     * 转义用于输出的标签属性值
     *
     * @param {String} str
     * @return {String}
     */
    function escapeAttrValue(str) {
      str = escapeQuote(str);
      str = escapeHtml(str);
      return str;
    }

    /**
     * 去掉不在白名单中的标签onIgnoreTag处理方法
     */
    function onIgnoreTagStripAll() {
      return '';
    }

    /**
     * 删除标签体
     *
     * @param {array} tags 要删除的标签列表
     * @param {function} next 对不在列表中的标签的处理函数，可选
     */
    function StripTagBody(tags, next) {
      if (typeof next !== 'function') {
        next = function () {};
      }

      var isRemoveAllTag = !Array.isArray(tags);
      function isRemoveTag(tag) {
        if (isRemoveAllTag) return true;
        return _.indexOf(tags, tag) !== -1;
      }

      var removeList = []; // 要删除的位置范围列表
      var posStart = false; // 当前标签开始位置

      return {
        onIgnoreTag: function onIgnoreTag(tag, html, options) {
          if (isRemoveTag(tag)) {
            if (options.isClosing) {
              var ret = '[/removed]';
              var end = options.position + ret.length;
              removeList.push([posStart !== false ? posStart : options.position, end]);
              posStart = false;
              return ret;
            } else {
              if (!posStart) {
                posStart = options.position;
              }
              return '[removed]';
            }
          } else {
            return next(tag, html, options);
          }
        },
        remove: function remove(html) {
          var rethtml = '';
          var lastPos = 0;
          _.forEach(removeList, function (pos) {
            rethtml += html.slice(lastPos, pos[0]);
            lastPos = pos[1];
          });
          rethtml += html.slice(lastPos);
          return rethtml;
        }
      };
    }

    /**
     * 去除备注标签
     *
     * @param {String} html
     * @return {String}
     */
    function stripCommentTag(html) {
      return html.replace(STRIP_COMMENT_TAG_REGEXP, '');
    }
    var STRIP_COMMENT_TAG_REGEXP = /<!--[\s\S]*?-->/g;

    /**
     * 去除不可见字符
     *
     * @param {String} html
     * @return {String}
     */
    function stripBlankChar(html) {
      var chars = html.split('');
      chars = chars.filter(function (char) {
        var c = char.charCodeAt(0);
        if (c === 127) return false;
        if (c <= 31) {
          if (c === 10 || c === 13) return true;
          return false;
        }
        return true;
      });
      return chars.join('');
    }

    exports.whiteList = whiteList;
    exports.onTag = onTag;
    exports.onIgnoreTag = onIgnoreTag;
    exports.onTagAttr = onTagAttr;
    exports.onIgnoreTagAttr = onIgnoreTagAttr;
    exports.safeAttrValue = safeAttrValue;
    exports.escapeHtml = escapeHtml;
    exports.escapeQuote = escapeQuote;
    exports.unescapeQuote = unescapeQuote;
    exports.escapeHtmlEntities = escapeHtmlEntities;
    exports.escapeDangerHtml5Entities = escapeDangerHtml5Entities;
    exports.clearNonPrintableCharacter = clearNonPrintableCharacter;
    exports.friendlyAttrValue = friendlyAttrValue;
    exports.escapeAttrValue = escapeAttrValue;
    exports.onIgnoreTagStripAll = onIgnoreTagStripAll;
    exports.StripTagBody = StripTagBody;
    exports.stripCommentTag = stripCommentTag;
    exports.stripBlankChar = stripBlankChar;
    exports.cssFilter = defaultCSSFilter;
  }, { "./util": 4, "cssfilter": 8 }], 2: [function (_require, module, exports) {
    /**
     * 模块入口
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var parser = _require('./parser');
    var FilterXSS = _require('./xss');

    /**
     * XSS过滤
     *
     * @param {String} html 要过滤的HTML代码
     * @param {Object} options 选项：whiteList, onTag, onTagAttr, onIgnoreTag, onIgnoreTagAttr, safeAttrValue, escapeHtml
     * @return {String}
     */
    function filterXSS(html, options) {
      var xss = new FilterXSS(options);
      return xss.process(html);
    }

    // 输出
    exports = module.exports = filterXSS;
    exports.FilterXSS = FilterXSS;
    for (var i in DEFAULT) exports[i] = DEFAULT[i];
    for (var i in parser) exports[i] = parser[i];

    // 在AMD下使用
    if (typeof define === 'function' && define.amd) {
      define(function () {
        return module.exports;
      });
    }

    // 在浏览器端使用
    if (typeof window !== 'undefined') {
      window.filterXSS = module.exports;
    }
  }, { "./default": 1, "./parser": 3, "./xss": 5 }], 3: [function (_require, module, exports) {
    /**
     * 简单 HTML Parser
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var _ = _require('./util');

    /**
     * 获取标签的名称
     *
     * @param {String} html 如：'<a hef="#">'
     * @return {String}
     */
    function getTagName(html) {
      var i = html.indexOf(' ');
      if (i === -1) {
        var tagName = html.slice(1, -1);
      } else {
        var tagName = html.slice(1, i + 1);
      }
      tagName = _.trim(tagName).toLowerCase();
      if (tagName.slice(0, 1) === '/') tagName = tagName.slice(1);
      if (tagName.slice(-1) === '/') tagName = tagName.slice(0, -1);
      return tagName;
    }

    /**
     * 是否为闭合标签
     *
     * @param {String} html 如：'<a hef="#">'
     * @return {Boolean}
     */
    function isClosing(html) {
      return html.slice(0, 2) === '</';
    }

    /**
     * 分析HTML代码，调用相应的函数处理，返回处理后的HTML
     *
     * @param {String} html
     * @param {Function} onTag 处理标签的函数
     *   参数格式： function (sourcePosition, position, tag, html, isClosing)
     * @param {Function} escapeHtml 对HTML进行转义的函数
     * @return {String}
     */
    function parseTag(html, onTag, escapeHtml) {
      'user strict';

      var rethtml = ''; // 待返回的HTML
      var lastPos = 0; // 上一个标签结束位置
      var tagStart = false; // 当前标签开始位置
      var quoteStart = false; // 引号开始位置
      var currentPos = 0; // 当前位置
      var len = html.length; // HTML长度
      var currentHtml = ''; // 当前标签的HTML代码
      var currentTagName = ''; // 当前标签的名称

      // 逐个分析字符
      for (currentPos = 0; currentPos < len; currentPos++) {
        var c = html.charAt(currentPos);
        if (tagStart === false) {
          if (c === '<') {
            tagStart = currentPos;
            continue;
          }
        } else {
          if (quoteStart === false) {
            if (c === '<') {
              rethtml += escapeHtml(html.slice(lastPos, currentPos));
              tagStart = currentPos;
              lastPos = currentPos;
              continue;
            }
            if (c === '>') {
              rethtml += escapeHtml(html.slice(lastPos, tagStart));
              currentHtml = html.slice(tagStart, currentPos + 1);
              currentTagName = getTagName(currentHtml);
              rethtml += onTag(tagStart, rethtml.length, currentTagName, currentHtml, isClosing(currentHtml));
              lastPos = currentPos + 1;
              tagStart = false;
              continue;
            }
            // HTML标签内的引号仅当前一个字符是等于号时才有效
            if ((c === '"' || c === "'") && html.charAt(currentPos - 1) === '=') {
              quoteStart = c;
              continue;
            }
          } else {
            if (c === quoteStart) {
              quoteStart = false;
              continue;
            }
          }
        }
      }
      if (lastPos < html.length) {
        rethtml += escapeHtml(html.substr(lastPos));
      }

      return rethtml;
    }

    // 不符合属性名称规则的正则表达式
    var REGEXP_ATTR_NAME = /[^a-zA-Z0-9_:\.\-]/img;

    /**
     * 分析标签HTML代码，调用相应的函数处理，返回HTML
     *
     * @param {String} html 如标签'<a href="#" target="_blank">' 则为 'href="#" target="_blank"'
     * @param {Function} onAttr 处理属性值的函数
     *   函数格式： function (name, value)
     * @return {String}
     */
    function parseAttr(html, onAttr) {
      'user strict';

      var lastPos = 0; // 当前位置
      var retAttrs = []; // 待返回的属性列表
      var tmpName = false; // 临时属性名称
      var len = html.length; // HTML代码长度

      function addAttr(name, value) {
        name = _.trim(name);
        name = name.replace(REGEXP_ATTR_NAME, '').toLowerCase();
        if (name.length < 1) return;
        var ret = onAttr(name, value || '');
        if (ret) retAttrs.push(ret);
      };

      // 逐个分析字符
      for (var i = 0; i < len; i++) {
        var c = html.charAt(i);
        var v, j;
        if (tmpName === false && c === '=') {
          tmpName = html.slice(lastPos, i);
          lastPos = i + 1;
          continue;
        }
        if (tmpName !== false) {
          // HTML标签内的引号仅当前一个字符是等于号时才有效
          if (i === lastPos && (c === '"' || c === "'") && html.charAt(i - 1) === '=') {
            j = html.indexOf(c, i + 1);
            if (j === -1) {
              break;
            } else {
              v = _.trim(html.slice(lastPos + 1, j));
              addAttr(tmpName, v);
              tmpName = false;
              i = j;
              lastPos = i + 1;
              continue;
            }
          }
        }
        if (c === ' ') {
          if (tmpName === false) {
            j = findNextEqual(html, i);
            if (j === -1) {
              v = _.trim(html.slice(lastPos, i));
              addAttr(v);
              tmpName = false;
              lastPos = i + 1;
              continue;
            } else {
              i = j - 1;
              continue;
            }
          } else {
            j = findBeforeEqual(html, i - 1);
            if (j === -1) {
              v = _.trim(html.slice(lastPos, i));
              v = stripQuoteWrap(v);
              addAttr(tmpName, v);
              tmpName = false;
              lastPos = i + 1;
              continue;
            } else {
              continue;
            }
          }
        }
      }

      if (lastPos < html.length) {
        if (tmpName === false) {
          addAttr(html.slice(lastPos));
        } else {
          addAttr(tmpName, stripQuoteWrap(_.trim(html.slice(lastPos))));
        }
      }

      return _.trim(retAttrs.join(' '));
    }

    function findNextEqual(str, i) {
      for (; i < str.length; i++) {
        var c = str[i];
        if (c === ' ') continue;
        if (c === '=') return i;
        return -1;
      }
    }

    function findBeforeEqual(str, i) {
      for (; i > 0; i--) {
        var c = str[i];
        if (c === ' ') continue;
        if (c === '=') return i;
        return -1;
      }
    }

    function isQuoteWrapString(text) {
      if (text[0] === '"' && text[text.length - 1] === '"' || text[0] === '\'' && text[text.length - 1] === '\'') {
        return true;
      } else {
        return false;
      }
    };

    function stripQuoteWrap(text) {
      if (isQuoteWrapString(text)) {
        return text.substr(1, text.length - 2);
      } else {
        return text;
      }
    };

    exports.parseTag = parseTag;
    exports.parseAttr = parseAttr;
  }, { "./util": 4 }], 4: [function (_require, module, exports) {
    module.exports = {
      indexOf: function indexOf(arr, item) {
        var i, j;
        if (Array.prototype.indexOf) {
          return arr.indexOf(item);
        }
        for (i = 0, j = arr.length; i < j; i++) {
          if (arr[i] === item) {
            return i;
          }
        }
        return -1;
      },
      forEach: function forEach(arr, fn, scope) {
        var i, j;
        if (Array.prototype.forEach) {
          return arr.forEach(fn, scope);
        }
        for (i = 0, j = arr.length; i < j; i++) {
          fn.call(scope, arr[i], i, arr);
        }
      },
      trim: function trim(str) {
        if (String.prototype.trim) {
          return str.trim();
        }
        return str.replace(/(^\s*)|(\s*$)/g, '');
      }
    };
  }, {}], 5: [function (_require, module, exports) {
    /**
     * 过滤XSS
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var FilterCSS = _require('cssfilter').FilterCSS;
    var DEFAULT = _require('./default');
    var parser = _require('./parser');
    var parseTag = parser.parseTag;
    var parseAttr = parser.parseAttr;
    var _ = _require('./util');

    /**
     * 返回值是否为空
     *
     * @param {Object} obj
     * @return {Boolean}
     */
    function isNull(obj) {
      return obj === undefined || obj === null;
    }

    /**
     * 取标签内的属性列表字符串
     *
     * @param {String} html
     * @return {Object}
     *   - {String} html
     *   - {Boolean} closing
     */
    function getAttrs(html) {
      var i = html.indexOf(' ');
      if (i === -1) {
        return {
          html: '',
          closing: html[html.length - 2] === '/'
        };
      }
      html = _.trim(html.slice(i + 1, -1));
      var isClosing = html[html.length - 1] === '/';
      if (isClosing) html = _.trim(html.slice(0, -1));
      return {
        html: html,
        closing: isClosing
      };
    }

    /**
     * XSS过滤对象
     *
     * @param {Object} options
     *   选项：whiteList, onTag, onTagAttr, onIgnoreTag,
     *        onIgnoreTagAttr, safeAttrValue, escapeHtml
     *        stripIgnoreTagBody, allowCommentTag, stripBlankChar
     *        css{whiteList, onAttr, onIgnoreAttr}
     */
    function FilterXSS(options) {
      options = options || {};

      if (options.stripIgnoreTag) {
        if (options.onIgnoreTag) {
          console.error('Notes: cannot use these two options "stripIgnoreTag" and "onIgnoreTag" at the same time');
        }
        options.onIgnoreTag = DEFAULT.onIgnoreTagStripAll;
      }

      options.whiteList = options.whiteList || DEFAULT.whiteList;
      options.onTag = options.onTag || DEFAULT.onTag;
      options.onTagAttr = options.onTagAttr || DEFAULT.onTagAttr;
      options.onIgnoreTag = options.onIgnoreTag || DEFAULT.onIgnoreTag;
      options.onIgnoreTagAttr = options.onIgnoreTagAttr || DEFAULT.onIgnoreTagAttr;
      options.safeAttrValue = options.safeAttrValue || DEFAULT.safeAttrValue;
      options.escapeHtml = options.escapeHtml || DEFAULT.escapeHtml;
      options.css = options.css || {};
      this.options = options;

      this.cssFilter = new FilterCSS(options.css);
    }

    /**
     * 开始处理
     *
     * @param {String} html
     * @return {String}
     */
    FilterXSS.prototype.process = function (html) {
      // 兼容各种奇葩输入
      html = html || '';
      html = html.toString();
      if (!html) return '';

      var me = this;
      var options = me.options;
      var whiteList = options.whiteList;
      var onTag = options.onTag;
      var onIgnoreTag = options.onIgnoreTag;
      var onTagAttr = options.onTagAttr;
      var onIgnoreTagAttr = options.onIgnoreTagAttr;
      var safeAttrValue = options.safeAttrValue;
      var escapeHtml = options.escapeHtml;
      var cssFilter = me.cssFilter;

      // 是否清除不可见字符
      if (options.stripBlankChar) {
        html = DEFAULT.stripBlankChar(html);
      }

      // 是否禁止备注标签
      if (!options.allowCommentTag) {
        html = DEFAULT.stripCommentTag(html);
      }

      // 如果开启了stripIgnoreTagBody
      if (options.stripIgnoreTagBody) {
        var stripIgnoreTagBody = DEFAULT.StripTagBody(options.stripIgnoreTagBody, onIgnoreTag);
        onIgnoreTag = stripIgnoreTagBody.onIgnoreTag;
      } else {
        stripIgnoreTagBody = false;
      }

      var retHtml = parseTag(html, function (sourcePosition, position, tag, html, isClosing) {
        var info = {
          sourcePosition: sourcePosition,
          position: position,
          isClosing: isClosing,
          isWhite: tag in whiteList
        };

        // 调用onTag处理
        var ret = onTag(tag, html, info);
        if (!isNull(ret)) return ret;

        // 默认标签处理方法
        if (info.isWhite) {
          // 白名单标签，解析标签属性
          // 如果是闭合标签，则不需要解析属性
          if (info.isClosing) {
            return '</' + tag + '>';
          }

          var attrs = getAttrs(html);
          var whiteAttrList = whiteList[tag];
          var attrsHtml = parseAttr(attrs.html, function (name, value) {

            // 调用onTagAttr处理
            var isWhiteAttr = _.indexOf(whiteAttrList, name) !== -1;
            var ret = onTagAttr(tag, name, value, isWhiteAttr);
            if (!isNull(ret)) return ret;

            // 默认的属性处理方法
            if (isWhiteAttr) {
              // 白名单属性，调用safeAttrValue过滤属性值
              value = safeAttrValue(tag, name, value, cssFilter);
              if (value) {
                return name + '="' + value + '"';
              } else {
                return name;
              }
            } else {
              // 非白名单属性，调用onIgnoreTagAttr处理
              var ret = onIgnoreTagAttr(tag, name, value, isWhiteAttr);
              if (!isNull(ret)) return ret;
              return;
            }
          });

          // 构造新的标签代码
          var html = '<' + tag;
          if (attrsHtml) html += ' ' + attrsHtml;
          if (attrs.closing) html += ' /';
          html += '>';
          return html;
        } else {
          // 非白名单标签，调用onIgnoreTag处理
          var ret = onIgnoreTag(tag, html, info);
          if (!isNull(ret)) return ret;
          return escapeHtml(html);
        }
      }, escapeHtml);

      // 如果开启了stripIgnoreTagBody，需要对结果再进行处理
      if (stripIgnoreTagBody) {
        retHtml = stripIgnoreTagBody.remove(retHtml);
      }

      return retHtml;
    };

    module.exports = FilterXSS;
  }, { "./default": 1, "./parser": 3, "./util": 4, "cssfilter": 8 }], 6: [function (_require, module, exports) {
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var parseStyle = _require('./parser');
    var _ = _require('./util');

    /**
     * 返回值是否为空
     *
     * @param {Object} obj
     * @return {Boolean}
     */
    function isNull(obj) {
      return obj === undefined || obj === null;
    }

    /**
     * 创建CSS过滤器
     *
     * @param {Object} options
     *   - {Object} whiteList
     *   - {Object} onAttr
     *   - {Object} onIgnoreAttr
     */
    function FilterCSS(options) {
      options = options || {};
      options.whiteList = options.whiteList || DEFAULT.whiteList;
      options.onAttr = options.onAttr || DEFAULT.onAttr;
      options.onIgnoreAttr = options.onIgnoreAttr || DEFAULT.onIgnoreAttr;
      this.options = options;
    }

    FilterCSS.prototype.process = function (css) {
      // 兼容各种奇葩输入
      css = css || '';
      css = css.toString();
      if (!css) return '';

      var me = this;
      var options = me.options;
      var whiteList = options.whiteList;
      var onAttr = options.onAttr;
      var onIgnoreAttr = options.onIgnoreAttr;

      var retCSS = parseStyle(css, function (sourcePosition, position, name, value, source) {

        var check = whiteList[name];
        var isWhite = false;
        if (check === true) isWhite = check;else if (typeof check === 'function') isWhite = check(value);else if (check instanceof RegExp) isWhite = check.test(value);
        if (isWhite !== true) isWhite = false;

        var opts = {
          position: position,
          sourcePosition: sourcePosition,
          source: source,
          isWhite: isWhite
        };

        if (isWhite) {

          var ret = onAttr(name, value, opts);
          if (isNull(ret)) {
            return name + ':' + value;
          } else {
            return ret;
          }
        } else {

          var ret = onIgnoreAttr(name, value, opts);
          if (!isNull(ret)) {
            return ret;
          }
        }
      });

      return retCSS;
    };

    module.exports = FilterCSS;
  }, { "./default": 7, "./parser": 9, "./util": 10 }], 7: [function (_require, module, exports) {
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    // 白名单值说明：
    // true: 允许该属性
    // Function: function (val) { } 返回true表示允许该属性，其他值均表示不允许
    // RegExp: regexp.test(val) 返回true表示允许该属性，其他值均表示不允许
    // 除上面列出的值外均表示不允许
    var whiteList = {};
    whiteList['align-content'] = false; // default: auto
    whiteList['align-items'] = false; // default: auto
    whiteList['align-self'] = false; // default: auto
    whiteList['alignment-adjust'] = false; // default: auto
    whiteList['alignment-baseline'] = false; // default: baseline
    whiteList['all'] = false; // default: depending on individual properties
    whiteList['anchor-point'] = false; // default: none
    whiteList['animation'] = false; // default: depending on individual properties
    whiteList['animation-delay'] = false; // default: 0
    whiteList['animation-direction'] = false; // default: normal
    whiteList['animation-duration'] = false; // default: 0
    whiteList['animation-fill-mode'] = false; // default: none
    whiteList['animation-iteration-count'] = false; // default: 1
    whiteList['animation-name'] = false; // default: none
    whiteList['animation-play-state'] = false; // default: running
    whiteList['animation-timing-function'] = false; // default: ease
    whiteList['azimuth'] = false; // default: center
    whiteList['backface-visibility'] = false; // default: visible
    whiteList['background'] = true; // default: depending on individual properties
    whiteList['background-attachment'] = true; // default: scroll
    whiteList['background-clip'] = true; // default: border-box
    whiteList['background-color'] = true; // default: transparent
    whiteList['background-image'] = true; // default: none
    whiteList['background-origin'] = true; // default: padding-box
    whiteList['background-position'] = true; // default: 0% 0%
    whiteList['background-repeat'] = true; // default: repeat
    whiteList['background-size'] = true; // default: auto
    whiteList['baseline-shift'] = false; // default: baseline
    whiteList['binding'] = false; // default: none
    whiteList['bleed'] = false; // default: 6pt
    whiteList['bookmark-label'] = false; // default: content()
    whiteList['bookmark-level'] = false; // default: none
    whiteList['bookmark-state'] = false; // default: open
    whiteList['border'] = true; // default: depending on individual properties
    whiteList['border-bottom'] = true; // default: depending on individual properties
    whiteList['border-bottom-color'] = true; // default: current color
    whiteList['border-bottom-left-radius'] = true; // default: 0
    whiteList['border-bottom-right-radius'] = true; // default: 0
    whiteList['border-bottom-style'] = true; // default: none
    whiteList['border-bottom-width'] = true; // default: medium
    whiteList['border-collapse'] = true; // default: separate
    whiteList['border-color'] = true; // default: depending on individual properties
    whiteList['border-image'] = true; // default: none
    whiteList['border-image-outset'] = true; // default: 0
    whiteList['border-image-repeat'] = true; // default: stretch
    whiteList['border-image-slice'] = true; // default: 100%
    whiteList['border-image-source'] = true; // default: none
    whiteList['border-image-width'] = true; // default: 1
    whiteList['border-left'] = true; // default: depending on individual properties
    whiteList['border-left-color'] = true; // default: current color
    whiteList['border-left-style'] = true; // default: none
    whiteList['border-left-width'] = true; // default: medium
    whiteList['border-radius'] = true; // default: 0
    whiteList['border-right'] = true; // default: depending on individual properties
    whiteList['border-right-color'] = true; // default: current color
    whiteList['border-right-style'] = true; // default: none
    whiteList['border-right-width'] = true; // default: medium
    whiteList['border-spacing'] = true; // default: 0
    whiteList['border-style'] = true; // default: depending on individual properties
    whiteList['border-top'] = true; // default: depending on individual properties
    whiteList['border-top-color'] = true; // default: current color
    whiteList['border-top-left-radius'] = true; // default: 0
    whiteList['border-top-right-radius'] = true; // default: 0
    whiteList['border-top-style'] = true; // default: none
    whiteList['border-top-width'] = true; // default: medium
    whiteList['border-width'] = true; // default: depending on individual properties
    whiteList['bottom'] = false; // default: auto
    whiteList['box-decoration-break'] = true; // default: slice
    whiteList['box-shadow'] = true; // default: none
    whiteList['box-sizing'] = true; // default: content-box
    whiteList['box-snap'] = true; // default: none
    whiteList['box-suppress'] = true; // default: show
    whiteList['break-after'] = true; // default: auto
    whiteList['break-before'] = true; // default: auto
    whiteList['break-inside'] = true; // default: auto
    whiteList['caption-side'] = false; // default: top
    whiteList['chains'] = false; // default: none
    whiteList['clear'] = true; // default: none
    whiteList['clip'] = false; // default: auto
    whiteList['clip-path'] = false; // default: none
    whiteList['clip-rule'] = false; // default: nonzero
    whiteList['color'] = true; // default: implementation dependent
    whiteList['color-interpolation-filters'] = true; // default: auto
    whiteList['column-count'] = false; // default: auto
    whiteList['column-fill'] = false; // default: balance
    whiteList['column-gap'] = false; // default: normal
    whiteList['column-rule'] = false; // default: depending on individual properties
    whiteList['column-rule-color'] = false; // default: current color
    whiteList['column-rule-style'] = false; // default: medium
    whiteList['column-rule-width'] = false; // default: medium
    whiteList['column-span'] = false; // default: none
    whiteList['column-width'] = false; // default: auto
    whiteList['columns'] = false; // default: depending on individual properties
    whiteList['contain'] = false; // default: none
    whiteList['content'] = false; // default: normal
    whiteList['counter-increment'] = false; // default: none
    whiteList['counter-reset'] = false; // default: none
    whiteList['counter-set'] = false; // default: none
    whiteList['crop'] = false; // default: auto
    whiteList['cue'] = false; // default: depending on individual properties
    whiteList['cue-after'] = false; // default: none
    whiteList['cue-before'] = false; // default: none
    whiteList['cursor'] = false; // default: auto
    whiteList['direction'] = false; // default: ltr
    whiteList['display'] = true; // default: depending on individual properties
    whiteList['display-inside'] = true; // default: auto
    whiteList['display-list'] = true; // default: none
    whiteList['display-outside'] = true; // default: inline-level
    whiteList['dominant-baseline'] = false; // default: auto
    whiteList['elevation'] = false; // default: level
    whiteList['empty-cells'] = false; // default: show
    whiteList['filter'] = false; // default: none
    whiteList['flex'] = false; // default: depending on individual properties
    whiteList['flex-basis'] = false; // default: auto
    whiteList['flex-direction'] = false; // default: row
    whiteList['flex-flow'] = false; // default: depending on individual properties
    whiteList['flex-grow'] = false; // default: 0
    whiteList['flex-shrink'] = false; // default: 1
    whiteList['flex-wrap'] = false; // default: nowrap
    whiteList['float'] = false; // default: none
    whiteList['float-offset'] = false; // default: 0 0
    whiteList['flood-color'] = false; // default: black
    whiteList['flood-opacity'] = false; // default: 1
    whiteList['flow-from'] = false; // default: none
    whiteList['flow-into'] = false; // default: none
    whiteList['font'] = true; // default: depending on individual properties
    whiteList['font-family'] = true; // default: implementation dependent
    whiteList['font-feature-settings'] = true; // default: normal
    whiteList['font-kerning'] = true; // default: auto
    whiteList['font-language-override'] = true; // default: normal
    whiteList['font-size'] = true; // default: medium
    whiteList['font-size-adjust'] = true; // default: none
    whiteList['font-stretch'] = true; // default: normal
    whiteList['font-style'] = true; // default: normal
    whiteList['font-synthesis'] = true; // default: weight style
    whiteList['font-variant'] = true; // default: normal
    whiteList['font-variant-alternates'] = true; // default: normal
    whiteList['font-variant-caps'] = true; // default: normal
    whiteList['font-variant-east-asian'] = true; // default: normal
    whiteList['font-variant-ligatures'] = true; // default: normal
    whiteList['font-variant-numeric'] = true; // default: normal
    whiteList['font-variant-position'] = true; // default: normal
    whiteList['font-weight'] = true; // default: normal
    whiteList['grid'] = false; // default: depending on individual properties
    whiteList['grid-area'] = false; // default: depending on individual properties
    whiteList['grid-auto-columns'] = false; // default: auto
    whiteList['grid-auto-flow'] = false; // default: none
    whiteList['grid-auto-rows'] = false; // default: auto
    whiteList['grid-column'] = false; // default: depending on individual properties
    whiteList['grid-column-end'] = false; // default: auto
    whiteList['grid-column-start'] = false; // default: auto
    whiteList['grid-row'] = false; // default: depending on individual properties
    whiteList['grid-row-end'] = false; // default: auto
    whiteList['grid-row-start'] = false; // default: auto
    whiteList['grid-template'] = false; // default: depending on individual properties
    whiteList['grid-template-areas'] = false; // default: none
    whiteList['grid-template-columns'] = false; // default: none
    whiteList['grid-template-rows'] = false; // default: none
    whiteList['hanging-punctuation'] = false; // default: none
    whiteList['height'] = true; // default: auto
    whiteList['hyphens'] = false; // default: manual
    whiteList['icon'] = false; // default: auto
    whiteList['image-orientation'] = false; // default: auto
    whiteList['image-resolution'] = false; // default: normal
    whiteList['ime-mode'] = false; // default: auto
    whiteList['initial-letters'] = false; // default: normal
    whiteList['inline-box-align'] = false; // default: last
    whiteList['justify-content'] = false; // default: auto
    whiteList['justify-items'] = false; // default: auto
    whiteList['justify-self'] = false; // default: auto
    whiteList['left'] = false; // default: auto
    whiteList['letter-spacing'] = true; // default: normal
    whiteList['lighting-color'] = true; // default: white
    whiteList['line-box-contain'] = false; // default: block inline replaced
    whiteList['line-break'] = false; // default: auto
    whiteList['line-grid'] = false; // default: match-parent
    whiteList['line-height'] = false; // default: normal
    whiteList['line-snap'] = false; // default: none
    whiteList['line-stacking'] = false; // default: depending on individual properties
    whiteList['line-stacking-ruby'] = false; // default: exclude-ruby
    whiteList['line-stacking-shift'] = false; // default: consider-shifts
    whiteList['line-stacking-strategy'] = false; // default: inline-line-height
    whiteList['list-style'] = true; // default: depending on individual properties
    whiteList['list-style-image'] = true; // default: none
    whiteList['list-style-position'] = true; // default: outside
    whiteList['list-style-type'] = true; // default: disc
    whiteList['margin'] = true; // default: depending on individual properties
    whiteList['margin-bottom'] = true; // default: 0
    whiteList['margin-left'] = true; // default: 0
    whiteList['margin-right'] = true; // default: 0
    whiteList['margin-top'] = true; // default: 0
    whiteList['marker-offset'] = false; // default: auto
    whiteList['marker-side'] = false; // default: list-item
    whiteList['marks'] = false; // default: none
    whiteList['mask'] = false; // default: border-box
    whiteList['mask-box'] = false; // default: see individual properties
    whiteList['mask-box-outset'] = false; // default: 0
    whiteList['mask-box-repeat'] = false; // default: stretch
    whiteList['mask-box-slice'] = false; // default: 0 fill
    whiteList['mask-box-source'] = false; // default: none
    whiteList['mask-box-width'] = false; // default: auto
    whiteList['mask-clip'] = false; // default: border-box
    whiteList['mask-image'] = false; // default: none
    whiteList['mask-origin'] = false; // default: border-box
    whiteList['mask-position'] = false; // default: center
    whiteList['mask-repeat'] = false; // default: no-repeat
    whiteList['mask-size'] = false; // default: border-box
    whiteList['mask-source-type'] = false; // default: auto
    whiteList['mask-type'] = false; // default: luminance
    whiteList['max-height'] = true; // default: none
    whiteList['max-lines'] = false; // default: none
    whiteList['max-width'] = true; // default: none
    whiteList['min-height'] = true; // default: 0
    whiteList['min-width'] = true; // default: 0
    whiteList['move-to'] = false; // default: normal
    whiteList['nav-down'] = false; // default: auto
    whiteList['nav-index'] = false; // default: auto
    whiteList['nav-left'] = false; // default: auto
    whiteList['nav-right'] = false; // default: auto
    whiteList['nav-up'] = false; // default: auto
    whiteList['object-fit'] = false; // default: fill
    whiteList['object-position'] = false; // default: 50% 50%
    whiteList['opacity'] = false; // default: 1
    whiteList['order'] = false; // default: 0
    whiteList['orphans'] = false; // default: 2
    whiteList['outline'] = false; // default: depending on individual properties
    whiteList['outline-color'] = false; // default: invert
    whiteList['outline-offset'] = false; // default: 0
    whiteList['outline-style'] = false; // default: none
    whiteList['outline-width'] = false; // default: medium
    whiteList['overflow'] = false; // default: depending on individual properties
    whiteList['overflow-wrap'] = false; // default: normal
    whiteList['overflow-x'] = false; // default: visible
    whiteList['overflow-y'] = false; // default: visible
    whiteList['padding'] = true; // default: depending on individual properties
    whiteList['padding-bottom'] = true; // default: 0
    whiteList['padding-left'] = true; // default: 0
    whiteList['padding-right'] = true; // default: 0
    whiteList['padding-top'] = true; // default: 0
    whiteList['page'] = false; // default: auto
    whiteList['page-break-after'] = false; // default: auto
    whiteList['page-break-before'] = false; // default: auto
    whiteList['page-break-inside'] = false; // default: auto
    whiteList['page-policy'] = false; // default: start
    whiteList['pause'] = false; // default: implementation dependent
    whiteList['pause-after'] = false; // default: implementation dependent
    whiteList['pause-before'] = false; // default: implementation dependent
    whiteList['perspective'] = false; // default: none
    whiteList['perspective-origin'] = false; // default: 50% 50%
    whiteList['pitch'] = false; // default: medium
    whiteList['pitch-range'] = false; // default: 50
    whiteList['play-during'] = false; // default: auto
    whiteList['position'] = false; // default: static
    whiteList['presentation-level'] = false; // default: 0
    whiteList['quotes'] = false; // default: text
    whiteList['region-fragment'] = false; // default: auto
    whiteList['resize'] = false; // default: none
    whiteList['rest'] = false; // default: depending on individual properties
    whiteList['rest-after'] = false; // default: none
    whiteList['rest-before'] = false; // default: none
    whiteList['richness'] = false; // default: 50
    whiteList['right'] = false; // default: auto
    whiteList['rotation'] = false; // default: 0
    whiteList['rotation-point'] = false; // default: 50% 50%
    whiteList['ruby-align'] = false; // default: auto
    whiteList['ruby-merge'] = false; // default: separate
    whiteList['ruby-position'] = false; // default: before
    whiteList['shape-image-threshold'] = false; // default: 0.0
    whiteList['shape-outside'] = false; // default: none
    whiteList['shape-margin'] = false; // default: 0
    whiteList['size'] = false; // default: auto
    whiteList['speak'] = false; // default: auto
    whiteList['speak-as'] = false; // default: normal
    whiteList['speak-header'] = false; // default: once
    whiteList['speak-numeral'] = false; // default: continuous
    whiteList['speak-punctuation'] = false; // default: none
    whiteList['speech-rate'] = false; // default: medium
    whiteList['stress'] = false; // default: 50
    whiteList['string-set'] = false; // default: none
    whiteList['tab-size'] = false; // default: 8
    whiteList['table-layout'] = false; // default: auto
    whiteList['text-align'] = true; // default: start
    whiteList['text-align-last'] = true; // default: auto
    whiteList['text-combine-upright'] = true; // default: none
    whiteList['text-decoration'] = true; // default: none
    whiteList['text-decoration-color'] = true; // default: currentColor
    whiteList['text-decoration-line'] = true; // default: none
    whiteList['text-decoration-skip'] = true; // default: objects
    whiteList['text-decoration-style'] = true; // default: solid
    whiteList['text-emphasis'] = true; // default: depending on individual properties
    whiteList['text-emphasis-color'] = true; // default: currentColor
    whiteList['text-emphasis-position'] = true; // default: over right
    whiteList['text-emphasis-style'] = true; // default: none
    whiteList['text-height'] = true; // default: auto
    whiteList['text-indent'] = true; // default: 0
    whiteList['text-justify'] = true; // default: auto
    whiteList['text-orientation'] = true; // default: mixed
    whiteList['text-overflow'] = true; // default: clip
    whiteList['text-shadow'] = true; // default: none
    whiteList['text-space-collapse'] = true; // default: collapse
    whiteList['text-transform'] = true; // default: none
    whiteList['text-underline-position'] = true; // default: auto
    whiteList['text-wrap'] = true; // default: normal
    whiteList['top'] = false; // default: auto
    whiteList['transform'] = false; // default: none
    whiteList['transform-origin'] = false; // default: 50% 50% 0
    whiteList['transform-style'] = false; // default: flat
    whiteList['transition'] = false; // default: depending on individual properties
    whiteList['transition-delay'] = false; // default: 0s
    whiteList['transition-duration'] = false; // default: 0s
    whiteList['transition-property'] = false; // default: all
    whiteList['transition-timing-function'] = false; // default: ease
    whiteList['unicode-bidi'] = false; // default: normal
    whiteList['vertical-align'] = false; // default: baseline
    whiteList['visibility'] = false; // default: visible
    whiteList['voice-balance'] = false; // default: center
    whiteList['voice-duration'] = false; // default: auto
    whiteList['voice-family'] = false; // default: implementation dependent
    whiteList['voice-pitch'] = false; // default: medium
    whiteList['voice-range'] = false; // default: medium
    whiteList['voice-rate'] = false; // default: normal
    whiteList['voice-stress'] = false; // default: normal
    whiteList['voice-volume'] = false; // default: medium
    whiteList['volume'] = false; // default: medium
    whiteList['white-space'] = false; // default: normal
    whiteList['widows'] = false; // default: 2
    whiteList['width'] = true; // default: auto
    whiteList['will-change'] = false; // default: auto
    whiteList['word-break'] = true; // default: normal
    whiteList['word-spacing'] = true; // default: normal
    whiteList['word-wrap'] = true; // default: normal
    whiteList['wrap-flow'] = false; // default: auto
    whiteList['wrap-through'] = false; // default: wrap
    whiteList['writing-mode'] = false; // default: horizontal-tb
    whiteList['z-index'] = false; // default: auto

    /**
     * 匹配到白名单上的一个属性时
     *
     * @param {String} name
     * @param {String} value
     * @param {Object} options
     * @return {String}
     */
    function onAttr(name, value, options) {}
    // do nothing

    /**
     * 匹配到不在白名单上的一个属性时
     *
     * @param {String} name
     * @param {String} value
     * @param {Object} options
     * @return {String}
     */
    function onIgnoreAttr(name, value, options) {
      // do nothing
    }

    exports.whiteList = whiteList;
    exports.onAttr = onAttr;
    exports.onIgnoreAttr = onIgnoreAttr;
  }, {}], 8: [function (_require, module, exports) {
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var FilterCSS = _require('./css');

    /**
     * XSS过滤
     *
     * @param {String} css 要过滤的CSS代码
     * @param {Object} options 选项：whiteList, onAttr, onIgnoreAttr
     * @return {String}
     */
    function filterCSS(html, options) {
      var xss = new FilterCSS(options);
      return xss.process(html);
    }

    // 输出
    exports = module.exports = filterCSS;
    exports.FilterCSS = FilterCSS;
    for (var i in DEFAULT) exports[i] = DEFAULT[i];

    // 在AMD下使用
    if (typeof define === 'function' && define.amd) {
      define(function () {
        return module.exports;
      });
    }

    // 在浏览器端使用
    if (typeof window !== 'undefined') {
      window.filterCSS = module.exports;
    }
  }, { "./css": 6, "./default": 7 }], 9: [function (_require, module, exports) {
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var _ = _require('./util');

    /**
     * 解析style
     *
     * @param {String} css
     * @param {Function} onAttr 处理属性的函数
     *   参数格式： function (sourcePosition, position, name, value, source)
     * @return {String}
     */
    function parseStyle(css, onAttr) {
      css = _.trimRight(css);
      if (css[css.length - 1] !== ';') css += ';';
      var cssLength = css.length;
      var isParenthesisOpen = false;
      var lastPos = 0;
      var i = 0;
      var retCSS = '';

      function addNewAttr() {
        // 如果没有正常的闭合圆括号，则直接忽略当前属性
        if (!isParenthesisOpen) {
          var source = _.trim(css.slice(lastPos, i));
          var j = source.indexOf(':');
          if (j !== -1) {
            var name = _.trim(source.slice(0, j));
            var value = _.trim(source.slice(j + 1));
            // 必须有属性名称
            if (name) {
              var ret = onAttr(lastPos, retCSS.length, name, value, source);
              if (ret) retCSS += ret + '; ';
            }
          }
        }
        lastPos = i + 1;
      }

      for (; i < cssLength; i++) {
        var c = css[i];
        if (c === '/' && css[i + 1] === '*') {
          // 备注开始
          var j = css.indexOf('*/', i + 2);
          // 如果没有正常的备注结束，则后面的部分全部跳过
          if (j === -1) break;
          // 直接将当前位置调到备注结尾，并且初始化状态
          i = j + 1;
          lastPos = i + 1;
          isParenthesisOpen = false;
        } else if (c === '(') {
          isParenthesisOpen = true;
        } else if (c === ')') {
          isParenthesisOpen = false;
        } else if (c === ';') {
          if (isParenthesisOpen) {
            // 在圆括号里面，忽略
          } else {
              addNewAttr();
            }
        } else if (c === '\n') {
          addNewAttr();
        }
      }

      return _.trim(retCSS);
    }

    module.exports = parseStyle;
  }, { "./util": 10 }], 10: [function (_require, module, exports) {
    module.exports = {
      indexOf: function indexOf(arr, item) {
        var i, j;
        if (Array.prototype.indexOf) {
          return arr.indexOf(item);
        }
        for (i = 0, j = arr.length; i < j; i++) {
          if (arr[i] === item) {
            return i;
          }
        }
        return -1;
      },
      forEach: function forEach(arr, fn, scope) {
        var i, j;
        if (Array.prototype.forEach) {
          return arr.forEach(fn, scope);
        }
        for (i = 0, j = arr.length; i < j; i++) {
          fn.call(scope, arr[i], i, arr);
        }
      },
      trim: function trim(str) {
        if (String.prototype.trim) {
          return str.trim();
        }
        return str.replace(/(^\s*)|(\s*$)/g, '');
      },
      trimRight: function trimRight(str) {
        if (String.prototype.trimRight) {
          return str.trimRight();
        }
        return str.replace(/(\s*$)/g, '');
      }
    };
  }, {}] }, {}, [2]);

exports["default"] = filterXSS;
module.exports = exports["default"];

},{}],14:[function(require,module,exports){
/**
 * Dom 操作工具包（基础核心包，主要都是 get 等读取操作）
 *
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var domUtils = {
    /**
     * 添加 class name
     * @param domList
     * @param className
     */
    addClass: function addClass(domList, className) {
        if (!domList) {
            return;
        }
        if (!!domList.nodeType) {
            domList = [domList];
        }
        var i, dom;
        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (dom.nodeType === 1 && !domUtils.hasClass(dom, className)) {
                dom.className = (dom.className + ' ' + className).trim();
            }
        }
    },
    /**
     * 给 DOM.getAttribute('style') 对象 设置 样式
     * @param styleStr
     * @param styleObj
     */
    appendStyle: function appendStyle(styleStr, styleObj) {
        if (!styleStr) {
            return;
        }
        var styleList = styleStr.split(';'),
            i,
            j,
            t;
        for (i = 0, j = styleList.length; i < j; i++) {
            if (styleList[i].indexOf(':') > 0) {
                t = styleList[i].split(':');
                styleObj[t[0].trim()] = t[1].trim();
            }
        }
    },
    /**
     * 修改子节点的属性（attribute）
     * @param dom
     * @param attr
     */
    attr: function attr(dom, _attr) {
        var key, value;
        if (!dom || !_attr || dom.nodeType !== 1) {
            return;
        }
        for (key in _attr) {
            if (_attr.hasOwnProperty(key) && typeof key == 'string') {
                value = _attr[key];
                if (!value) {
                    dom.removeAttribute(key);
                } else {
                    dom.setAttribute(key, value);
                }
            }
        }
    },
    /**
     * 判断 dom 是否为可编辑的 dom 类型
     * @param dom
     * @returns {*|boolean}
     */
    canEdit: function canEdit(dom) {
        //过滤 script、style等标签
        var filterTag = ['script', 'style'];

        return dom && (dom.nodeType == 1 || dom.nodeType == 3) && (domUtils.isTag(dom, 'br') || !domUtils.isEmptyDom(dom)) && !domUtils.getParentByTagName(dom, _commonConst2['default'].TAG.TMP_TAG, true, null) && !(dom.nodeType === 1 && domUtils.isTag(dom, filterTag) || dom.nodeType === 3 && dom.parentNode && domUtils.isTag(dom.parentNode, filterTag));
    },
    /**
     * 清理 dom 内无用的 childNodes（主要用于 处理 剪切板的 html）
     * @param dom
     */
    childNodesFilter: function childNodesFilter(dom) {
        if (!dom || dom.nodeType !== 1) {
            return;
        }
        var i, d;
        for (i = dom.childNodes.length - 1; i >= 0; i--) {
            d = dom.childNodes[i];
            if (d.nodeType == 1) {
                if (/link|style|script|meta/ig.test(d.nodeName)) {
                    dom.removeChild(d);
                }
                domUtils.childNodesFilter(d);
            } else if (d.nodeType != 3) {
                dom.removeChild(d);
            }
        }
    },
    /**
     * 清除 Dom 上 某一个 inline 的 style 属性
     * @param dom
     * @param styleKey
     */
    clearStyle: function clearStyle(dom, styleKey) {
        var parent;
        while (dom.getAttribute(_commonConst2['default'].ATTR.SPAN) === _commonConst2['default'].ATTR.SPAN) {
            dom.style[styleKey] = '';

            parent = dom.parentNode;
            if (parent.getAttribute(_commonConst2['default'].ATTR.SPAN) !== _commonConst2['default'].ATTR.SPAN) {
                break;
            }
            if (!dom.previousSibling && !dom.nextSibling) {
                dom = parent;
            } else if (!dom.previousSibling) {
                domUtils.insert(parent, dom, false);
                domUtils.mergeAtoB(parent, dom, false);
                dom.style[styleKey] = '';
            } else if (!dom.nextSibling) {
                domUtils.insert(parent, dom, true);
                domUtils.mergeAtoB(parent, dom, false);
                dom.style[styleKey] = '';
            } else {
                var nSpan = domUtils.createSpan(),
                    tmpDom;
                nSpan.setAttribute('style', parent.getAttribute('style'));
                while (dom.nextSibling) {
                    tmpDom = dom.nextSibling;
                    nSpan.insertBefore(tmpDom, null);
                    domUtils.mergeAtoB(parent, tmpDom, false);
                }
                domUtils.insert(parent, dom, true);
                domUtils.insert(dom, nSpan, true);
                domUtils.mergeAtoB(parent, dom, false);
                domUtils.mergeAtoB(parent, nSpan, false);
            }
        }
    },
    /**
     * 比较 IndexList
     * @param a
     * @param b
     * @returns {number}
     */
    compareIndexList: function compareIndexList(a, b) {
        var i,
            j = Math.min(a.length, b.length),
            x,
            y;
        for (i = 0; i < j; i++) {
            x = a[i];
            y = b[i];
            if (x < y) {
                return -1;
            }
            if (x > y) {
                return 1;
            }
        }

        if (a.length < b.length) {
            return -1;
        }

        if (a.length > b.length) {
            return 1;
        }

        return 0;
    },
    /**
     * a 是否包含 b （from jQuery 1.10.2）
     * @param a
     * @param b
     * @returns {boolean}
     */
    contains: function contains(a, b) {
        var adown = a.nodeType === 9 ? a.documentElement : a,
            bup = b && b.parentNode;
        return a === bup || !!(bup && bup.nodeType === 1 && (adown.contains ? adown.contains(bup) : a.compareDocumentPosition && a.compareDocumentPosition(bup) & 16));
    },
    /**
     * 创建 wiz编辑器 自用的 span
     */
    createSpan: function createSpan() {
        var s = _commonEnv2['default'].doc.createElement('span');
        s.setAttribute(_commonConst2['default'].ATTR.SPAN, _commonConst2['default'].ATTR.SPAN);
        return s;
    },
    /**
     * 设置 dom css
     * @param dom
     * @param style {{}}
     * @param onlyWizSpan
     */
    css: function css(dom, style, onlyWizSpan) {
        if (!dom || !style || domUtils.isTag(dom, 'br')) {
            //禁止给 br 添加任何样式
            return;
        }
        onlyWizSpan = !!onlyWizSpan;
        var k, v;
        for (k in style) {
            if (style.hasOwnProperty(k) && typeof k == 'string') {
                v = style[k];
                if (onlyWizSpan && !v && v !== 0) {
                    domUtils.clearStyle(dom, k);
                } else if (v.toString().indexOf('!important') > 0) {
                    //对于 具有 !important 的样式需要特殊添加
                    domUtils.clearStyle(dom, k);
                    dom.style.cssText += k + ':' + v;
                } else if (k.toLowerCase() == 'font-size') {
                    //如果设置的字体与 body 默认字体 同样大小， 则扩展设置 rem
                    domUtils.clearStyle(dom, k);
                    v = getRem(v);
                    if (v) {
                        dom.style.cssText += k + ':' + v;
                    }
                } else {
                    dom.style[k] = v;
                }
            }
        }

        function getRem(fontSize) {
            var s = _commonEnv2['default'].win.getComputedStyle(_commonEnv2['default'].doc.body),
                rSize = parseInt(s.fontSize, 10),
                size = parseInt(fontSize, 10);
            if (isNaN(rSize) || isNaN(size) || rSize == 0) {
                return null;
            }
            return Math.round(size / rSize * 1000) / 1000 + 'rem';
        }
    },
    /**
     * 设置 焦点
     */
    focus: function focus() {
        if (_commonEnv2['default'].win.WizTemplate) {
            _commonEnv2['default'].win.WizTemplate.focus();
        } else {
            _commonEnv2['default'].doc.body.focus();
        }
    },
    /**
     * 获取 dom 的 计算样式
     * @param dom
     * @param name
     * @param includeParent  （Boolean 如果当前Dom 不存在指定的样式，是否递归到父节点）
     * @returns {*}
     */
    getComputedStyle: function getComputedStyle(dom, name, includeParent) {
        if (!dom || !name) {
            return '';
        }
        var value;
        //while (includeParent && !value && dom!=ENV.doc.body) {
        while (!value) {
            var s = _commonEnv2['default'].win.getComputedStyle(dom);
            value = s[name] || '';

            if (/^rgba?\(.*\)$/i.test(value)) {
                value = _commonUtils2['default'].rgb2Hex(value);
            }

            if (dom == _commonEnv2['default'].doc.body || !includeParent || !!value) {
                break;
            }

            //(includeParent && !value)
            dom = dom.parentNode;
        }
        return value;
    },
    getDocType: function getDocType(doc) {
        var docType = doc.doctype;
        if (!!docType && !docType.systemId && !docType.publicId) {
            docType = '<!DOCTYPE HTML>';
        } else if (!!docType) {
            docType = '<!DOCTYPE HTML PUBLIC "' + docType.publicId + '" "' + docType.systemId + '" >';
        } else {
            docType = '<!DOCTYPE HTML>';
        }
        return docType;
    },
    /**
     * 根据 dom 树索引集合 获取 dom
     * @param indexList
     * @returns {*}
     */
    getDomByIndexList: function getDomByIndexList(indexList) {
        if (!indexList || indexList.length === 0) {
            return null;
        }
        var i, j, d, offset;
        d = _commonEnv2['default'].doc.body;
        try {
            for (i = 0, j = indexList.length - 1; i < j; i++) {
                d = d.childNodes[indexList[i]];
            }
            offset = indexList[i];
            return { dom: d, offset: offset };
        } catch (e) {
            return null;
        }
    },
    /**
     * 获取 Dom 的子元素长度（同时支持 TextNode 和 Element）
     * @param dom
     * @returns {*}
     */
    getDomEndOffset: function getDomEndOffset(dom) {
        if (!dom) {
            return 0;
        }
        return dom.nodeType == 3 ? dom.nodeValue.length : dom.childNodes.length;
    },
    /**
     * 获取 Dom 在当前相邻节点中的 位置（index）
     * @param dom
     * @returns {number}
     */
    getDomIndex: function getDomIndex(dom) {
        if (!dom || !dom.parentNode) {
            return -1;
        }
        var k = 0,
            e = dom;
        while (e = e.previousSibling) {
            ++k;
        }
        return k;
    },
    /**
     * 获取 DomA 到 DomB 中包含的所有 叶子节点
     * @param options
     * @returns {{}}
     */
    getDomListA2B: function getDomListA2B(options) {
        var startDom = options.startDom,
            startOffset = options.startOffset,
            endDom = options.endDom,
            endOffset = options.endOffset,
            noSplit = !!options.noSplit,
            isText,
            changeStart = false,
            changeEnd = false;

        //修正 start & end 位置
        if (startDom.nodeType == 1 && startOffset > 0 && startOffset < startDom.childNodes.length) {
            startDom = startDom.childNodes[startOffset];
            startOffset = 0;
        }
        if (endDom.nodeType == 1 && endOffset > 0 && endOffset < endDom.childNodes.length) {
            endDom = endDom.childNodes[endOffset];
            endOffset = 0;
        }
        //如果起始点 和终止点位置不一样， 且 endOffset == 0，则找到 endOom 前一个叶子节点
        if (startDom !== endDom && endOffset === 0) {
            endDom = domUtils.getPreviousNode(endDom, false, startDom);
            //如果 修正后的 endDom 为 自闭合标签， 需要特殊处理
            if (domUtils.isSelfClosingTag(endDom)) {
                endOffset = 1;
            } else {
                endOffset = domUtils.getDomEndOffset(endDom);
            }
        }

        // get dom which is start and end
        if (startDom == endDom && startOffset != endOffset) {
            isText = startDom.nodeType == 3;
            if (isText && !startDom.parentNode.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                startDom = noSplit ? startDom : domUtils.splitRangeText(startDom, startOffset, endOffset);
                endDom = startDom;
                changeStart = true;
                changeEnd = true;
            } else if (startDom.nodeType == 1 && startDom.childNodes.length > 0 && !domUtils.isSelfClosingTag(startDom)) {
                startDom = startDom.childNodes[startOffset];
                endDom = endDom.childNodes[endOffset - 1];
                changeStart = true;
                changeEnd = true;
            }
        } else if (startDom !== endDom) {
            if (startDom.nodeType == 3 && !startDom.parentNode.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                startDom = noSplit ? startDom : domUtils.splitRangeText(startDom, startOffset, null);
                changeStart = true;
            } else if (startDom.nodeType == 1 && startDom.childNodes.length > 0 && startOffset < startDom.childNodes.length) {
                startDom = startDom.childNodes[startOffset];
                changeStart = true;
            }
            if (endDom.nodeType == 3 && endOffset > 0 && !endDom.parentNode.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                endDom = noSplit ? endDom : domUtils.splitRangeText(endDom, 0, endOffset);
                changeEnd = true;
            } else if (!domUtils.isSelfClosingTag(endDom) && endDom.nodeType == 1 && endOffset > 0) {
                endDom = domUtils.getLastDeepChild(endDom.childNodes[endOffset - 1]);
                changeEnd = true;
            }
        }
        if (changeStart) {
            startOffset = 0;
        }
        if (changeEnd) {
            endOffset = domUtils.getDomEndOffset(endDom);
        }

        //make the array
        var curDom = startDom,
            result = [];
        if (startOffset == startDom.length) {
            curDom = domUtils.getNextNode(curDom, false, endDom);
        }

        while (curDom && !(startDom == endDom && startOffset == endOffset)) {
            if (curDom == endDom || curDom == endDom.parentNode) {
                addDomForGetDomList(result, endDom);
                break;
            } else if (domUtils.isBody(curDom)) {
                addDomForGetDomList(result, curDom);
                break;
            } else {
                addDomForGetDomList(result, curDom);
            }
            curDom = domUtils.getNextNode(curDom, false, endDom);
        }

        // startDom 和 endDom 在 clearChild 操作中可能会被删除，所以必须要记住边缘 Dom 范围
        var startDomBak = domUtils.getPreviousNode(result[0], false, null),
            endDomBak = domUtils.getNextNode(result[result.length - 1], false, null);
        if (startDomBak && startDomBak.nodeType == 1 && startDomBak.firstChild) {
            startDomBak = startDomBak.firstChild;
        }
        if (endDomBak && endDomBak.nodeType == 1 && endDomBak.lastChild) {
            endDomBak = endDomBak.lastChild;
        }
        var startOffsetBak = domUtils.getDomEndOffset(startDomBak),
            endOffsetBak = 0;

        return {
            list: result,
            startDom: startDom,
            startOffset: startOffset,
            endDom: endDom,
            endOffset: endOffset,
            startDomBak: startDomBak,
            startOffsetBak: startOffsetBak,
            endDomBak: endDomBak,
            endOffsetBak: endOffsetBak
        };

        function addDomForGetDomList(main, sub) {
            main.push(sub);
        }
    },
    /**
     * 获取 DOM 的 坐标 & 大小
     * @param obj
     * @returns {*}
     */
    getDomPosition: function getDomPosition(obj) {
        if (!obj) {
            return null;
        }
        return {
            top: obj.offsetTop,
            left: obj.offsetLeft,
            height: obj.offsetHeight,
            width: obj.offsetWidth
        };
    },
    /**
     * 获取 dom 子孙元素中第一个 叶子节点
     * @param obj
     * @returns {*}
     */
    getFirstDeepChild: function getFirstDeepChild(obj) {
        if (!obj) {
            return null;
        }
        while (obj.childNodes && obj.childNodes.length > 0) {
            obj = obj.childNodes[0];
        }
        return obj;
    },
    /**
     * 获取 dom 子孙元素中最后一个 叶子节点
     * @param obj
     * @returns {*}
     */
    getLastDeepChild: function getLastDeepChild(obj) {
        if (!obj) {
            return null;
        }
        while (obj.childNodes && obj.childNodes.length > 0) {
            obj = obj.childNodes[obj.childNodes.length - 1];
        }
        return obj;
    },
    /**
     * 获取 图片数据
     * @param img
     * @returns {*}
     */
    getImageData: function getImageData(img) {
        var size = domUtils.getImageSize(img.src);
        // Create an empty canvas element
        var canvas = _commonEnv2['default'].doc.createElement("canvas");
        canvas.width = size.width;
        canvas.height = size.height;

        // Copy the image contents to the canvas
        var ctx = canvas.getContext("2d");
        ctx.drawImage(img, 0, 0);

        // Get the data-URL formatted image
        // Firefox supports PNG and JPEG. You could check img.src to
        // guess the original format, but be aware the using "image/jpg"
        // will re-encode the image.
        var dataURL = canvas.toDataURL("image/png");

        return dataURL.replace(/^data:image\/(png|jpg);base64,/, "");
    },
    /**
     * 获取 图片 宽高
     * @param imgSrc
     * @returns {{width: Number, height: Number}}
     */
    getImageSize: function getImageSize(imgSrc) {
        var newImg = new Image();
        newImg.src = imgSrc;
        var height = newImg.height;
        var width = newImg.width;
        return { width: width, height: height };
    },
    /**
     * 获取 dom 在 dom 树内的 索引集合
     * @param dom
     * @returns {Array}
     */
    getIndexListByDom: function getIndexListByDom(dom) {
        var e = dom,
            indexList = [];
        while (e && !domUtils.isBody(e)) {
            indexList.splice(0, 0, domUtils.getDomIndex(e));
            e = e.parentNode;
        }
        return indexList;
    },
    /**
     * 获取 DOM 的 下一个 可编辑的叶子节点
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getNextNodeCanEdit: function getNextNodeCanEdit(dom, onlyElement, endDom) {
        dom = domUtils.getNextNode(dom, onlyElement, endDom);
        while (dom && !domUtils.canEdit(dom)) {
            dom = domUtils.getNextNode(dom, onlyElement, endDom);
        }
        return dom;
    },
    /**
     * 获取 DOM 的下一个叶子节点（包括不相邻的情况），到达指定的 endDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getNextNode: function getNextNode(dom, onlyElement, endDom) {
        if (!dom || dom == endDom) {
            return null;
        }
        onlyElement = !!onlyElement;
        function next(d) {
            if (!d) {
                return null;
            }
            return onlyElement ? d.nextElementSibling : d.nextSibling;
        }

        function first(d) {
            if (!d) {
                return null;
            }
            return onlyElement ? d.firstElementChild : d.firstChild;
        }

        if (!next(dom) && !dom.parentNode) {
            return null;
        } else if (!next(dom)) {
            //if hasn't nextSibling,so find its parent's nextSibling
            while (dom.parentNode) {
                dom = dom.parentNode;
                if (dom == endDom) {
                    break;
                }
                if (domUtils.isBody(dom)) {
                    dom = null;
                    break;
                }
                if (next(dom)) {
                    dom = next(dom);
                    break;
                }
            }
        } else {
            dom = next(dom);
        }

        if (dom == endDom) {
            return dom;
        }

        //if next node has child nodes, so find the first child node.
        var tmpD;
        tmpD = first(dom);
        if (!!dom && tmpD) {
            while (tmpD) {
                dom = tmpD;
                if (dom == endDom) {
                    break;
                }
                tmpD = first(tmpD);
            }
        }
        return dom;
    },
    /**
     * 获取 页面滚动条位置
     * @returns {{}}
     */
    getPageScroll: function getPageScroll() {
        var scroll = {};
        if (typeof _commonEnv2['default'].win.pageYOffset != 'undefined') {
            scroll.left = _commonEnv2['default'].win.pageXOffset;
            scroll.top = _commonEnv2['default'].win.pageYOffset;
        } else if (typeof _commonEnv2['default'].doc.compatMode != 'undefined' && _commonEnv2['default'].doc.compatMode != 'BackCompat') {
            scroll.left = _commonEnv2['default'].doc.documentElement.scrollLeft;
            scroll.top = _commonEnv2['default'].doc.documentElement.scrollTop;
        } else if (typeof _commonEnv2['default'].doc.body != 'undefined') {
            scroll.left = _commonEnv2['default'].doc.body.scrollLeft;
            scroll.top = _commonEnv2['default'].doc.body.scrollTop;
        }
        return scroll;
    },
    /**
     * 根据 filterFn 函数设置的 自定义规则 查找 Dom 的父节点
     * @param node
     * @param filterFn
     * @param includeSelf
     * @returns {*}
     */
    getParentByFilter: function getParentByFilter(node, filterFn, includeSelf) {
        if (node && !domUtils.isBody(node)) {
            node = includeSelf ? node : node.parentNode;
            while (node) {
                if (!filterFn || filterFn(node)) {
                    return node;
                }
                if (domUtils.isBody(node)) {
                    return null;
                }
                node = node.parentNode;
            }
        }
        return null;
    },
    /**
     * 根据 Tag 名称查找 Dom 的父节点
     * @param node
     * @param tagNames
     * @param includeSelf
     * @param excludeFn
     * @returns {*}
     */
    getParentByTagName: function getParentByTagName(node, tagNames, includeSelf, excludeFn) {
        if (!node) {
            return null;
        }
        tagNames = _commonUtils2['default'].listToMap(_commonUtils2['default'].isArray(tagNames) ? tagNames : [tagNames]);
        return domUtils.getParentByFilter(node, function (node) {
            return tagNames[node.tagName] && !(excludeFn && excludeFn(node));
        }, includeSelf);
    },
    /**
     * 获取多个 dom 共同的父节点
     * @param domList
     */
    getParentRoot: function getParentRoot(domList) {
        if (!domList || domList.length === 0) {
            return null;
        }
        var i,
            j,
            tmpIdx,
            pNode,
            parentList = [];
        pNode = domList[0].nodeType == 1 ? domList[0] : domList[0].parentNode;
        while (pNode && !domUtils.isBody(pNode)) {
            parentList.push(pNode);
            pNode = pNode.parentNode;
        }
        for (i = 1, j = domList.length; i < j; i++) {
            pNode = domList[i];
            while (pNode) {
                if (domUtils.isBody(pNode)) {
                    return _commonEnv2['default'].doc.body;
                }
                tmpIdx = parentList.indexOf(pNode);
                if (tmpIdx > -1) {
                    parentList.splice(0, tmpIdx);
                    break;
                }
                pNode = pNode.parentNode;
            }
        }
        if (parentList.length === 0) {
            return _commonEnv2['default'].doc.body;
        } else {
            return parentList[0];
        }
    },
    /**
     * 获取 DOM 的 下一个 可编辑的叶子节点
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getPreviousNodeCanEdit: function getPreviousNodeCanEdit(dom, onlyElement, endDom) {
        dom = domUtils.getPreviousNode(dom, onlyElement, endDom);
        while (dom && !domUtils.canEdit(dom)) {
            dom = domUtils.getPreviousNode(dom, onlyElement, endDom);
        }
        return dom;
    },
    /**
     * 获取 DOM 的前一个叶子节点（包括不相邻的情况），到达指定的 startDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param startDom
     * @returns {*}
     */
    getPreviousNode: function getPreviousNode(dom, onlyElement, startDom) {
        if (!dom || dom == startDom) {
            return null;
        }
        onlyElement = !!onlyElement;
        function prev(d) {
            return onlyElement ? d.previousElementSibling : d.previousSibling;
        }

        function last(d) {
            return onlyElement ? d.lastElementChild : d.lastChild;
        }

        if (!prev(dom)) {
            //if hasn't previousSibling,so find its parent's previousSibling
            while (dom.parentNode) {
                dom = dom.parentNode;
                if (dom == startDom) {
                    break;
                }
                if (domUtils.isBody(dom)) {
                    dom = null;
                    break;
                }
                if (prev(dom)) {
                    dom = prev(dom);
                    break;
                }
            }
        } else {
            dom = prev(dom);
        }

        if (!dom) {
            return null;
        }
        //对于查找前一个dom节点的算法 与 查找 下一个dom的算法略有不同
        //如果 dom 与 startDom 相同， 但 dom 有子元素的时候， 不能直接返回 dom
        if (dom == startDom && (dom.nodeType === 3 || dom.nodeType === 1 && dom.childNodes.length === 0)) {
            return dom;
        }

        //if previous node has child nodes, so find the last child node.
        var tmpD;
        tmpD = last(dom);
        if (!!dom && tmpD) {
            while (tmpD) {
                dom = tmpD;
                if (dom == startDom && (dom.nodeType === 3 || dom.nodeType === 1 && dom.childNodes.length === 0)) {
                    break;
                }
                tmpD = last(tmpD);
            }
        }

        return dom;
    },
    /**
     * 给 dom 内添加 Tab 时 获取 4 个 ' '
     */
    getTab: function getTab() {
        var x = _commonEnv2['default'].doc.createElement('span');
        x.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;';
        return x.childNodes[0];
    },
    /**
     * 获取 td,th 单元格 在 table 中的 行列坐标
     * @param td
     */
    getTdIndex: function getTdIndex(td) {
        return {
            x: td.cellIndex,
            y: td.parentNode.rowIndex,
            maxX: td.parentNode.cells.length,
            maxY: td.parentNode.parentNode.rows.length
        };
    },
    getOffset: function getOffset(dom) {
        var offset = { top: 0, left: 0 };
        if (dom.offsetParent) {
            while (dom.offsetParent) {
                offset.top += dom.offsetTop;
                offset.left += dom.offsetLeft;
                dom = dom.offsetParent;
            }
        } else {
            offset.left += dom.offsetLeft;
            offset.top += dom.offsetTop;
        }
        return offset;
    },
    /**
     * 根据 dom 获取其 修订的父节点， 如果不是 修订内容，则返回空
     * @param dom
     * @returns {*}
     */
    getWizAmendParent: function getWizAmendParent(dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            //node.childNodes.length == 0 时，键盘敲入的字符加在 span 外面
            return node && node.nodeType === 1 && (node.getAttribute(_commonConst2['default'].ATTR.SPAN_INSERT) || node.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE));
        }, true);
    },
    /**
     * 判断 dom 是否含有 某个 class name
     * @param obj
     * @param className
     * @returns {boolean}
     */
    hasClass: function hasClass(obj, className) {
        if (obj && obj.nodeType === 1) {
            return (' ' + obj.className + ' ').indexOf(' ' + className + ' ') > -1;
        }
        return false;
    },
    /**
     * wiz 编辑器使用的 插入 dom 方法（isAfter 默认为 false，即将 dom 插入到 target 前面）
     * @param target
     * @param dom
     * @param isAfter
     */
    insert: function insert(target, dom, isAfter) {
        isAfter = !!isAfter;
        if (!target || !dom) {
            return;
        }
        var isBody = target === _commonEnv2['default'].doc.body,
            parent = isBody ? target : target.parentNode,
            nextDom = isBody ? isAfter ? null : _commonEnv2['default'].doc.body.childNodes[0] : isAfter ? target.nextSibling : target;
        var i, d, last;
        if (!_commonUtils2['default'].isArray(dom)) {
            parent.insertBefore(dom, nextDom);
        } else {
            last = nextDom;
            for (i = dom.length - 1; i >= 0; i--) {
                d = dom[i];
                parent.insertBefore(d, last);
                last = d;
            }
        }
    },
    /**
     * 判断 dom 是否为 document.body
     * @param dom
     * @returns {*|boolean|boolean}
     */
    isBody: function isBody(dom) {
        return dom && dom == _commonEnv2['default'].doc.body;
    },
    /**
     * 判断 dom 是否为空（里面仅有 br 时 也被认为空）
     * @param dom
     * @returns {*}
     */
    isEmptyDom: function isEmptyDom(dom) {
        var i, j, v;
        if (dom.nodeType === 3) {
            v = dom.nodeValue;
            return _commonUtils2['default'].isEmpty(v);
        }

        if (dom.nodeType !== 1) {
            return true;
        }

        if (dom.childNodes.length === 0) {
            return domUtils.isTag(dom, 'br') || !domUtils.isSelfClosingTag(dom);
        }

        for (i = 0, j = dom.childNodes.length; i < j; i++) {
            if (!domUtils.isEmptyDom(dom.childNodes[i])) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 内容是否为 填充的特殊字符
     * @param node
     * @param isInStart
     * @returns {boolean}
     */
    isFillChar: function isFillChar(node, isInStart) {
        return node.nodeType == 3 && !node.nodeValue.replace(new RegExp((isInStart ? '^' : '') + _commonConst2['default'].FILL_CHAR), '').length;
    },
    /**
     * 判断 dom 是否为 自闭和标签 （主要用于清理冗余 dom 使用，避免 dom 被删除）
     * @param node
     * @returns {boolean}
     */
    isSelfClosingTag: function isSelfClosingTag(node) {
        var selfLib = /^(area|base|br|col|command|embed|hr|img|input|keygen|link|meta|param|source|track|wbr)$/i;
        return node.nodeType === 1 && selfLib.test(node.tagName);
    },
    /**
     * 判断两个 span 属性（style & attribute）是否相同（属性相同且相邻的两个 span 才可以合并）
     * @param n
     * @param m
     * @returns {boolean}
     */
    isSameSpan: function isSameSpan(n, m) {
        return !!n && !!m && n.nodeType == 1 && m.nodeType == 1 && domUtils.isTag(n, 'span') && n.tagName == m.tagName && n.getAttribute(_commonConst2['default'].ATTR.SPAN) == _commonConst2['default'].ATTR.SPAN && domUtils.isSameStyle(n, m) && domUtils.isSameAttr(n, m);
    },
    /**
     * 判断两个 dom 的 attribute 是否相同
     * @param n
     * @param m
     * @returns {boolean}
     */
    isSameAttr: function isSameAttr(n, m) {
        var attrA = n.attributes,
            attrB = m.attributes;
        if (attrA.length != attrB.length) {
            return false;
        }
        var i, j, a;
        for (i = 0, j = attrA.length; i < j; i++) {
            a = attrA[i];
            if (a.name == 'style') {
                continue;
            }
            if (a.name === _commonConst2['default'].ATTR.SPAN_TIMESTAMP) {
                if (!_commonUtils2['default'].isSameAmendTime(a.value, attrB[a.name].value)) {
                    return false;
                }
                continue;
            } else if (!attrB[a.name] || attrB[a.name].value != a.value) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 的 style （inline）是否相同
     * @param n
     * @param m
     */
    isSameStyle: function isSameStyle(n, m) {
        var styleA = {};
        var styleB = {};
        domUtils.appendStyle(n.getAttribute('style'), styleA);
        domUtils.appendStyle(m.getAttribute('style'), styleB);
        var k;
        for (k in styleA) {
            if (styleA.hasOwnProperty(k)) {
                if (styleB[k] !== styleA[k]) {
                    return false;
                }
                delete styleA[k];
                delete styleB[k];
            }
        }
        for (k in styleB) {
            if (styleB.hasOwnProperty(k)) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 是否为指定的 tagName
     * @param dom
     * @param tagNames
     * @returns {boolean}
     */
    isTag: function isTag(dom, tagNames) {
        if (!_commonUtils2['default'].isArray(tagNames)) {
            tagNames = [tagNames];
        }
        if (!dom || dom.nodeType !== 1) {
            return false;
        }
        var i,
            j,
            tag = dom.tagName.toLowerCase();
        for (i = 0, j = tagNames.length; i < j; i++) {
            if (tag === tagNames[i].toLowerCase()) {
                return true;
            }
        }
        return false;
    },
    /**
     * 判断 TextNode 内容是否为 非空 有效
     * @param node
     * @returns {boolean}
     */
    isUsableTextNode: function isUsableTextNode(node) {
        return node.nodeType == 3 && !_commonUtils2['default'].isEmpty(node.nodeValue);
    },
    /**
     * 判断 dom 是否为 wiz 编辑器 的 span
     * @param dom
     * @returns {boolean}
     */
    isWizSpan: function isWizSpan(dom) {
        return !!dom && !!dom.getAttribute(_commonConst2['default'].ATTR.SPAN);
    },
    /**
     * 把 domA 合并到 domB （仅合并 attribute 和 style）
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeAtoB: function mergeAtoB(objA, objB, isOverlay) {
        domUtils.mergeStyleAToB(objA, objB, isOverlay);
        domUtils.mergeAttrAtoB(objA, objB, isOverlay);
    },
    /**
     * 把 domA 的属性（attribute） 合并到 domB
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeAttrAtoB: function mergeAttrAtoB(objA, objB, isOverlay) {
        if (objA.nodeType != 1 || objB.nodeType != 1) {
            return;
        }
        var attrA = objA.attributes,
            attrB = objB.attributes,
            i,
            j,
            a;
        for (i = 0, j = attrA.length; i < j; i++) {
            a = attrA[i];
            if (a.name == 'style') {
                continue;
            }
            if (attrB[a.name] && !isOverlay) {
                continue;
            }
            objB.setAttribute(a.name, a.value);
        }
    },
    /**
     * 把 domA 的样式（style） 合并到 domB
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeStyleAToB: function mergeStyleAToB(objA, objB, isOverlay) {
        if (objA.nodeType != 1 || objB.nodeType != 1) {
            return;
        }
        var sA = objA.getAttribute('style'),
            sB = objB.getAttribute('style') || '';
        if (!sA) {
            return;
        }
        var styleObj = {};
        if (!!isOverlay) {
            domUtils.appendStyle(sB, styleObj);
            domUtils.appendStyle(sA, styleObj);
        } else {
            domUtils.appendStyle(sA, styleObj);
            domUtils.appendStyle(sB, styleObj);
        }

        var result = [];
        for (var k in styleObj) {
            if (styleObj.hasOwnProperty(k)) {
                result.push(k + ':' + styleObj[k]);
            }
        }
        objB.setAttribute('style', result.join(';'));
    },
    /**
     * 移除 class name
     * @param domList
     * @param className
     */
    removeClass: function removeClass(domList, className) {
        if (!domList) {
            return;
        }
        if (!!domList.nodeType) {
            domList = [domList];
        }
        var i, dom;
        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (dom.nodeType === 1) {
                dom.className = (" " + dom.className + " ").replace(' ' + className + ' ', ' ').trim();
            }
        }
    },
    /**
     * 从 Dom 中清除指定 name 的 tag
     * @param name
     */
    removeDomByName: function removeDomByName(name) {
        var s = _commonEnv2['default'].doc.getElementsByName(name);
        var i, dom;
        for (i = s.length - 1; i >= 0; i--) {
            dom = s[i];
            dom.parentNode.removeChild(dom);
        }
    },
    /**
     * 从 Dom 中清除指定 的 tag
     * @param tag
     */
    removeDomByTag: function removeDomByTag(tag) {
        var s = _commonEnv2['default'].doc.getElementsByTagName(tag);
        var i, dom;
        for (i = s.length - 1; i >= 0; i--) {
            dom = s[i];
            dom.parentNode.removeChild(dom);
        }
    },
    /**
     * 从 html 源码中清除指定 name 的 style
     * 因为使用正则，不可能直接将 有嵌套 div 的div html 代码删除，所以此函数只能针对 style 等不会包含 html 代码的 tag 进行操作
     * @param html
     * @param name
     * @returns {string}
     */
    removeStyleByNameFromHtml: function removeStyleByNameFromHtml(html, name) {
        var reg = new RegExp('<style( ([^<>])+[ ]+|[ ]+)name *= *[\'"]' + name + '[\'"][^<>]*>[^<]*<\/style>', 'ig');
        return html.replace(reg, '');
    },
    /**
     * 从 html 源码中清除指定的 tag （注意，一定要保证该 tag 内不存在嵌套同样 tag 的情况）
     * @param html
     * @param tag
     * @returns {string}
     */
    removeDomByTagFromHtml: function removeDomByTagFromHtml(html, tag) {
        var reg = new RegExp('<' + tag + '([ ][^>]*)*>.*<\/' + tag + '>', 'ig');
        return html.replace(reg, '');
    },
    /**
     * 从 dom 集合中删除符合特殊规则的 dom
     * @param domList
     * @param filter
     * @returns {Array} 返回被删除的集合列表
     */
    removeListFilter: function removeListFilter(domList, filter) {
        var removeList = [],
            i,
            dom;

        if (!domList || !filter) {
            return removeList;
        }

        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (filter(dom)) {
                removeList.unshift(domList.splice(i, 1)[0]);
            }
        }
        return removeList;
    },
    /**
     * 根据 查询表达式 查找 dom，并放到 list 集合内
     * @param dom
     * @param expStr
     * @param list
     */
    search: function search(dom, expStr, list) {
        //TODO 兼容问题
        var tmpList = dom.querySelectorAll(expStr),
            i,
            j,
            d;
        list = list ? list : [];
        for (i = 0, j = tmpList.length; i < j; i++) {
            d = tmpList[i];
            list.push(d);
        }
    },
    /**
     * 设置区域可编辑
     * @param content
     * @param enable
     */
    setContenteditable: function setContenteditable(content, enable) {
        if (!content && _commonEnv2['default'].win.WizTemplate) {
            _commonEnv2['default'].win.WizTemplate.setContenteditable(enable);
        } else {
            if (!content) {
                content = _commonEnv2['default'].doc.body;
            }
            content.setAttribute('contenteditable', enable ? 'true' : 'false');
        }
    },
    /**
     * 自动布局（根据 target 的位置 以及 屏幕大小，设置 layerObj 的坐标，保证在可视区域内显示）
     * @param options
     * {layerObj, target, layout, fixed, noSpace, reverse}
     */
    setLayout: function setLayout(options) {
        var layerObj = options.layerObj,
            target = options.target,
            layout = options.layout,
            fixed = !!options.fixed,
            noSpace = !!options.noSpace,
            reverse = !!options.reverse;

        var confirmPos = domUtils.getDomPosition(layerObj),
            targetPos = target.nodeType ? domUtils.getDomPosition(target) : target,
            scrollPos = domUtils.getPageScroll(),
            winWidth = _commonEnv2['default'].doc.documentElement.clientWidth,
            winHeight = _commonEnv2['default'].doc.documentElement.clientHeight,
            bodyTop = window.getComputedStyle ? _commonEnv2['default'].win.getComputedStyle(_commonEnv2['default'].doc.body, null)['margin-top'] : 0,
            left = '50%',
            top = '30%',
            mTop = 0,
            mLeft = -confirmPos.width / 2,
            minWidth,
            maxWidth,
            minHeight,
            maxHeight;

        //iphone 客户端 编辑时 window 窗口顶端有其他 window 遮罩， 所以必须要计算 body 的 margin-top
        if (!!bodyTop) {
            bodyTop = parseInt(bodyTop);
            if (isNaN(bodyTop)) {
                bodyTop = 0;
            }
        }

        if (fixed) {
            minWidth = 0;
            maxWidth = winWidth - 5; //右侧需要保留一些空间，避免有时候超出
            minHeight = 0 + bodyTop;
            maxHeight = winHeight;
        } else {
            minWidth = 0 + scrollPos.left;
            maxWidth = winWidth + scrollPos.left - 5; //右侧需要保留一些空间，避免有时候超出
            minHeight = 0 + (scrollPos.top <= bodyTop ? 0 : Math.abs(scrollPos.top - bodyTop)) + bodyTop;
            maxHeight = winHeight + scrollPos.top;
        }

        if (targetPos && layout) {
            mTop = 0;
            mLeft = 0;
            if (layout == _commonConst2['default'].TYPE.POS.upLeft || layout == _commonConst2['default'].TYPE.POS.upRight) {
                top = targetPos.top - confirmPos.height - (noSpace ? 0 : _commonConst2['default'].AMEND.INFO_SPACE);
            } else if (layout == _commonConst2['default'].TYPE.POS.downLeft || layout == _commonConst2['default'].TYPE.POS.downRight) {
                top = targetPos.top + targetPos.height + (noSpace ? 0 : _commonConst2['default'].AMEND.INFO_SPACE);
            } else if (layout == _commonConst2['default'].TYPE.POS.leftUp || layout == _commonConst2['default'].TYPE.POS.leftDown) {
                left = targetPos.left - confirmPos.width - (noSpace ? 0 : _commonConst2['default'].AMEND.INFO_SPACE);
            } else if (layout == _commonConst2['default'].TYPE.POS.rightUp || layout == _commonConst2['default'].TYPE.POS.rightDown) {
                left = targetPos.left + targetPos.width + (noSpace ? 0 : _commonConst2['default'].AMEND.INFO_SPACE);
            }

            if (layout == _commonConst2['default'].TYPE.POS.upLeft || layout == _commonConst2['default'].TYPE.POS.downLeft) {
                left = targetPos.left;
                if (fixed) {
                    left -= scrollPos.left;
                }
            } else if (layout == _commonConst2['default'].TYPE.POS.upRight || layout == _commonConst2['default'].TYPE.POS.downRight) {
                left = targetPos.left + targetPos.width - confirmPos.width;
                if (fixed) {
                    left -= scrollPos.left;
                }
            } else if (layout == _commonConst2['default'].TYPE.POS.leftUp || layout == _commonConst2['default'].TYPE.POS.rightUp) {
                top = targetPos.top;
                if (fixed) {
                    top -= scrollPos.top;
                }
            } else if (layout == _commonConst2['default'].TYPE.POS.leftDown || layout == _commonConst2['default'].TYPE.POS.rightDown) {
                top = targetPos.top + targetPos.height - confirmPos.height;
                if (fixed) {
                    top -= scrollPos.top;
                }
            }

            if (left + confirmPos.width > maxWidth) {
                left = maxWidth - confirmPos.width;
            }
            if (left < minWidth) {
                left = minWidth;
            }
            if (top + confirmPos.height > maxHeight) {
                top = maxHeight - confirmPos.height;
            }
            if (reverse && top < minHeight) {
                top = targetPos.top + targetPos.height;
            }
            if (top < minHeight || top + confirmPos.height > maxHeight) {
                top = minHeight;
            }
        }
        domUtils.css(layerObj, {
            left: left + 'px',
            top: top + 'px',
            'margin-top': mTop + 'px',
            'margin-left': mLeft + 'px'
        }, false);
    },

    /**
     * 根据 光标选择范围 拆分 textNode
     * splitRangeText 不能返回 TextNode，所以在 wizSpan 内要把 TextNode 独立分割出来，然后返回其 parentNode
     * @param node
     * @param start
     * @param end
     * @returns {*}
     */
    splitRangeText: function splitRangeText(node, start, end) {
        if (!domUtils.isUsableTextNode(node)) {
            return node;
        }
        var p,
            s,
            t,
            v = node.nodeValue;
        p = node.parentNode;
        //            var isWizSpan = domUtils.isWizSpan(p);
        s = domUtils.createSpan();

        if (!start && !end || start === 0 && end === node.nodeValue.length) {
            //the range is all text in this node
            // td,th 必须特殊处理，否则会导致 td 被添加 修订样式
            if (p.childNodes.length > 1 || domUtils.isTag(p, ['td', 'th'])) {
                p.insertBefore(s, node);
                s.appendChild(node);
            } else {
                //if textNode is the only child node, return its parent node.
                s = p;
            }
        } else if (start === 0) {
            //the range is [0, n] (n<length)
            p.insertBefore(s, node);
            s.innerText = v.substring(start, end);
            node.nodeValue = v.substring(end);
        } else if (!end || end === node.nodeValue.length) {
            p.insertBefore(s, node.nextSibling);
            s.innerText = v.substring(start);
            node.nodeValue = v.substring(0, start);
        } else {
            //the range is [m, n] (m>0 && n<length)
            t = _commonEnv2['default'].doc.createTextNode(v.substring(end));
            p.insertBefore(s, node.nextSibling);
            s.innerText = v.substring(start, end);
            p.insertBefore(t, s.nextSibling);
            //必须要先添加文字，最后删除多余文字，否则，如果先删除后边文字，会导致滚动条跳动
            node.nodeValue = v.substring(0, start);
        }
        return s;
    }
};

exports['default'] = domUtils;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10}],15:[function(require,module,exports){
/**
 * DOM 操作工具包（扩展类库）
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domBase = require('./domBase');

var _domBase2 = _interopRequireDefault(_domBase);

/**
 * 清理无用的 子节点 span ，合并 attribute & style 相同的 span
 * @param dom
 * @param excludeList
 */
_domBase2['default'].clearChild = function (dom, excludeList) {
    if (!dom) {
        return;
    }
    var isExclude = excludeList.indexOf(dom) >= 0;
    if (!isExclude && dom.nodeType == 3 && !_domBase2['default'].isUsableTextNode(dom)) {
        dom.parentNode.removeChild(dom);
        return;
    } else if (!isExclude && dom.nodeType == 3) {
        dom.nodeValue = dom.nodeValue.replace(_commonConst2['default'].FILL_CHAR_REG, '');
        return;
    }

    if (!isExclude && dom.nodeType == 1) {
        var ns = dom.childNodes,
            i,
            item;
        for (i = ns.length - 1; i >= 0; i--) {
            item = ns[i];
            _domBase2['default'].clearChild(item, excludeList);
        }
        _domBase2['default'].mergeChildSpan(dom, excludeList);

        if (excludeList.indexOf(dom) < 0 && dom.childNodes.length === 0 && dom.nodeType == 1 && !_domBase2['default'].isSelfClosingTag(dom) &&
        //                    dom.tagName.toLowerCase() == 'span' && !!dom.getAttribute(CONST.ATTR.SPAN)) {
        !!dom.getAttribute(_commonConst2['default'].ATTR.SPAN)) {
            dom.parentNode.removeChild(dom);
        }
    }
};
/**
 * 合并 子节点中相邻且相同（style & attribute ）的 span
 * merge the same span with parent and child nodes.
 * @param dom
 * @param excludeList
 */
_domBase2['default'].mergeChildSpan = function (dom, excludeList) {
    if (!dom || dom.nodeType !== 1) {
        return;
    }
    var i, j;
    for (i = 0, j = dom.children.length; i < j; i++) {
        _domBase2['default'].mergeChildSpan(dom.children[i], excludeList);
    }
    _domBase2['default'].mergeSiblingSpan(dom, excludeList);

    var n = dom.children[0],
        tmp;
    if (!!n && excludeList.indexOf(n) < 0 && dom.childNodes.length == 1 && dom.getAttribute(_commonConst2['default'].ATTR.SPAN) == _commonConst2['default'].ATTR.SPAN && n.getAttribute(_commonConst2['default'].ATTR.SPAN) == _commonConst2['default'].ATTR.SPAN) {
        _domBase2['default'].mergeChildToParent(dom, n);
    } else {
        while (!!n) {
            if (excludeList.indexOf(n) < 0 && excludeList.indexOf(dom) < 0 && _domBase2['default'].isSameSpan(dom, n)) {
                tmp = n.previousElementSibling;
                _domBase2['default'].mergeChildToParent(dom, n);
                n = tmp ? tmp.nextElementSibling : dom.children[0];
            } else {
                n = n.nextElementSibling;
            }
        }
    }
};
/**
 * 将 子节点 合并到 父节点 （主要用于 嵌套的 span 合并）
 * @param parent
 * @param child
 */
_domBase2['default'].mergeChildToParent = function (parent, child) {
    if (!parent || !child || child.parentNode !== parent) {
        return;
    }
    while (child.childNodes.length > 0) {
        _domBase2['default'].insert(child, child.childNodes[0], false);
    }
    _domBase2['default'].mergeAtoB(parent, child, false);
    _domBase2['default'].mergeAtoB(child, parent, true);
    parent.removeChild(child);
};
/**
 * 合并相邻且相同（style & attribute ）的 span
 * @param parentDom
 * @param excludeList
 */
_domBase2['default'].mergeSiblingSpan = function (parentDom, excludeList) {
    var n = parentDom.childNodes[0],
        m,
        tmp;
    if (!n) {
        return;
    }
    while (n) {
        m = n.nextSibling;
        if (m && excludeList.indexOf(m) < 0 && excludeList.indexOf(n) < 0 && _domBase2['default'].isSameSpan(n, m)) {
            while (m.childNodes.length) {
                tmp = m.childNodes[0];
                if (tmp && (tmp.innerHTML || tmp.nodeValue && tmp.nodeValue != _commonConst2['default'].FILL_CHAR)) {
                    n.appendChild(tmp);
                } else {
                    m.removeChild(tmp);
                }
            }
            m.parentNode.removeChild(m);
        } else {
            n = m;
        }
    }
};
_domBase2['default'].modifyChildNodesStyle = function (dom, style, attr) {
    if (!dom) {
        return;
    }
    var ns = dom.childNodes,
        done = false,
        i,
        item;
    for (i = 0; i < ns.length; i++) {
        item = ns[i];
        if (!done && _domBase2['default'].isUsableTextNode(item)) {
            done = true;
            _domBase2['default'].modifyStyle(dom, style, attr);
        } else if (item.nodeType == 1) {
            _domBase2['default'].modifyChildNodesStyle(item, style, attr);
        }
    }
};
_domBase2['default'].modifyNodeStyle = function (item, style, attr) {
    if (item.nodeType == 1) {
        if (_domBase2['default'].isSelfClosingTag(item)) {
            _domBase2['default'].modifyStyle(item, style, attr);
        } else {
            _domBase2['default'].modifyChildNodesStyle(item, style, attr);
        }
    } else if (_domBase2['default'].isUsableTextNode(item)) {
        item = _domBase2['default'].splitRangeText(item, null, null);
        _domBase2['default'].modifyStyle(item, style, attr);
    }
    return item;
};
/**
 * 修改 集合中所有Dom 的样式（style） & 属性（attribute）
 * @param domList
 * @param style
 * @param attr
 */
_domBase2['default'].modifyNodesStyle = function (domList, style, attr) {
    if (domList.length === 0) {
        return;
    }
    var i, j, item;
    for (i = 0, j = domList.length; i < j; i++) {
        item = domList[i];
        domList[i] = _domBase2['default'].modifyNodeStyle(item, style, attr);
    }
};
/**
 * 修改 Dom 的样式（style） & 属性（attribute）
 * @param dom
 * @param style
 * @param attr
 */
_domBase2['default'].modifyStyle = function (dom, style, attr) {

    var isSelfClosingTag = _domBase2['default'].isSelfClosingTag(dom);
    //自闭合标签 不允许设置 新增的修订标识
    if (attr && attr[_commonConst2['default'].ATTR.SPAN_INSERT] && isSelfClosingTag) {
        return;
    }

    var d = dom;

    if (attr && (attr[_commonConst2['default'].ATTR.SPAN_INSERT] || attr[_commonConst2['default'].ATTR.SPAN_DELETE])) {
        //如果 dom 是 修订的内容， 且设定修订内容 则必须要针对 修订DOM 处理
        d = _domBase2['default'].getWizAmendParent(dom);
        if (!d) {
            d = dom;
        } else {
            dom = null;
        }
    }

    if (!!dom && !isSelfClosingTag && (!_domBase2['default'].isTag(dom, 'span') || dom.getAttribute(_commonConst2['default'].ATTR.SPAN) !== _commonConst2['default'].ATTR.SPAN)) {
        d = _domBase2['default'].createSpan();
        dom.insertBefore(d, null);
        while (dom.childNodes.length > 1) {
            d.insertBefore(dom.childNodes[0], null);
        }
    }
    _domBase2['default'].css(d, style, false);
    _domBase2['default'].attr(d, attr);
};
/**
 * 在删除 当前用户已删除 指定的Dom 后， 判断其 parentNode 是否为空，如果为空，继续删除
 * @param pDom
 */
_domBase2['default'].removeEmptyParent = function (pDom) {
    if (!pDom) {
        return;
    }
    var p;
    if (_domBase2['default'].isEmptyDom(pDom)) {
        if (pDom === _commonEnv2['default'].doc.body || _domBase2['default'].isTag(pDom, ['td', 'th'])) {
            //如果 pDom 为 body | td | th 且为空， 则添加 br 标签
            pDom.innerHTML = '<br/>';
        } else {
            p = pDom.parentNode;
            if (p) {
                p.removeChild(pDom);
                _domBase2['default'].removeEmptyParent(p);
            }
        }
    }
};

/**
 * 将 mainDom 以子节点 subDom 为分割点 分割为两个 mainDom（用于 修订处理）
 * @param mainDom
 * @param subDom
 */
_domBase2['default'].splitDom = function (mainDom, subDom) {
    if (!mainDom || !subDom || !subDom.previousSibling) {
        return;
    }
    var p = mainDom.parentNode,
        m2 = mainDom.cloneNode(false),
        next;
    while (subDom) {
        next = subDom.nextSibling;
        m2.appendChild(subDom);
        subDom = next;
    }
    p.insertBefore(m2, mainDom.nextSibling);
};

exports['default'] = _domBase2['default'];
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10,"./domBase":14}],16:[function(require,module,exports){
/**
 * img 操作基本方法集合
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('./../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('./../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

var handleSuffix = ['lt', 'tm', 'rt', 'rm', 'rb', 'bm', 'lb', 'lm'];

var resizingHanlde = '';
var WIZ_STYLE = 'wiz_style';

var startOffsetX;
var startOffsetY;
var lastMousex;
var lastMousey;
var oppCornerX;
var oppCornerY;

var cursorOri;
var cursor;

function init() {
    cursorOri = _commonEnv2['default'].doc.body.style.cursor || '';

    // TODO 临时为 pc端处理，pc端整合后， 直接删除
    _commonEnv2['default'].win.WizImgResizeOnGetHTML = function () {};
}

function initImageDragResize(img) {
    if (!img || !img.tagName || img.tagName.toLowerCase() != 'img') return;
    if (!canDragResize(img)) return;
    //
    var container = createHandles();
    if (!container) {
        return;
    }
    resetHandlesSize(img);
    initImage(img);

    _event.bindContainer(container);
}

function clearHandles() {
    removeImgAttributes();
    removeHandles();
    _commonEnv2['default'].doc.body.style.cursor = cursorOri;
}

function createHandles() {
    var container = getHandleContainer();
    if (container) {
        return container;
    }
    container = _commonEnv2['default'].doc.createElement(_commonConst2['default'].TAG.TMP_TAG);
    _domUtilsDomExtend2['default'].addClass(container, _commonConst2['default'].CLASS.IMG_RESIZE_CONTAINER);
    container.setAttribute('contenteditable', 'false');
    container.setAttribute(WIZ_STYLE, 'unsave');

    for (var i = 0; i < handleSuffix.length; i++) {
        var handle = _commonEnv2['default'].doc.createElement('div');
        _domUtilsDomExtend2['default'].addClass(handle, _commonConst2['default'].CLASS.IMG_RESIZE_HANDLE);
        _domUtilsDomExtend2['default'].addClass(handle, handleSuffix[i]);
        _domUtilsDomExtend2['default'].attr(handle, {
            'data-type': handleSuffix[i]
        });
        container.appendChild(handle);
    }
    _commonEnv2['default'].doc.body.appendChild(container);
    return container;
}

function getHandleContainer() {
    var container = _commonEnv2['default'].doc.body.querySelector('.' + _commonConst2['default'].CLASS.IMG_RESIZE_CONTAINER);
    if (!container || container.length < 1) {
        return null;
    }
    return container;
}

function setHandleSize(imgOptions, handle) {
    if (!imgOptions || !handle) return;
    var offset = imgOptions.offset;
    var x = offset.left,
        y = offset.top,
        width = imgOptions.width,
        height = imgOptions.height;

    var handleName = handle.getAttribute('data-type');
    var left = 0,
        top = 0;
    switch (handleName) {
        case 'lt':
            left = x - 7;
            top = y - 7;
            break;
        case 'tm':
            left = x + (width - 7) / 2;
            top = y - 7;
            break;
        case 'rt':
            left = x + width;
            top = y - 7;
            break;
        case 'rm':
            left = x + width;
            top = y + (height - 7) / 2;
            break;
        case 'rb':
            left = x + width;
            top = y + height;
            break;
        case 'bm':
            left = x + (width - 7) / 2;
            top = y + height;
            break;
        case 'lb':
            left = x - 7;
            top = y + height;
            break;
        case 'lm':
            left = x - 7;
            top = y + (height - 7) / 2;
            break;
    }
    _domUtilsDomExtend2['default'].css(handle, {
        left: left + 'px',
        top: top + 'px'
    });
}

function resetHandlesSize(img) {
    if (!img) {
        return;
    }
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    var handles = container.querySelectorAll('.' + _commonConst2['default'].CLASS.IMG_RESIZE_HANDLE);

    var imgOptions = {
        offset: _domUtilsDomExtend2['default'].getOffset(img),
        width: img.width,
        height: img.height
    };
    for (var i = 0; i < handles.length; i++) {
        var handle = handles[i];
        setHandleSize(imgOptions, handle);
        handle.style.visibility = 'inherit';
    }
}

function removeImgAttributes() {
    var imgList = _commonEnv2['default'].doc.querySelectorAll('.' + _commonConst2['default'].CLASS.IMG_RESIZE_ACTIVE);
    if (!imgList || imgList.length === 0) {
        return;
    }
    var i;
    for (i = imgList.length - 1; i >= 0; i--) {
        _domUtilsDomExtend2['default'].removeClass(imgList[i], _commonConst2['default'].CLASS.IMG_RESIZE_ACTIVE);
    }
}

function removeHandles() {
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    _event.unbindContainer(container);
    container.parentNode.removeChild(container);
}

function initImage(img) {
    if (!img) {
        return;
    }
    removeImgAttributes();
    _domUtilsDomExtend2['default'].addClass(img, _commonConst2['default'].CLASS.IMG_RESIZE_ACTIVE);
    img.attributes[_commonConst2['default'].ATTR.IMG_RATE] = img.width / img.height;
}

function canDragResize(img) {
    if (!img) return false;
    //
    var className = img.getAttribute('class');
    if (className && -1 != className.indexOf(_commonConst2['default'].CLASS.IMG_NOT_DRAG)) return false;
    //
    return true;
}

function showHandles(show) {
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    container.style.display = show ? 'block' : 'none';

    if (!show) {
        clearHandles();
    }
}
function scaleImgSize(rate, widthDraged, heightDraged, img) {
    if (!img) return;
    //
    var widthSized = heightDraged * rate;
    var heightSized = widthDraged / rate;
    //
    if (widthSized < widthDraged) widthSized = widthDraged;else heightSized = heightDraged;
    //
    img.width = widthSized;
    img.height = heightSized;
}

var _event = {
    bind: function bind() {
        _event.unbind();
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.BEFORE_GET_DOCHTML, _event.handler.beforeGetDocHtml);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
    },
    bindContainer: function bindContainer(container) {
        _event.unbindContainer(container);
        container.addEventListener('mousedown', _event.handler.onContainerMouseDown);
    },
    unbind: function unbind() {
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
    },
    unbindContainer: function unbindContainer(container) {
        container.removeEventListener('mousedown', _event.handler.onContainerMouseDown);
    },
    handler: {
        beforeGetDocHtml: function beforeGetDocHtml() {
            clearHandles();
        },
        onKeyDown: function onKeyDown() {
            showHandles(false);
        },
        onContainerMouseDown: function onContainerMouseDown(e) {
            var elm = e.target || e.srcElement;
            resizingHanlde = elm.getAttribute('data-type');
            var img = _commonEnv2['default'].doc.querySelector('.' + _commonConst2['default'].CLASS.IMG_RESIZE_ACTIVE);
            var mousex, mousey, offset;
            if (!img) {
                return;
            }

            mousex = e.pageX;
            mousey = e.pageY;
            offset = _domUtilsDomExtend2['default'].getOffset(img);
            //
            switch (resizingHanlde) {
                case 'lt':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = offset.top - mousey;
                    //
                    oppCornerX = offset.left + img.width;
                    oppCornerY = offset.top + img.height;
                    //
                    cursor = 'nw-resize';
                    break;
                case 'tm':
                    startOffsetX = undefined;
                    startOffsetY = offset.top - mousey;
                    //
                    cursor = 'n-resize';

                    break;
                case 'rt':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = offset.top - mousey;
                    //
                    oppCornerX = offset.left;
                    oppCornerY = offset.top + img.height;
                    //
                    cursor = 'ne-resize';
                    break;
                case 'rm':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = undefined;
                    //
                    cursor = 'e-resize';
                    break;
                case 'rb':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    cursor = 'se-resize';
                    break;
                case 'bm':
                    startOffsetX = undefined;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    oppCornerX = offset.left / 2;
                    oppCornerY = offset.top;
                    //
                    cursor = 's-resize';
                    break;
                case 'lb':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    oppCornerX = offset.left + img.width;
                    oppCornerY = offset.top;
                    //
                    cursor = 'sw-resize';
                    break;
                case 'lm':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = undefined;
                    //
                    cursor = 'w-resize';
                    break;
            }
            _commonUtils2['default'].stopEvent(e);
        },
        onMouseDown: function onMouseDown() {
            showHandles(false);
            removeImgAttributes();
        },
        onMouseMove: function onMouseMove(e) {
            var img = _commonEnv2['default'].doc.querySelector('.' + _commonConst2['default'].CLASS.IMG_RESIZE_ACTIVE);
            var offset, mousex, mousey;
            if (!img) {
                return;
            }
            offset = _domUtilsDomExtend2['default'].getOffset(img);
            //
            if (resizingHanlde) {
                //
                mousex = e.pageX;
                mousey = e.pageY;
                //
                _commonEnv2['default'].doc.body.style.cursor = cursor;
                // console.log('mousex: ' + mousex + ', mousey: ' + mousey);
                // console.log('lastMousex: ' + lastMousex + ', lastMousey: ' + lastMousey);
                var rate;
                var widthDraged;
                var heightDraged;
                var widthSized;
                var heightSized;
                //
                if (!lastMousex || !lastMousey) {
                    lastMousex = mousex;
                    lastMousey = mousey;
                }
                //
                switch (resizingHanlde) {
                    case 'tm':
                        img.width = img.width;
                        if (mousey < offset.top) {
                            img.height += lastMousey - mousey;
                        } else {
                            heightSized = img.height - (mousey - lastMousey) - startOffsetY;
                            img.height = heightSized < 0 ? 0 : heightSized;
                        }
                        break;
                    case 'rm':
                        widthSized = mousex - offset.left - startOffsetX;
                        img.width = widthSized < 0 ? 0 : widthSized;
                        img.height = img.height;
                        img.attributes[_commonConst2['default'].ATTR.IMG_RATE] = img.width / img.height;
                        break;
                    case 'bm':
                        img.width = img.width;
                        heightSized = mousey - oppCornerY - startOffsetY;
                        img.height = heightSized < 0 ? 0 : heightSized;
                        img.attributes[_commonConst2['default'].ATTR.IMG_RATE] = img.width / img.height;
                        break;
                    case 'lm':
                        img.height = img.height;
                        if (mousex < offset.left) {
                            img.width += lastMousex - mousex;
                        } else {
                            widthSized = img.width - (mousex - lastMousex) - startOffsetX;
                            img.width = widthSized < 0 ? 0 : widthSized;
                        }
                        break;
                    case 'lt':
                        rate = Number(img.attributes[_commonConst2['default'].ATTR.IMG_RATE]);
                        //
                        widthDraged = oppCornerX - mousex;
                        heightDraged = oppCornerY - mousey;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'rt':
                        rate = Number(img.attributes[_commonConst2['default'].ATTR.IMG_RATE]);
                        //
                        widthDraged = mousex - oppCornerX;
                        heightDraged = oppCornerY - mousey;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'lb':
                        rate = Number(img.attributes[_commonConst2['default'].ATTR.IMG_RATE]);
                        //
                        widthDraged = oppCornerX - mousex;
                        heightDraged = mousey - oppCornerY;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'rb':
                        rate = Number(img.attributes[_commonConst2['default'].ATTR.IMG_RATE]);
                        // console.log('mousex: ' + mousex + 'mousey: ' + mousey);
                        widthDraged = mousex - offset.left;
                        heightDraged = mousey - offset.top;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        //
                        // console.log('rate: ' + rate + ', ' + 'widthDraged: ' + widthDraged + ', ' + 'heightDraged: ' + heightDraged + ', ' + 'widthSized: ' +
                        // 	widthSized + ', ' + 'heightSized: ' + heightSized);
                        break;
                }
                //
                if (img.style.cssText) {
                    var cssText = img.style.cssText;
                    cssText = cssText.replace(/width:\s*\d+.?\d+px;?/ig, 'width: ' + img.width + 'px').replace(/height:\s*\d+.?\d+px;?/ig, 'height: ' + img.height + 'px');
                    //
                    img.style.cssText = cssText;
                }
                //
                lastMousex = mousex;
                lastMousey = mousey;

                resetHandlesSize(img);
                _commonEnv2['default'].event.call(_commonConst2['default'].EVENT.UPDATE_RENDER, null);
                //
                // TODO pc统一使用 editor 的 修改判断后可删除此逻辑
                if (_commonEnv2['default'].win.WizChromeBrowser) {
                    _commonEnv2['default'].win.WizChromeBrowser.OnDomModified();
                }
            }
        },
        onMouseUp: function onMouseUp(e) {
            var elm = e.target || e.srcElement;
            if (elm && elm.tagName && elm.tagName.toLowerCase() == 'img') {
                initImageDragResize(elm);
                //
            }
            //
            resizingHanlde = '';
            //
            lastMousex = undefined;
            lastMousey = undefined;
            //
            oppCornerX = undefined;
            oppCornerY = undefined;
            //
            startOffsetX = undefined;
            startOffsetY = undefined;
            //
            _commonEnv2['default'].doc.body.style.cursor = cursorOri;
        }
    }
};

var imgResize = {
    init: init,
    bind: _event.bind,
    unbind: _event.unbind
};

exports['default'] = imgResize;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10,"./../domUtils/domExtend":15,"./../rangeUtils/rangeExtend":23}],17:[function(require,module,exports){
/**
 * img 操作基本方法集合
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('./../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('./../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

var _imgResize = require('./imgResize');

var _imgResize2 = _interopRequireDefault(_imgResize);

var imgUtils = {
    on: function on() {
        _imgResize2['default'].init();
        _imgResize2['default'].bind();
    },
    off: function off() {
        _imgResize2['default'].unbind();
    },
    getAll: function getAll(onlyLocal) {
        var images = _commonEnv2['default'].doc.images,
            img,
            imageSrcs = [],
            tmp = {},
            src;
        for (img in images) {
            if (images.hasOwnProperty(img)) {
                //有特殊字符的文件名， 得到src 时是被转义后的名字，所以必须 decode 处理
                src = decodeURIComponent(images[img].src);
                if (imgFilter(images[img], onlyLocal) && !tmp[src]) {
                    imageSrcs.push(src);
                    tmp[src] = true;
                }
            }
        }
        return imageSrcs;
    },
    getImageSize: function getImageSize(imgSrc) {
        var newImg = new Image();
        newImg.src = imgSrc;
        var height = newImg.height;
        var width = newImg.width;
        return { width: width, height: height };
    },
    getImageData: function getImageData(img) {
        var size = imgUtils.getImageSize(img.src);
        // Create an empty canvas element
        var canvas = _commonEnv2['default'].doc.createElement("canvas");
        canvas.width = size.width;
        canvas.height = size.height;

        // Copy the image contents to the canvas
        var ctx = canvas.getContext("2d");
        ctx.drawImage(img, 0, 0);

        // Get the data-URL formatted image
        // Firefox supports PNG and JPEG. You could check img.src to
        // guess the original format, but be aware the using "image/jpg"
        // will re-encode the image.
        var dataURL = canvas.toDataURL("image/png");

        return dataURL.replace(/^data:image\/(png|jpg);base64,/, "");
    },
    makeAttachmentHtml: function makeAttachmentHtml(guid, imgPath) {
        return '<div style="margin: 15px auto;"><a href="wiz:open_attachment?guid=' + guid + '"><img src="' + imgPath + '" style="width: 280px; height:auto;"></a></div><div><br/></div>';
    },
    makeDomByPath: function makeDomByPath(imgPath) {
        var result = [],
            paths = [],
            main,
            img,
            i,
            j;
        if (imgPath.indexOf('*')) {
            paths = imgPath.split("*");
        } else {
            paths.push(imgPath);
        }

        for (i = 0, j = paths.length; i < j; i++) {
            main = _commonEnv2['default'].doc.createElement("div");
            result.push(main);

            img = _commonEnv2['default'].doc.createElement("img");
            img.src = paths[i];
            img.style.maxWidth = '100%';
            main.insertBefore(img, null);
        }

        main = _commonEnv2['default'].doc.createElement("div");
        main.insertBefore(_commonEnv2['default'].doc.createElement("br"), null);
        result.push(main);
        return result;
    }
};

function imgFilter(img, onlyLocal) {
    if (!img || img.className && img.className.indexOf('wiz-todo') > -1) {
        //checklist 的图片不进行获取
        return false;
    }
    var path = img.src;
    if (!path) {
        return false;
    }
    var rLocal = /^(http|https|ftp):/,
        rNoBase64 = /^(data):/,
        result;

    result = !rNoBase64.test(path);
    if (!result || !onlyLocal) {
        return result;
    }
    return !rLocal.test(path);
}

exports['default'] = imgUtils;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10,"./../domUtils/domExtend":15,"./../rangeUtils/rangeExtend":23,"./imgResize":16}],18:[function(require,module,exports){
"use strict";
Object.defineProperty(exports, "__esModule", {
            value: true
});
var Markdown;

if (typeof exports === "object" && typeof require === "function") // we're in a CommonJS (e.g. Node.js) module
            Markdown = exports;else Markdown = {};

// The following text is included for historical reasons, but should
// be taken with a pinch of salt; it's not all true anymore.

//
// Wherever possible, Showdown is a straight, line-by-line port
// of the Perl version of Markdown.
//
// This is not a normal parser design; it's basically just a
// series of string substitutions.  It's hard to read and
// maintain this way,  but keeping Showdown close to the original
// design makes it easier to port new features.
//
// More importantly, Showdown behaves like markdown.pl in most
// edge cases.  So web applications can do client-side preview
// in Javascript, and then build identical HTML on the server.
//
// This port needs the new RegExp functionality of ECMA 262,
// 3rd Edition (i.e. Javascript 1.5).  Most modern web browsers
// should do fine.  Even with the new regular expression features,
// We do a lot of work to emulate Perl's regex functionality.
// The tricky changes in this file mostly have the "attacklab:"
// label.  Major or self-explanatory changes don't.
//
// Smart diff tools like Araxis Merge will be able to match up
// this file with markdown.pl in a useful way.  A little tweaking
// helps: in a copy of markdown.pl, replace "#" with "//" and
// replace "$text" with "text".  Be sure to ignore whitespace
// and line endings.
//

//
// Usage:
//
//   var text = "Markdown *rocks*.";
//
//   var converter = new Markdown.Converter();
//   var html = converter.makeHtml(text);
//
//   alert(html);
//
// Note: move the sample code to the bottom of this
// file before uncommenting it.
//

(function () {

            function identity(x) {
                        return x;
            }
            function returnFalse(x) {
                        return false;
            }

            function HookCollection() {}

            HookCollection.prototype = {

                        chain: function chain(hookname, func) {
                                    var original = this[hookname];
                                    if (!original) throw new Error("unknown hook " + hookname);

                                    if (original === identity) this[hookname] = func;else this[hookname] = function (text) {
                                                var args = Array.prototype.slice.call(arguments, 0);
                                                args[0] = original.apply(null, args);
                                                return func.apply(null, args);
                                    };
                        },
                        set: function set(hookname, func) {
                                    if (!this[hookname]) throw new Error("unknown hook " + hookname);
                                    this[hookname] = func;
                        },
                        addNoop: function addNoop(hookname) {
                                    this[hookname] = identity;
                        },
                        addFalse: function addFalse(hookname) {
                                    this[hookname] = returnFalse;
                        }
            };

            Markdown.HookCollection = HookCollection;

            // g_urls and g_titles allow arbitrary user-entered strings as keys. This
            // caused an exception (and hence stopped the rendering) when the user entered
            // e.g. [push] or [__proto__]. Adding a prefix to the actual key prevents this
            // (since no builtin property starts with "s_"). See
            // http://meta.stackexchange.com/questions/64655/strange-wmd-bug
            // (granted, switching from Array() to Object() alone would have left only __proto__
            // to be a problem)
            function SaveHash() {}
            SaveHash.prototype = {
                        set: function set(key, value) {
                                    this["s_" + key] = value;
                        },
                        get: function get(key) {
                                    return this["s_" + key];
                        }
            };

            Markdown.Converter = function (OPTIONS) {
                        var pluginHooks = this.hooks = new HookCollection();

                        // given a URL that was encountered by itself (without markup), should return the link text that's to be given to this link
                        pluginHooks.addNoop("plainLinkText");

                        // called with the orignal text as given to makeHtml. The result of this plugin hook is the actual markdown source that will be cooked
                        pluginHooks.addNoop("preConversion");

                        // called with the text once all normalizations have been completed (tabs to spaces, line endings, etc.), but before any conversions have
                        pluginHooks.addNoop("postNormalization");

                        // Called with the text before / after creating block elements like code blocks and lists. Note that this is called recursively
                        // with inner content, e.g. it's called with the full text, and then only with the content of a blockquote. The inner
                        // call will receive outdented text.
                        pluginHooks.addNoop("preBlockGamut");
                        pluginHooks.addNoop("postBlockGamut");

                        // called with the text of a single block element before / after the span-level conversions (bold, code spans, etc.) have been made
                        pluginHooks.addNoop("preSpanGamut");
                        pluginHooks.addNoop("postSpanGamut");

                        // called with the final cooked HTML code. The result of this plugin hook is the actual output of makeHtml
                        pluginHooks.addNoop("postConversion");

                        //
                        // Private state of the converter instance:
                        //

                        // Global hashes, used by various utility routines
                        var g_urls;
                        var g_titles;
                        var g_html_blocks;

                        // Used to track when we're inside an ordered or unordered list
                        // (see _ProcessListItems() for details):
                        var g_list_level;

                        OPTIONS = OPTIONS || {};
                        var asciify = identity,
                            deasciify = identity;
                        if (OPTIONS.nonAsciiLetters) {

                                    /* In JavaScript regular expressions, \w only denotes [a-zA-Z0-9_].
                                     * That's why there's inconsistent handling e.g. with intra-word bolding
                                     * of Japanese words. That's why we do the following if OPTIONS.nonAsciiLetters
                                     * is true:
                                     *
                                     * Before doing bold and italics, we find every instance
                                     * of a unicode word character in the Markdown source that is not
                                     * matched by \w, and the letter "Q". We take the character's code point
                                     * and encode it in base 51, using the "digits"
                                     *
                                     *     A, B, ..., P, R, ..., Y, Z, a, b, ..., y, z
                                     *
                                     * delimiting it with "Q" on both sides. For example, the source
                                     *
                                     * > In Chinese, the smurfs are called 藍精靈, meaning "blue spirits".
                                     *
                                     * turns into
                                     *
                                     * > In Chinese, the smurfs are called QNIhQQMOIQQOuUQ, meaning "blue spirits".
                                     *
                                     * Since everything that is a letter in Unicode is now a letter (or
                                     * several letters) in ASCII, \w and \b should always do the right thing.
                                     *
                                     * After the bold/italic conversion, we decode again; since "Q" was encoded
                                     * alongside all non-ascii characters (as "QBfQ"), and the conversion
                                     * will not generate "Q", the only instances of that letter should be our
                                     * encoded characters. And since the conversion will not break words, the
                                     * "Q...Q" should all still be in one piece.
                                     *
                                     * We're using "Q" as the delimiter because it's probably one of the
                                     * rarest characters, and also because I can't think of any special behavior
                                     * that would ever be triggered by this letter (to use a silly example, if we
                                     * delimited with "H" on the left and "P" on the right, then "Ψ" would be
                                     * encoded as "HTTP", which may cause special behavior). The latter would not
                                     * actually be a huge issue for bold/italic, but may be if we later use it
                                     * in other places as well.
                                     * */
                                    (function () {
                                                var lettersThatJavaScriptDoesNotKnowAndQ = /[Q\u00aa\u00b5\u00ba\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u02c1\u02c6-\u02d1\u02e0-\u02e4\u02ec\u02ee\u0370-\u0374\u0376-\u0377\u037a-\u037d\u0386\u0388-\u038a\u038c\u038e-\u03a1\u03a3-\u03f5\u03f7-\u0481\u048a-\u0523\u0531-\u0556\u0559\u0561-\u0587\u05d0-\u05ea\u05f0-\u05f2\u0621-\u064a\u0660-\u0669\u066e-\u066f\u0671-\u06d3\u06d5\u06e5-\u06e6\u06ee-\u06fc\u06ff\u0710\u0712-\u072f\u074d-\u07a5\u07b1\u07c0-\u07ea\u07f4-\u07f5\u07fa\u0904-\u0939\u093d\u0950\u0958-\u0961\u0966-\u096f\u0971-\u0972\u097b-\u097f\u0985-\u098c\u098f-\u0990\u0993-\u09a8\u09aa-\u09b0\u09b2\u09b6-\u09b9\u09bd\u09ce\u09dc-\u09dd\u09df-\u09e1\u09e6-\u09f1\u0a05-\u0a0a\u0a0f-\u0a10\u0a13-\u0a28\u0a2a-\u0a30\u0a32-\u0a33\u0a35-\u0a36\u0a38-\u0a39\u0a59-\u0a5c\u0a5e\u0a66-\u0a6f\u0a72-\u0a74\u0a85-\u0a8d\u0a8f-\u0a91\u0a93-\u0aa8\u0aaa-\u0ab0\u0ab2-\u0ab3\u0ab5-\u0ab9\u0abd\u0ad0\u0ae0-\u0ae1\u0ae6-\u0aef\u0b05-\u0b0c\u0b0f-\u0b10\u0b13-\u0b28\u0b2a-\u0b30\u0b32-\u0b33\u0b35-\u0b39\u0b3d\u0b5c-\u0b5d\u0b5f-\u0b61\u0b66-\u0b6f\u0b71\u0b83\u0b85-\u0b8a\u0b8e-\u0b90\u0b92-\u0b95\u0b99-\u0b9a\u0b9c\u0b9e-\u0b9f\u0ba3-\u0ba4\u0ba8-\u0baa\u0bae-\u0bb9\u0bd0\u0be6-\u0bef\u0c05-\u0c0c\u0c0e-\u0c10\u0c12-\u0c28\u0c2a-\u0c33\u0c35-\u0c39\u0c3d\u0c58-\u0c59\u0c60-\u0c61\u0c66-\u0c6f\u0c85-\u0c8c\u0c8e-\u0c90\u0c92-\u0ca8\u0caa-\u0cb3\u0cb5-\u0cb9\u0cbd\u0cde\u0ce0-\u0ce1\u0ce6-\u0cef\u0d05-\u0d0c\u0d0e-\u0d10\u0d12-\u0d28\u0d2a-\u0d39\u0d3d\u0d60-\u0d61\u0d66-\u0d6f\u0d7a-\u0d7f\u0d85-\u0d96\u0d9a-\u0db1\u0db3-\u0dbb\u0dbd\u0dc0-\u0dc6\u0e01-\u0e30\u0e32-\u0e33\u0e40-\u0e46\u0e50-\u0e59\u0e81-\u0e82\u0e84\u0e87-\u0e88\u0e8a\u0e8d\u0e94-\u0e97\u0e99-\u0e9f\u0ea1-\u0ea3\u0ea5\u0ea7\u0eaa-\u0eab\u0ead-\u0eb0\u0eb2-\u0eb3\u0ebd\u0ec0-\u0ec4\u0ec6\u0ed0-\u0ed9\u0edc-\u0edd\u0f00\u0f20-\u0f29\u0f40-\u0f47\u0f49-\u0f6c\u0f88-\u0f8b\u1000-\u102a\u103f-\u1049\u1050-\u1055\u105a-\u105d\u1061\u1065-\u1066\u106e-\u1070\u1075-\u1081\u108e\u1090-\u1099\u10a0-\u10c5\u10d0-\u10fa\u10fc\u1100-\u1159\u115f-\u11a2\u11a8-\u11f9\u1200-\u1248\u124a-\u124d\u1250-\u1256\u1258\u125a-\u125d\u1260-\u1288\u128a-\u128d\u1290-\u12b0\u12b2-\u12b5\u12b8-\u12be\u12c0\u12c2-\u12c5\u12c8-\u12d6\u12d8-\u1310\u1312-\u1315\u1318-\u135a\u1380-\u138f\u13a0-\u13f4\u1401-\u166c\u166f-\u1676\u1681-\u169a\u16a0-\u16ea\u1700-\u170c\u170e-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176c\u176e-\u1770\u1780-\u17b3\u17d7\u17dc\u17e0-\u17e9\u1810-\u1819\u1820-\u1877\u1880-\u18a8\u18aa\u1900-\u191c\u1946-\u196d\u1970-\u1974\u1980-\u19a9\u19c1-\u19c7\u19d0-\u19d9\u1a00-\u1a16\u1b05-\u1b33\u1b45-\u1b4b\u1b50-\u1b59\u1b83-\u1ba0\u1bae-\u1bb9\u1c00-\u1c23\u1c40-\u1c49\u1c4d-\u1c7d\u1d00-\u1dbf\u1e00-\u1f15\u1f18-\u1f1d\u1f20-\u1f45\u1f48-\u1f4d\u1f50-\u1f57\u1f59\u1f5b\u1f5d\u1f5f-\u1f7d\u1f80-\u1fb4\u1fb6-\u1fbc\u1fbe\u1fc2-\u1fc4\u1fc6-\u1fcc\u1fd0-\u1fd3\u1fd6-\u1fdb\u1fe0-\u1fec\u1ff2-\u1ff4\u1ff6-\u1ffc\u203f-\u2040\u2054\u2071\u207f\u2090-\u2094\u2102\u2107\u210a-\u2113\u2115\u2119-\u211d\u2124\u2126\u2128\u212a-\u212d\u212f-\u2139\u213c-\u213f\u2145-\u2149\u214e\u2183-\u2184\u2c00-\u2c2e\u2c30-\u2c5e\u2c60-\u2c6f\u2c71-\u2c7d\u2c80-\u2ce4\u2d00-\u2d25\u2d30-\u2d65\u2d6f\u2d80-\u2d96\u2da0-\u2da6\u2da8-\u2dae\u2db0-\u2db6\u2db8-\u2dbe\u2dc0-\u2dc6\u2dc8-\u2dce\u2dd0-\u2dd6\u2dd8-\u2dde\u2e2f\u3005-\u3006\u3031-\u3035\u303b-\u303c\u3041-\u3096\u309d-\u309f\u30a1-\u30fa\u30fc-\u30ff\u3105-\u312d\u3131-\u318e\u31a0-\u31b7\u31f0-\u31ff\u3400-\u4db5\u4e00-\u9fc3\ua000-\ua48c\ua500-\ua60c\ua610-\ua62b\ua640-\ua65f\ua662-\ua66e\ua67f-\ua697\ua717-\ua71f\ua722-\ua788\ua78b-\ua78c\ua7fb-\ua801\ua803-\ua805\ua807-\ua80a\ua80c-\ua822\ua840-\ua873\ua882-\ua8b3\ua8d0-\ua8d9\ua900-\ua925\ua930-\ua946\uaa00-\uaa28\uaa40-\uaa42\uaa44-\uaa4b\uaa50-\uaa59\uac00-\ud7a3\uf900-\ufa2d\ufa30-\ufa6a\ufa70-\ufad9\ufb00-\ufb06\ufb13-\ufb17\ufb1d\ufb1f-\ufb28\ufb2a-\ufb36\ufb38-\ufb3c\ufb3e\ufb40-\ufb41\ufb43-\ufb44\ufb46-\ufbb1\ufbd3-\ufd3d\ufd50-\ufd8f\ufd92-\ufdc7\ufdf0-\ufdfb\ufe33-\ufe34\ufe4d-\ufe4f\ufe70-\ufe74\ufe76-\ufefc\uff10-\uff19\uff21-\uff3a\uff3f\uff41-\uff5a\uff66-\uffbe\uffc2-\uffc7\uffca-\uffcf\uffd2-\uffd7\uffda-\uffdc]/g;
                                                var cp_Q = "Q".charCodeAt(0);
                                                var cp_A = "A".charCodeAt(0);
                                                var cp_Z = "Z".charCodeAt(0);
                                                var dist_Za = "a".charCodeAt(0) - cp_Z - 1;

                                                asciify = function (text) {
                                                            return text.replace(lettersThatJavaScriptDoesNotKnowAndQ, function (m) {
                                                                        var c = m.charCodeAt(0);
                                                                        var s = "";
                                                                        var v;
                                                                        while (c > 0) {
                                                                                    v = c % 51 + cp_A;
                                                                                    if (v >= cp_Q) v++;
                                                                                    if (v > cp_Z) v += dist_Za;
                                                                                    s = String.fromCharCode(v) + s;
                                                                                    c = c / 51 | 0;
                                                                        }
                                                                        return "Q" + s + "Q";
                                                            });
                                                };

                                                deasciify = function (text) {
                                                            return text.replace(/Q([A-PR-Za-z]{1,3})Q/g, function (m, s) {
                                                                        var c = 0;
                                                                        var v;
                                                                        for (var i = 0; i < s.length; i++) {
                                                                                    v = s.charCodeAt(i);
                                                                                    if (v > cp_Z) v -= dist_Za;
                                                                                    if (v > cp_Q) v--;
                                                                                    v -= cp_A;
                                                                                    c = c * 51 + v;
                                                                        }
                                                                        return String.fromCharCode(c);
                                                            });
                                                };
                                    })();
                        }

                        var _DoItalicsAndBold = OPTIONS.asteriskIntraWordEmphasis ? _DoItalicsAndBold_AllowIntrawordWithAsterisk : _DoItalicsAndBoldStrict;

                        this.makeHtml = function (text) {

                                    //
                                    // Main function. The order in which other subs are called here is
                                    // essential. Link and image substitutions need to happen before
                                    // _EscapeSpecialCharsWithinTagAttributes(), so that any *'s or _'s in the <a>
                                    // and <img> tags get encoded.
                                    //

                                    // This will only happen if makeHtml on the same converter instance is called from a plugin hook.
                                    // Don't do that.
                                    if (g_urls) throw new Error("Recursive call to converter.makeHtml");

                                    // Create the private state objects.
                                    g_urls = new SaveHash();
                                    g_titles = new SaveHash();
                                    g_html_blocks = [];
                                    g_list_level = 0;

                                    text = pluginHooks.preConversion(text);

                                    // attacklab: Replace ~ with ~T
                                    // This lets us use tilde as an escape char to avoid md5 hashes
                                    // The choice of character is arbitray; anything that isn't
                                    // magic in Markdown will work.
                                    text = text.replace(/~/g, "~T");

                                    // attacklab: Replace $ with ~D
                                    // RegExp interprets $ as a special character
                                    // when it's in a replacement string
                                    text = text.replace(/\$/g, "~D");

                                    // Standardize line endings
                                    text = text.replace(/\r\n/g, "\n"); // DOS to Unix
                                    text = text.replace(/\r/g, "\n"); // Mac to Unix

                                    // Make sure text begins and ends with a couple of newlines:
                                    text = "\n\n" + text + "\n\n";

                                    // Convert all tabs to spaces.
                                    text = _Detab(text);

                                    // Strip any lines consisting only of spaces and tabs.
                                    // This makes subsequent regexen easier to write, because we can
                                    // match consecutive blank lines with /\n+/ instead of something
                                    // contorted like /[ \t]*\n+/ .
                                    text = text.replace(/^[ \t]+$/mg, "");

                                    text = pluginHooks.postNormalization(text);

                                    // Turn block-level HTML blocks into hash entries
                                    text = _HashHTMLBlocks(text);

                                    // Strip link definitions, store in hashes.
                                    text = _StripLinkDefinitions(text);

                                    text = _RunBlockGamut(text);

                                    text = _UnescapeSpecialChars(text);

                                    // attacklab: Restore dollar signs
                                    text = text.replace(/~D/g, "$$");

                                    // attacklab: Restore tildes
                                    text = text.replace(/~T/g, "~");

                                    text = pluginHooks.postConversion(text);

                                    g_html_blocks = g_titles = g_urls = null;

                                    return text;
                        };

                        function _StripLinkDefinitions(text) {
                                    //
                                    // Strips link definitions from text, stores the URLs and titles in
                                    // hash references.
                                    //

                                    // Link defs are in the form: ^[id]: url "optional title"

                                    /*
                                     text = text.replace(/
                                     ^[ ]{0,3}\[([^\[\]]+)\]:  // id = $1  attacklab: g_tab_width - 1
                                     [ \t]*
                                     \n?                 // maybe *one* newline
                                     [ \t]*
                                     <?(\S+?)>?          // url = $2
                                     (?=\s|$)            // lookahead for whitespace instead of the lookbehind removed below
                                     [ \t]*
                                     \n?                 // maybe one newline
                                     [ \t]*
                                     (                   // (potential) title = $3
                                     (\n*)           // any lines skipped = $4 attacklab: lookbehind removed
                                     [ \t]+
                                     ["(]
                                     (.+?)           // title = $5
                                     [")]
                                     [ \t]*
                                     )?                  // title is optional
                                     (\n+)             // subsequent newlines = $6, capturing because they must be put back if the potential title isn't an actual title
                                     /gm, function(){...});
                                     */

                                    text = text.replace(/^[ ]{0,3}\[([^\[\]]+)\]:[ \t]*\n?[ \t]*<?(\S+?)>?(?=\s|$)[ \t]*\n?[ \t]*((\n*)["(](.+?)[")][ \t]*)?(\n+)/gm, function (wholeMatch, m1, m2, m3, m4, m5, m6) {
                                                m1 = m1.toLowerCase();
                                                g_urls.set(m1, _EncodeAmpsAndAngles(m2)); // Link IDs are case-insensitive
                                                if (m4) {
                                                            // Oops, found blank lines, so it's not a title.
                                                            // Put back the parenthetical statement we stole.
                                                            return m3 + m6;
                                                } else if (m5) {
                                                            g_titles.set(m1, m5.replace(/"/g, "&quot;"));
                                                }

                                                // Completely remove the definition from the text
                                                return "";
                                    });

                                    return text;
                        }

                        function _HashHTMLBlocks(text) {

                                    // Hashify HTML blocks:
                                    // We only want to do this for block-level HTML tags, such as headers,
                                    // lists, and tables. That's because we still want to wrap <p>s around
                                    // "paragraphs" that are wrapped in non-block-level tags, such as anchors,
                                    // phrase emphasis, and spans. The list of tags we're looking for is
                                    // hard-coded:
                                    var block_tags_a = "p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math|ins|del";
                                    var block_tags_b = "p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math";

                                    // First, look for nested blocks, e.g.:
                                    //   <div>
                                    //     <div>
                                    //     tags for inner block must be indented.
                                    //     </div>
                                    //   </div>
                                    //
                                    // The outermost tags must start at the left margin for this to match, and
                                    // the inner nested divs must be indented.
                                    // We need to do this before the next, more liberal match, because the next
                                    // match will start at the first `<div>` and stop at the first `</div>`.

                                    // attacklab: This regex can be expensive when it fails.

                                    /*
                                     text = text.replace(/
                                     (                       // save in $1
                                     ^                   // start of line  (with /m)
                                     <($block_tags_a)    // start tag = $2
                                     \b                  // word break
                                     // attacklab: hack around khtml/pcre bug...
                                     [^\r]*?\n           // any number of lines, minimally matching
                                     </\2>               // the matching end tag
                                     [ \t]*              // trailing spaces/tabs
                                     (?=\n+)             // followed by a newline
                                     )                       // attacklab: there are sentinel newlines at end of document
                                     /gm,function(){...}};
                                     */
                                    text = text.replace(/^(<(p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math|ins|del)\b[^\r]*?\n<\/\2>[ \t]*(?=\n+))/gm, hashMatch);

                                    //
                                    // Now match more liberally, simply from `\n<tag>` to `</tag>\n`
                                    //

                                    /*
                                     text = text.replace(/
                                     (                       // save in $1
                                     ^                   // start of line  (with /m)
                                     <($block_tags_b)    // start tag = $2
                                     \b                  // word break
                                     // attacklab: hack around khtml/pcre bug...
                                     [^\r]*?             // any number of lines, minimally matching
                                     .*</\2>             // the matching end tag
                                     [ \t]*              // trailing spaces/tabs
                                     (?=\n+)             // followed by a newline
                                     )                       // attacklab: there are sentinel newlines at end of document
                                     /gm,function(){...}};
                                     */
                                    text = text.replace(/^(<(p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math)\b[^\r]*?.*<\/\2>[ \t]*(?=\n+)\n)/gm, hashMatch);

                                    // Special case just for <hr />. It was easier to make a special case than
                                    // to make the other regex more complicated.

                                    /*
                                     text = text.replace(/
                                     \n                  // Starting after a blank line
                                     [ ]{0,3}
                                     (                   // save in $1
                                     (<(hr)          // start tag = $2
                                     \b          // word break
                                     ([^<>])*?
                                     \/?>)           // the matching end tag
                                     [ \t]*
                                     (?=\n{2,})      // followed by a blank line
                                     )
                                     /g,hashMatch);
                                     */
                                    text = text.replace(/\n[ ]{0,3}((<(hr)\b([^<>])*?\/?>)[ \t]*(?=\n{2,}))/g, hashMatch);

                                    // Special case for standalone HTML comments:

                                    /*
                                     text = text.replace(/
                                     \n\n                                            // Starting after a blank line
                                     [ ]{0,3}                                        // attacklab: g_tab_width - 1
                                     (                                               // save in $1
                                     <!
                                     (--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)   // see http://www.w3.org/TR/html-markup/syntax.html#comments and http://meta.stackexchange.com/q/95256
                                     >
                                     [ \t]*
                                     (?=\n{2,})                                  // followed by a blank line
                                     )
                                     /g,hashMatch);
                                     */
                                    text = text.replace(/\n\n[ ]{0,3}(<!(--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)>[ \t]*(?=\n{2,}))/g, hashMatch);

                                    // PHP and ASP-style processor instructions (<?...?> and <%...%>)

                                    /*
                                     text = text.replace(/
                                     (?:
                                     \n\n            // Starting after a blank line
                                     )
                                     (                   // save in $1
                                     [ ]{0,3}        // attacklab: g_tab_width - 1
                                     (?:
                                     <([?%])     // $2
                                     [^\r]*?
                                     \2>
                                     )
                                     [ \t]*
                                     (?=\n{2,})      // followed by a blank line
                                     )
                                     /g,hashMatch);
                                     */
                                    text = text.replace(/(?:\n\n)([ ]{0,3}(?:<([?%])[^\r]*?\2>)[ \t]*(?=\n{2,}))/g, hashMatch);

                                    return text;
                        }

                        function hashBlock(text) {
                                    text = text.replace(/(^\n+|\n+$)/g, "");
                                    // Replace the element text with a marker ("~KxK" where x is its key)
                                    return "\n\n~K" + (g_html_blocks.push(text) - 1) + "K\n\n";
                        }

                        function hashMatch(wholeMatch, m1) {
                                    return hashBlock(m1);
                        }

                        var blockGamutHookCallback = function blockGamutHookCallback(t) {
                                    return _RunBlockGamut(t);
                        };

                        function _RunBlockGamut(text, doNotUnhash, doNotCreateParagraphs) {
                                    //
                                    // These are all the transformations that form block-level
                                    // tags like paragraphs, headers, and list items.
                                    //

                                    text = pluginHooks.preBlockGamut(text, blockGamutHookCallback);

                                    text = _DoHeaders(text);

                                    // Do Horizontal Rules:
                                    var replacement = "<hr />\n";
                                    text = text.replace(/^[ ]{0,2}([ ]?\*[ ]?){3,}[ \t]*$/gm, replacement);
                                    text = text.replace(/^[ ]{0,2}([ ]?-[ ]?){3,}[ \t]*$/gm, replacement);
                                    text = text.replace(/^[ ]{0,2}([ ]?_[ ]?){3,}[ \t]*$/gm, replacement);

                                    text = _DoLists(text);
                                    text = _DoCodeBlocks(text);
                                    text = _DoBlockQuotes(text);

                                    text = pluginHooks.postBlockGamut(text, blockGamutHookCallback);

                                    // We already ran _HashHTMLBlocks() before, in Markdown(), but that
                                    // was to escape raw HTML in the original Markdown source. This time,
                                    // we're escaping the markup we've just created, so that we don't wrap
                                    // <p> tags around block-level tags.
                                    text = _HashHTMLBlocks(text);

                                    text = _FormParagraphs(text, doNotUnhash, doNotCreateParagraphs);

                                    return text;
                        }

                        function _RunSpanGamut(text) {
                                    //
                                    // These are all the transformations that occur *within* block-level
                                    // tags like paragraphs, headers, and list items.
                                    //

                                    text = pluginHooks.preSpanGamut(text);

                                    text = _DoCodeSpans(text);
                                    text = _EscapeSpecialCharsWithinTagAttributes(text);
                                    text = _EncodeBackslashEscapes(text);

                                    // Process anchor and image tags. Images must come first,
                                    // because ![foo][f] looks like an anchor.
                                    text = _DoImages(text);
                                    text = _DoAnchors(text);

                                    // Make links out of things like `<http://example.com/>`
                                    // Must come after _DoAnchors(), because you can use < and >
                                    // delimiters in inline links like [this](<url>).
                                    text = _DoAutoLinks(text);

                                    text = text.replace(/~P/g, "://"); // put in place to prevent autolinking; reset now

                                    text = _EncodeAmpsAndAngles(text);
                                    text = _DoItalicsAndBold(text);

                                    // Do hard breaks:
                                    text = text.replace(/  +\n/g, " <br>\n");

                                    text = pluginHooks.postSpanGamut(text);

                                    return text;
                        }

                        function _EscapeSpecialCharsWithinTagAttributes(text) {
                                    //
                                    // Within tags -- meaning between < and > -- encode [\ ` * _] so they
                                    // don't conflict with their use in Markdown for code, italics and strong.
                                    //

                                    // Build a regex to find HTML tags and comments.  See Friedl's
                                    // "Mastering Regular Expressions", 2nd Ed., pp. 200-201.

                                    // SE: changed the comment part of the regex

                                    var regex = /(<[a-z\/!$]("[^"]*"|'[^']*'|[^'">])*>|<!(--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)>)/gi;

                                    text = text.replace(regex, function (wholeMatch) {
                                                var tag = wholeMatch.replace(/(.)<\/?code>(?=.)/g, "$1`");
                                                tag = escapeCharacters(tag, wholeMatch.charAt(1) == "!" ? "\\`*_/" : "\\`*_"); // also escape slashes in comments to prevent autolinking there -- http://meta.stackexchange.com/questions/95987
                                                return tag;
                                    });

                                    return text;
                        }

                        function _DoAnchors(text) {

                                    if (text.indexOf("[") === -1) return text;

                                    //
                                    // Turn Markdown link shortcuts into XHTML <a> tags.
                                    //
                                    //
                                    // First, handle reference-style links: [link text] [id]
                                    //

                                    /*
                                     text = text.replace(/
                                     (                           // wrap whole match in $1
                                     \[
                                     (
                                     (?:
                                     \[[^\]]*\]      // allow brackets nested one level
                                     |
                                     [^\[]           // or anything else
                                     )*
                                     )
                                     \]
                                     [ ]?                    // one optional space
                                     (?:\n[ ]*)?             // one optional newline followed by spaces
                                     \[
                                     (.*?)                   // id = $3
                                     \]
                                     )
                                     ()()()()                    // pad remaining backreferences
                                     /g, writeAnchorTag);
                                     */
                                    text = text.replace(/(\[((?:\[[^\]]*\]|[^\[\]])*)\][ ]?(?:\n[ ]*)?\[(.*?)\])()()()()/g, writeAnchorTag);

                                    //
                                    // Next, inline-style links: [link text](url "optional title")
                                    //

                                    /*
                                     text = text.replace(/
                                     (                           // wrap whole match in $1
                                     \[
                                     (
                                     (?:
                                     \[[^\]]*\]      // allow brackets nested one level
                                     |
                                     [^\[\]]         // or anything else
                                     )*
                                     )
                                     \]
                                     \(                      // literal paren
                                     [ \t]*
                                     ()                      // no id, so leave $3 empty
                                     <?(                     // href = $4
                                     (?:
                                     \([^)]*\)       // allow one level of (correctly nested) parens (think MSDN)
                                     |
                                     [^()\s]
                                     )*?
                                     )>?
                                     [ \t]*
                                     (                       // $5
                                     (['"])              // quote char = $6
                                     (.*?)               // Title = $7
                                     \6                  // matching quote
                                     [ \t]*              // ignore any spaces/tabs between closing quote and )
                                     )?                      // title is optional
                                     \)
                                     )
                                     /g, writeAnchorTag);
                                     */

                                    text = text.replace(/(\[((?:\[[^\]]*\]|[^\[\]])*)\]\([ \t]*()<?((?:\([^)]*\)|[^()\s])*?)>?[ \t]*((['"])(.*?)\6[ \t]*)?\))/g, writeAnchorTag);

                                    //
                                    // Last, handle reference-style shortcuts: [link text]
                                    // These must come last in case you've also got [link test][1]
                                    // or [link test](/foo)
                                    //

                                    /*
                                     text = text.replace(/
                                     (                   // wrap whole match in $1
                                     \[
                                     ([^\[\]]+)      // link text = $2; can't contain '[' or ']'
                                     \]
                                     )
                                     ()()()()()          // pad rest of backreferences
                                     /g, writeAnchorTag);
                                     */
                                    text = text.replace(/(\[([^\[\]]+)\])()()()()()/g, writeAnchorTag);

                                    return text;
                        }

                        function writeAnchorTag(wholeMatch, m1, m2, m3, m4, m5, m6, m7) {
                                    if (m7 == undefined) m7 = "";
                                    var whole_match = m1;
                                    var link_text = m2.replace(/:\/\//g, "~P"); // to prevent auto-linking withing the link. will be converted back after the auto-linker runs
                                    var link_id = m3.toLowerCase();
                                    var url = m4;
                                    var title = m7;

                                    if (url == "") {
                                                if (link_id == "") {
                                                            // lower-case and turn embedded newlines into spaces
                                                            link_id = link_text.toLowerCase().replace(/ ?\n/g, " ");
                                                }
                                                url = "#" + link_id;

                                                if (g_urls.get(link_id) != undefined) {
                                                            url = g_urls.get(link_id);
                                                            if (g_titles.get(link_id) != undefined) {
                                                                        title = g_titles.get(link_id);
                                                            }
                                                } else {
                                                            if (whole_match.search(/\(\s*\)$/m) > -1) {
                                                                        // Special case for explicit empty url
                                                                        url = "";
                                                            } else {
                                                                        return whole_match;
                                                            }
                                                }
                                    }
                                    url = attributeSafeUrl(url);

                                    var result = "<a href=\"" + url + "\"";

                                    if (title != "") {
                                                title = attributeEncode(title);
                                                title = escapeCharacters(title, "*_");
                                                result += " title=\"" + title + "\"";
                                    }

                                    result += ">" + link_text + "</a>";

                                    return result;
                        }

                        function _DoImages(text) {

                                    if (text.indexOf("![") === -1) return text;

                                    //
                                    // Turn Markdown image shortcuts into <img> tags.
                                    //

                                    //
                                    // First, handle reference-style labeled images: ![alt text][id]
                                    //

                                    /*
                                     text = text.replace(/
                                     (                   // wrap whole match in $1
                                     !\[
                                     (.*?)           // alt text = $2
                                     \]
                                     [ ]?            // one optional space
                                     (?:\n[ ]*)?     // one optional newline followed by spaces
                                     \[
                                     (.*?)           // id = $3
                                     \]
                                     )
                                     ()()()()            // pad rest of backreferences
                                     /g, writeImageTag);
                                     */
                                    text = text.replace(/(!\[(.*?)\][ ]?(?:\n[ ]*)?\[(.*?)\])()()()()/g, writeImageTag);

                                    //
                                    // Next, handle inline images:  ![alt text](url "optional title")
                                    // Don't forget: encode * and _

                                    /*
                                     text = text.replace(/
                                     (                   // wrap whole match in $1
                                     !\[
                                     (.*?)           // alt text = $2
                                     \]
                                     \s?             // One optional whitespace character
                                     \(              // literal paren
                                     [ \t]*
                                     ()              // no id, so leave $3 empty
                                     <?(\S+?)>?      // src url = $4
                                     [ \t]*
                                     (               // $5
                                     (['"])      // quote char = $6
                                     (.*?)       // title = $7
                                     \6          // matching quote
                                     [ \t]*
                                     )?              // title is optional
                                     \)
                                     )
                                     /g, writeImageTag);
                                     */
                                    text = text.replace(/(!\[(.*?)\]\s?\([ \t]*()<?(\S+?)>?[ \t]*((['"])(.*?)\6[ \t]*)?\))/g, writeImageTag);

                                    return text;
                        }

                        function attributeEncode(text) {
                                    // unconditionally replace angle brackets here -- what ends up in an attribute (e.g. alt or title)
                                    // never makes sense to have verbatim HTML in it (and the sanitizer would totally break it)
                                    return text.replace(/>/g, "&gt;").replace(/</g, "&lt;").replace(/"/g, "&quot;").replace(/'/g, "&#39;");
                        }

                        function writeImageTag(wholeMatch, m1, m2, m3, m4, m5, m6, m7) {
                                    var whole_match = m1;
                                    var alt_text = m2;
                                    var link_id = m3.toLowerCase();
                                    var url = m4;
                                    var title = m7;

                                    if (!title) title = "";

                                    if (url == "") {
                                                if (link_id == "") {
                                                            // lower-case and turn embedded newlines into spaces
                                                            link_id = alt_text.toLowerCase().replace(/ ?\n/g, " ");
                                                }
                                                url = "#" + link_id;

                                                if (g_urls.get(link_id) != undefined) {
                                                            url = g_urls.get(link_id);
                                                            if (g_titles.get(link_id) != undefined) {
                                                                        title = g_titles.get(link_id);
                                                            }
                                                } else {
                                                            return whole_match;
                                                }
                                    }

                                    alt_text = escapeCharacters(attributeEncode(alt_text), "*_[]()");
                                    url = escapeCharacters(url, "*_");
                                    var result = "<img src=\"" + url + "\" alt=\"" + alt_text + "\"";

                                    // attacklab: Markdown.pl adds empty title attributes to images.
                                    // Replicate this bug.

                                    //if (title != "") {
                                    title = attributeEncode(title);
                                    title = escapeCharacters(title, "*_");
                                    result += " title=\"" + title + "\"";
                                    //}

                                    result += " />";

                                    return result;
                        }

                        function _DoHeaders(text) {

                                    // Setext-style headers:
                                    //  Header 1
                                    //  ========
                                    //
                                    //  Header 2
                                    //  --------
                                    //
                                    text = text.replace(/^(.+)[ \t]*\n=+[ \t]*\n+/gm, function (wholeMatch, m1) {
                                                return "<h1>" + _RunSpanGamut(m1) + "</h1>\n\n";
                                    });

                                    text = text.replace(/^(.+)[ \t]*\n-+[ \t]*\n+/gm, function (matchFound, m1) {
                                                return "<h2>" + _RunSpanGamut(m1) + "</h2>\n\n";
                                    });

                                    // atx-style headers:
                                    //  # Header 1
                                    //  ## Header 2
                                    //  ## Header 2 with closing hashes ##
                                    //  ...
                                    //  ###### Header 6
                                    //

                                    /*
                                     text = text.replace(/
                                     ^(\#{1,6})      // $1 = string of #'s
                                     [ \t]*
                                     (.+?)           // $2 = Header text
                                     [ \t]*
                                     \#*             // optional closing #'s (not counted)
                                     \n+
                                     /gm, function() {...});
                                     */

                                    text = text.replace(/^(\#{1,6})[ \t]*(.+?)[ \t]*\#*\n+/gm, function (wholeMatch, m1, m2) {
                                                var h_level = m1.length;
                                                return "<h" + h_level + ">" + _RunSpanGamut(m2) + "</h" + h_level + ">\n\n";
                                    });

                                    return text;
                        }

                        function _DoLists(text, isInsideParagraphlessListItem) {
                                    //
                                    // Form HTML ordered (numbered) and unordered (bulleted) lists.
                                    //

                                    // attacklab: add sentinel to hack around khtml/safari bug:
                                    // http://bugs.webkit.org/show_bug.cgi?id=11231
                                    text += "~0";

                                    // Re-usable pattern to match any entirel ul or ol list:

                                    /*
                                     var whole_list = /
                                     (                                   // $1 = whole list
                                     (                               // $2
                                     [ ]{0,3}                    // attacklab: g_tab_width - 1
                                     ([*+-]|\d+[.])              // $3 = first list item marker
                                     [ \t]+
                                     )
                                     [^\r]+?
                                     (                               // $4
                                     ~0                          // sentinel for workaround; should be $
                                     |
                                     \n{2,}
                                     (?=\S)
                                     (?!                         // Negative lookahead for another list item marker
                                     [ \t]*
                                     (?:[*+-]|\d+[.])[ \t]+
                                     )
                                     )
                                     )
                                     /g
                                     */
                                    var whole_list = /^(([ ]{0,3}([*+-]|\d+[.])[ \t]+)[^\r]+?(~0|\n{2,}(?=\S)(?![ \t]*(?:[*+-]|\d+[.])[ \t]+)))/gm;
                                    var list_type;
                                    if (g_list_level) {
                                                text = text.replace(whole_list, function (wholeMatch, m1, m2) {
                                                            var list = m1;
                                                            list_type = getListType(m2);
                                                            //2015-10-22 wiz：删除起始序列号 支持
                                                            //var first_number;
                                                            //if (list_type === "ol")
                                                            //    first_number = parseInt(m2, 10)

                                                            var result = _ProcessListItems(list, list_type, isInsideParagraphlessListItem);

                                                            // Trim any trailing whitespace, to put the closing `</$list_type>`
                                                            // up on the preceding line, to get it past the current stupid
                                                            // HTML block parser. This is a hack to work around the terrible
                                                            // hack that is the HTML block parser.
                                                            var resultStr = result.list_str.replace(/\s+$/, "");
                                                            var opening = "<" + list_type;
                                                            //if (first_number && first_number !== 1)
                                                            //    opening += " start=\"" + first_number + "\"";
                                                            resultStr = opening + ">" + resultStr + "</" + result.list_type + ">\n";
                                                            list_type = result.list_type;
                                                            return resultStr;
                                                });
                                    } else {
                                                whole_list = /(\n\n|^\n?)(([ ]{0,3}([*+-]|\d+[.])[ \t]+)[^\r]+?(~0|\n{2,}(?=\S)(?![ \t]*(?:[*+-]|\d+[.])[ \t]+)))/gm;
                                                text = text.replace(whole_list, function (wholeMatch, m1, m2, m3) {
                                                            var runup = m1;
                                                            var list = m2;
                                                            list_type = getListType(m3);
                                                            //2015-10-22 wiz：删除起始序列号 支持
                                                            //var first_number;
                                                            //if (list_type === "ol")
                                                            //    first_number = parseInt(m3, 10)

                                                            var result = _ProcessListItems(list, list_type);

                                                            var opening = "<" + list_type;
                                                            //if (first_number && first_number !== 1)
                                                            //    opening += " start=\"" + first_number + "\"";

                                                            var resultStr = runup + opening + ">\n" + result.list_str + "</" + result.list_type + ">\n";
                                                            list_type = result.list_type;
                                                            return resultStr;
                                                });
                                    }

                                    // attacklab: strip sentinel
                                    text = text.replace(/~0/, "");

                                    return text;
                        }

                        var _listItemMarkers = { ol: "\\d+[.]", ul: "[*+-]" };

                        function getListType(str) {
                                    return str.search(/[*+-]/g) > -1 ? "ul" : "ol";
                        }

                        function _ProcessListItems(list_str, list_type, isInsideParagraphlessListItem) {
                                    //
                                    //  Process the contents of a single ordered or unordered list, splitting it
                                    //  into individual list items.
                                    //
                                    //  list_type is either "ul" or "ol".

                                    // The $g_list_level global keeps track of when we're inside a list.
                                    // Each time we enter a list, we increment it; when we leave a list,
                                    // we decrement. If it's zero, we're not in a list anymore.
                                    //
                                    // We do this because when we're not inside a list, we want to treat
                                    // something like this:
                                    //
                                    //    I recommend upgrading to version
                                    //    8. Oops, now this line is treated
                                    //    as a sub-list.
                                    //
                                    // As a single paragraph, despite the fact that the second line starts
                                    // with a digit-period-space sequence.
                                    //
                                    // Whereas when we're inside a list (or sub-list), that line will be
                                    // treated as the start of a sub-list. What a kludge, huh? This is
                                    // an aspect of Markdown's syntax that's hard to parse perfectly
                                    // without resorting to mind-reading. Perhaps the solution is to
                                    // change the syntax rules such that sub-lists must start with a
                                    // starting cardinal number; e.g. "1." or "a.".

                                    g_list_level++;

                                    // trim trailing blank lines:
                                    list_str = list_str.replace(/\n{2,}$/, "\n");

                                    // attacklab: add sentinel to emulate \z
                                    list_str += "~0";

                                    // In the original attacklab showdown, list_type was not given to this function, and anything
                                    // that matched /[*+-]|\d+[.]/ would just create the next <li>, causing this mismatch:
                                    //
                                    //  Markdown          rendered by WMD        rendered by MarkdownSharp
                                    //  ------------------------------------------------------------------
                                    //  1. first          1. first               1. first
                                    //  2. second         2. second              2. second
                                    //  - third           3. third                   * third
                                    //
                                    // We changed this to behave identical to MarkdownSharp. This is the constructed RegEx,
                                    // with {MARKER} being one of \d+[.] or [*+-], depending on list_type:

                                    /*
                                     list_str = list_str.replace(/
                                     (^[ \t]*)                       // leading whitespace = $1
                                     ({MARKER}) [ \t]+               // list marker = $2
                                     ([^\r]+?                        // list item text   = $3
                                     (\n+)
                                     )
                                     (?=
                                     (~0 | \2 ({MARKER}) [ \t]+)
                                     )
                                     /gm, function(){...});
                                     */

                                    //2015-10-22 wiz: 修改 list 的支持规则， 同级的 无序列表 和 有序列表 不会自动处理为 父子关系， 而是生成平级的两个列表；
                                    //var marker = _listItemMarkers[list_type];
                                    //var re = new RegExp("(^[ \\t]*)(" + marker + ")[ \\t]+([^\\r]+?(\\n+))(?=(~0|\\1(" + marker + ")[ \\t]+))", "gm");
                                    var re = new RegExp("(^[ \\t]*)([*+-]|\\d+[.])[ \\t]+([^\\r]+?(\\n+))(?=(~0|\\1([*+-]|\\d+[.])[ \\t]+))", "gm");
                                    var last_item_had_a_double_newline = false;
                                    list_str = list_str.replace(re, function (wholeMatch, m1, m2, m3) {
                                                var item = m3;
                                                var leading_space = m1;
                                                var cur_list_type = getListType(m2);
                                                var ends_with_double_newline = /\n\n$/.test(item);
                                                var contains_double_newline = ends_with_double_newline || item.search(/\n{2,}/) > -1;

                                                var loose = contains_double_newline || last_item_had_a_double_newline;
                                                item = _RunBlockGamut(_Outdent(item), /* doNotUnhash = */true, /* doNotCreateParagraphs = */!loose);

                                                var itemHtml = '';
                                                if (cur_list_type != list_type) {
                                                            itemHtml = '</' + list_type + '>\n<' + cur_list_type + '>\n';
                                                            list_type = cur_list_type;
                                                }
                                                itemHtml += "<li>" + item + "</li>\n";

                                                last_item_had_a_double_newline = ends_with_double_newline;
                                                return itemHtml;
                                    });

                                    // attacklab: strip sentinel
                                    list_str = list_str.replace(/~0/g, "");

                                    g_list_level--;
                                    return { list_str: list_str, list_type: list_type };
                        }

                        function _DoCodeBlocks(text) {
                                    //
                                    //  Process Markdown `<pre><code>` blocks.
                                    //

                                    /*
                                     text = text.replace(/
                                     (?:\n\n|^)
                                     (                               // $1 = the code block -- one or more lines, starting with a space/tab
                                     (?:
                                     (?:[ ]{4}|\t)           // Lines must start with a tab or a tab-width of spaces - attacklab: g_tab_width
                                     .*\n+
                                     )+
                                     )
                                     (\n*[ ]{0,3}[^ \t\n]|(?=~0))    // attacklab: g_tab_width
                                     /g ,function(){...});
                                     */

                                    // attacklab: sentinel workarounds for lack of \A and \Z, safari\khtml bug
                                    text += "~0";

                                    text = text.replace(/(?:\n\n|^\n?)((?:(?:[ ]{4}|\t).*\n+)+)(\n*[ ]{0,3}[^ \t\n]|(?=~0))/g, function (wholeMatch, m1, m2) {
                                                var codeblock = m1;
                                                var nextChar = m2;

                                                codeblock = _EncodeCode(_Outdent(codeblock));
                                                codeblock = _Detab(codeblock);
                                                codeblock = codeblock.replace(/^\n+/g, ""); // trim leading newlines
                                                codeblock = codeblock.replace(/\n+$/g, ""); // trim trailing whitespace

                                                codeblock = "<pre><code>" + codeblock + "\n</code></pre>";

                                                return "\n\n" + codeblock + "\n\n" + nextChar;
                                    });

                                    // attacklab: strip sentinel
                                    text = text.replace(/~0/, "");

                                    return text;
                        }

                        function _DoCodeSpans(text) {
                                    //
                                    // * Backtick quotes are used for <code></code> spans.
                                    //
                                    // * You can use multiple backticks as the delimiters if you want to
                                    //   include literal backticks in the code span. So, this input:
                                    //
                                    //      Just type ``foo `bar` baz`` at the prompt.
                                    //
                                    //   Will translate to:
                                    //
                                    //      <p>Just type <code>foo `bar` baz</code> at the prompt.</p>
                                    //
                                    //   There's no arbitrary limit to the number of backticks you
                                    //   can use as delimters. If you need three consecutive backticks
                                    //   in your code, use four for delimiters, etc.
                                    //
                                    // * You can use spaces to get literal backticks at the edges:
                                    //
                                    //      ... type `` `bar` `` ...
                                    //
                                    //   Turns to:
                                    //
                                    //      ... type <code>`bar`</code> ...
                                    //

                                    /*
                                     text = text.replace(/
                                     (^|[^\\`])      // Character before opening ` can't be a backslash or backtick
                                     (`+)            // $2 = Opening run of `
                                     (?!`)           // and no more backticks -- match the full run
                                     (               // $3 = The code block
                                     [^\r]*?
                                     [^`]        // attacklab: work around lack of lookbehind
                                     )
                                     \2              // Matching closer
                                     (?!`)
                                     /gm, function(){...});
                                     */

                                    text = text.replace(/(^|[^\\`])(`+)(?!`)([^\r]*?[^`])\2(?!`)/gm, function (wholeMatch, m1, m2, m3, m4) {
                                                var c = m3;
                                                c = c.replace(/^([ \t]*)/g, ""); // leading whitespace
                                                c = c.replace(/[ \t]*$/g, ""); // trailing whitespace
                                                c = _EncodeCode(c);
                                                c = c.replace(/:\/\//g, "~P"); // to prevent auto-linking. Not necessary in code *blocks*, but in code spans. Will be converted back after the auto-linker runs.
                                                return m1 + "<code>" + c + "</code>";
                                    });

                                    return text;
                        }

                        function _EncodeCode(text) {
                                    //
                                    // Encode/escape certain characters inside Markdown code runs.
                                    // The point is that in code, these characters are literals,
                                    // and lose their special Markdown meanings.
                                    //
                                    // Encode all ampersands; HTML entities are not
                                    // entities within a Markdown code span.
                                    text = text.replace(/&/g, "&amp;");

                                    // Do the angle bracket song and dance:
                                    text = text.replace(/</g, "&lt;");
                                    text = text.replace(/>/g, "&gt;");

                                    // Now, escape characters that are magic in Markdown:
                                    text = escapeCharacters(text, "\*_{}[]\\", false);

                                    // jj the line above breaks this:
                                    //---

                                    //* Item

                                    //   1. Subitem

                                    //            special char: *
                                    //---

                                    return text;
                        }

                        function _DoItalicsAndBoldStrict(text) {

                                    if (text.indexOf("*") === -1 && text.indexOf("_") === -1) return text;

                                    text = asciify(text);

                                    // <strong> must go first:

                                    // (^|[\W_])           Start with a non-letter or beginning of string. Store in \1.
                                    // (?:(?!\1)|(?=^))    Either the next character is *not* the same as the previous,
                                    //                     or we started at the end of the string (in which case the previous
                                    //                     group had zero width, so we're still there). Because the next
                                    //                     character is the marker, this means that if there are e.g. multiple
                                    //                     underscores in a row, we can only match the left-most ones (which
                                    //                     prevents foo___bar__ from getting bolded)
                                    // (\*|_)              The marker character itself, asterisk or underscore. Store in \2.
                                    // \2                  The marker again, since bold needs two.
                                    // (?=\S)              The first bolded character cannot be a space.
                                    // ([^\r]*?\S)         The actual bolded string. At least one character, and it cannot *end*
                                    //                     with a space either. Note that like in many other places, [^\r] is
                                    //                     just a workaround for JS' lack of single-line regexes; it's equivalent
                                    //                     to a . in an /s regex, because the string cannot contain any \r (they
                                    //                     are removed in the normalizing step).
                                    // \2\2                The marker character, twice -- end of bold.
                                    // (?!\2)              Not followed by another marker character (ensuring that we match the
                                    //                     rightmost two in a longer row)...
                                    // (?=[\W_]|$)         ...but by any other non-word character or the end of string.
                                    text = text.replace(/(^|[\W_])(?:(?!\1)|(?=^))(\*|_)\2(?=\S)([^\r]*?\S)\2\2(?!\2)(?=[\W_]|$)/g, "$1<strong>$3</strong>");

                                    // This is almost identical to the <strong> regex, except 1) there's obviously just one marker
                                    // character, and 2) the italicized string cannot contain the marker character.
                                    text = text.replace(/(^|[\W_])(?:(?!\1)|(?=^))(\*|_)(?=\S)((?:(?!\2)[^\r])*?\S)\2(?!\2)(?=[\W_]|$)/g, "$1<em>$3</em>");

                                    return deasciify(text);
                        }

                        function _DoItalicsAndBold_AllowIntrawordWithAsterisk(text) {

                                    if (text.indexOf("*") === -1 && text.indexOf("_") === -1) return text;

                                    text = asciify(text);

                                    // <strong> must go first:
                                    // (?=[^\r][*_]|[*_])               Optimization only, to find potentially relevant text portions faster. Minimally slower in Chrome, but much faster in IE.
                                    // (                                Store in \1. This is the last character before the delimiter
                                    //     ^                            Either we're at the start of the string (i.e. there is no last character)...
                                    //     |                            ... or we allow one of the following:
                                    //     (?=                          (lookahead; we're not capturing this, just listing legal possibilities)
                                    //         \W__                     If the delimiter is __, then this last character must be non-word non-underscore (extra-word emphasis only)
                                    //         |
                                    //         (?!\*)[\W_]\*\*          If the delimiter is **, then this last character can be non-word non-asterisk (extra-word emphasis)...
                                    //         |
                                    //         \w\*\*\w                 ...or it can be word/underscore, but only if the first bolded character is such a character as well (intra-word emphasis)
                                    //     )
                                    //     [^\r]                        actually capture the character (can't use `.` since it could be \n)
                                    // )
                                    // (\*\*|__)                        Store in \2: the actual delimiter
                                    // (?!\2)                           not followed by the delimiter again (at most one more asterisk/underscore is allowed)
                                    // (?=\S)                           the first bolded character can't be a space
                                    // (                                Store in \3: the bolded string
                                    //
                                    //     (?:|                         Look at all bolded characters except for the last one. Either that's empty, meaning only a single character was bolded...
                                    //       [^\r]*?                    ... otherwise take arbitrary characters, minimally matching; that's all bolded characters except for the last *two*
                                    //       (?!\2)                       the last two characters cannot be the delimiter itself (because that would mean four underscores/asterisks in a row)
                                    //       [^\r]                        capture the next-to-last bolded character
                                    //     )
                                    //     (?=                          lookahead at the very last bolded char and what comes after
                                    //         \S_                      for underscore-bolding, it can be any non-space
                                    //         |
                                    //         \w                       for asterisk-bolding (otherwise the previous alternative would've matched, since \w implies \S), either the last char is word/underscore...
                                    //         |
                                    //         \S\*\*(?:[\W_]|$)        ... or it's any other non-space, but in that case the character *after* the delimiter may not be a word character
                                    //     )
                                    //     .                            actually capture the last character (can use `.` this time because the lookahead ensures \S in all cases)
                                    // )
                                    // (?=                              lookahead; list the legal possibilities for the closing delimiter and its following character
                                    //     __(?:\W|$)                   for underscore-bolding, the following character (if any) must be non-word non-underscore
                                    //     |
                                    //     \*\*(?:[^*]|$)               for asterisk-bolding, any non-asterisk is allowed (note we already ensured above that it's not a word character if the last bolded character wasn't one)
                                    // )
                                    // \2                               actually capture the closing delimiter (and make sure that it matches the opening one)

                                    //2015-10-26 改善对 xxx**(1)**xxx 的支持
                                    //text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W__|(?!\*)[\W_]\*\*|\w\*\*\w)[^\r])(\*\*|__)(?!\2)(?=\S)((?:|[^\r]*?(?!\2)[^\r])(?=\S_|\w|\S\*\*(?:[\W_]|$)).)(?=__(?:\W|$)|\*\*(?:[^*]|$))\2/g,
                                    //    "$1<strong>$3</strong>");
                                    text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W__|(?!\*)[\w\W_]\*\*|\w\*\*\w)[^\r])(\*\*|__)(?!\2)(?=\S)((?:|[^\r]*?(?!\2)[^\r])(?=\S_|\w|.\*\*(?:[\w\W_]|$)).)(?=__(?:\W|$)|\*\*(?:[^*]|$))\2/g, "$1<strong>$3</strong>");

                                    // now <em>:
                                    // (?=[^\r][*_]|[*_])               Optimization, see above.
                                    // (                                Store in \1. This is the last character before the delimiter
                                    //     ^                            Either we're at the start of the string (i.e. there is no last character)...
                                    //     |                            ... or we allow one of the following:
                                    //     (?=                          (lookahead; we're not capturing this, just listing legal possibilities)
                                    //         \W_                      If the delimiter is _, then this last character must be non-word non-underscore (extra-word emphasis only)
                                    //         |
                                    //         (?!\*)                   otherwise, we list two possiblities for * as the delimiter; in either case, the last characters cannot be an asterisk itself
                                    //         (?:
                                    //             [\W_]\*              this last character can be non-word (extra-word emphasis)...
                                    //             |
                                    //             \D\*(?=\w)\D         ...or it can be word (otherwise the first alternative would've matched), but only if
                                    //                                      a) the first italicized character is such a character as well (intra-word emphasis), and
                                    //                                      b) neither character on either side of the asterisk is a digit
                                    //         )
                                    //     )
                                    //     [^\r]                        actually capture the character (can't use `.` since it could be \n)
                                    // )
                                    // (\*|_)                           Store in \2: the actual delimiter
                                    // (?!\2\2\2)                       not followed by more than two more instances of the delimiter
                                    // (?=\S)                           the first italicized character can't be a space
                                    // (                                Store in \3: the italicized string
                                    //     (?:(?!\2)[^\r])*?            arbitrary characters except for the delimiter itself, minimally matching
                                    //     (?=                          lookahead at the very last italicized char and what comes after
                                    //         [^\s_]_                  for underscore-italicizing, it can be any non-space non-underscore
                                    //         |
                                    //         (?=\w)\D\*\D             for asterisk-italicizing, either the last char is word/underscore *and* neither character on either side of the asterisk is a digit...
                                    //         |
                                    //         [^\s*]\*(?:[\W_]|$)      ... or that last char is any other non-space non-asterisk, but then the character after the delimiter (if any) must be non-word
                                    //     )
                                    //     .                            actually capture the last character (can use `.` this time because the lookahead ensures \S in all cases)
                                    // )
                                    // (?=                              lookahead; list the legal possibilities for the closing delimiter and its following character
                                    //     _(?:\W|$)                    for underscore-italicizing, the following character (if any) must be non-word non-underscore
                                    //     |
                                    //     \*(?:[^*]|$)                 for asterisk-italicizing, any non-asterisk is allowed; all other restrictions have already been ensured in the previous lookahead
                                    // )
                                    // \2                               actually capture the closing delimiter (and make sure that it matches the opening one)

                                    //2015-10-26 改善对 xxx*(1)*xxx 的支持
                                    //text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W_|(?!\*)(?:[\W_]\*|\D\*(?=\w)\D))[^\r])(\*|_)(?!\2\2\2)(?=\S)((?:(?!\2)[^\r])*?(?=[^\s_]_|(?=\w)\D\*\D|[^\s*]\*(?:[\W_]|$)).)(?=_(?:\W|$)|\*(?:[^*]|$))\2/g,
                                    //    "$1<em>$3</em>");
                                    text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W_|(?!\*)(?:[\w\W_]\*|\D\*(?=\w)\D))[^\r])(\*|_)(?!\2\2\2)(?=\S)((?:(?!\2)[^\r])*?(?=[^\s_]_|(?=[\w\W])\D\*\D|[^\s*]\*(?:[\w\W_]|$)).)(?=_(?:\W|$)|\*(?:[^*]|$))\2/g, "$1<em>$3</em>");

                                    return deasciify(text);
                        }

                        function _DoBlockQuotes(text) {

                                    /*
                                     text = text.replace(/
                                     (                           // Wrap whole match in $1
                                     (
                                     ^[ \t]*>[ \t]?      // '>' at the start of a line
                                     .+\n                // rest of the first line
                                     (.+\n)*             // subsequent consecutive lines
                                     \n*                 // blanks
                                     )+
                                     )
                                     /gm, function(){...});
                                     */

                                    text = text.replace(/((^[ \t]*>[ \t]?.+\n(.+\n)*\n*)+)/gm, function (wholeMatch, m1) {
                                                var bq = m1;

                                                // attacklab: hack around Konqueror 3.5.4 bug:
                                                // "----------bug".replace(/^-/g,"") == "bug"

                                                bq = bq.replace(/^[ \t]*>[ \t]?/gm, "~0"); // trim one level of quoting

                                                // attacklab: clean up hack
                                                bq = bq.replace(/~0/g, "");

                                                bq = bq.replace(/^[ \t]+$/gm, ""); // trim whitespace-only lines
                                                bq = _RunBlockGamut(bq); // recurse

                                                bq = bq.replace(/(^|\n)/g, "$1  ");
                                                // These leading spaces screw with <pre> content, so we need to fix that:
                                                bq = bq.replace(/(\s*<pre>[^\r]+?<\/pre>)/gm, function (wholeMatch, m1) {
                                                            var pre = m1;
                                                            // attacklab: hack around Konqueror 3.5.4 bug:
                                                            pre = pre.replace(/^  /mg, "~0");
                                                            pre = pre.replace(/~0/g, "");
                                                            return pre;
                                                });

                                                return hashBlock("<blockquote>\n" + bq + "\n</blockquote>");
                                    });
                                    return text;
                        }

                        function _FormParagraphs(text, doNotUnhash, doNotCreateParagraphs) {
                                    //
                                    //  Params:
                                    //    $text - string to process with html <p> tags
                                    //

                                    // Strip leading and trailing lines:
                                    text = text.replace(/^\n+/g, "");
                                    text = text.replace(/\n+$/g, "");

                                    var grafs = text.split(/\n{2,}/g);
                                    var grafsOut = [];

                                    var markerRe = /~K(\d+)K/;

                                    //
                                    // Wrap <p> tags.
                                    //
                                    var end = grafs.length;
                                    for (var i = 0; i < end; i++) {
                                                var str = grafs[i];

                                                // if this is an HTML marker, copy it
                                                if (markerRe.test(str)) {
                                                            grafsOut.push(str);
                                                } else if (/\S/.test(str)) {
                                                            str = _RunSpanGamut(str);
                                                            str = str.replace(/^([ \t]*)/g, doNotCreateParagraphs ? "" : "<p>");
                                                            if (!doNotCreateParagraphs) str += "</p>";
                                                            grafsOut.push(str);
                                                }
                                    }
                                    //
                                    // Unhashify HTML blocks
                                    //
                                    if (!doNotUnhash) {
                                                end = grafsOut.length;
                                                for (var i = 0; i < end; i++) {
                                                            var foundAny = true;
                                                            while (foundAny) {
                                                                        // we may need several runs, since the data may be nested
                                                                        foundAny = false;
                                                                        grafsOut[i] = grafsOut[i].replace(/~K(\d+)K/g, function (wholeMatch, id) {
                                                                                    foundAny = true;
                                                                                    return g_html_blocks[id];
                                                                        });
                                                            }
                                                }
                                    }
                                    return grafsOut.join("\n\n");
                        }

                        function _EncodeAmpsAndAngles(text) {
                                    // Smart processing for ampersands and angle brackets that need to be encoded.

                                    // Ampersand-encoding based entirely on Nat Irons's Amputator MT plugin:
                                    //   http://bumppo.net/projects/amputator/
                                    text = text.replace(/&(?!#?[xX]?(?:[0-9a-fA-F]+|\w+);)/g, "&amp;");

                                    // Encode naked <'s
                                    text = text.replace(/<(?![a-z\/?!]|~D)/gi, "&lt;");

                                    return text;
                        }

                        function _EncodeBackslashEscapes(text) {
                                    //
                                    //   Parameter:  String.
                                    //   Returns:    The string, with after processing the following backslash
                                    //               escape sequences.
                                    //

                                    // attacklab: The polite way to do this is with the new
                                    // escapeCharacters() function:
                                    //
                                    //     text = escapeCharacters(text,"\\",true);
                                    //     text = escapeCharacters(text,"`*_{}[]()>#+-.!",true);
                                    //
                                    // ...but we're sidestepping its use of the (slow) RegExp constructor
                                    // as an optimization for Firefox.  This function gets called a LOT.

                                    text = text.replace(/\\(\\)/g, escapeCharacters_callback);
                                    text = text.replace(/\\([`*_{}\[\]()>#+-.!])/g, escapeCharacters_callback);
                                    return text;
                        }

                        var charInsideUrl = "[-A-Z0-9+&@#/%?=~_|[\\]()!:,.;]",
                            charEndingUrl = "[-A-Z0-9+&@#/%=~_|[\\])]",
                            autoLinkRegex = new RegExp("(=\"|<)?\\b(https?|ftp)(://" + charInsideUrl + "*" + charEndingUrl + ")(?=$|\\W)", "gi"),
                            endCharRegex = new RegExp(charEndingUrl, "i");

                        function handleTrailingParens(wholeMatch, lookbehind, protocol, link, index, str) {

                                    if (/^<[^<>]*(https?|ftp)/.test(str)) {
                                                //避免 html 标签内 属性值的 超链接被替换为 a 标签（例如 img 的src 属性）
                                                return wholeMatch;
                                    }
                                    if (lookbehind) return wholeMatch;
                                    if (link.charAt(link.length - 1) !== ")") return "<" + protocol + link + ">";
                                    var parens = link.match(/[()]/g);
                                    var level = 0;
                                    for (var i = 0; i < parens.length; i++) {
                                                if (parens[i] === "(") {
                                                            if (level <= 0) level = 1;else level++;
                                                } else {
                                                            level--;
                                                }
                                    }
                                    var tail = "";
                                    if (level < 0) {
                                                var re = new RegExp("\\){1," + -level + "}$");
                                                link = link.replace(re, function (trailingParens) {
                                                            tail = trailingParens;
                                                            return "";
                                                });
                                    }
                                    if (tail) {
                                                var lastChar = link.charAt(link.length - 1);
                                                if (!endCharRegex.test(lastChar)) {
                                                            tail = lastChar + tail;
                                                            link = link.substr(0, link.length - 1);
                                                }
                                    }
                                    return "<" + protocol + link + ">" + tail;
                        }

                        function _DoAutoLinks(text) {

                                    // note that at this point, all other URL in the text are already hyperlinked as <a href=""></a>
                                    // *except* for the <http://www.foo.com> case

                                    // automatically add < and > around unadorned raw hyperlinks
                                    // must be preceded by a non-word character (and not by =" or <) and followed by non-word/EOF character
                                    // simulating the lookbehind in a consuming way is okay here, since a URL can neither and with a " nor
                                    // with a <, so there is no risk of overlapping matches.
                                    text = text.replace(autoLinkRegex, handleTrailingParens);

                                    //  autolink anything like <http://example.com>

                                    var replacer = function replacer(wholematch, m1) {
                                                var url = attributeSafeUrl(m1);

                                                return "<a href=\"" + url + "\">" + pluginHooks.plainLinkText(m1) + "</a>";
                                    };
                                    text = text.replace(/<((https?|ftp):[^'">\s]+)>/gi, replacer);

                                    // Email addresses: <address@domain.foo>
                                    /*
                                     text = text.replace(/
                                     <
                                     (?:mailto:)?
                                     (
                                     [-.\w]+
                                     \@
                                     [-a-z0-9]+(\.[-a-z0-9]+)*\.[a-z]+
                                     )
                                     >
                                     /gi, _DoAutoLinks_callback());
                                     */

                                    /* disabling email autolinking, since we don't do that on the server, either
                                     text = text.replace(/<(?:mailto:)?([-.\w]+\@[-a-z0-9]+(\.[-a-z0-9]+)*\.[a-z]+)>/gi,
                                     function(wholeMatch,m1) {
                                     return _EncodeEmailAddress( _UnescapeSpecialChars(m1) );
                                     }
                                     );
                                     */
                                    return text;
                        }

                        function _UnescapeSpecialChars(text) {
                                    //
                                    // Swap back in all the special characters we've hidden.
                                    //
                                    text = text.replace(/~E(\d+)E/g, function (wholeMatch, m1) {
                                                var charCodeToReplace = parseInt(m1);
                                                return String.fromCharCode(charCodeToReplace);
                                    });
                                    return text;
                        }

                        function _Outdent(text) {
                                    //
                                    // Remove one level of line-leading tabs or spaces
                                    //

                                    // attacklab: hack around Konqueror 3.5.4 bug:
                                    // "----------bug".replace(/^-/g,"") == "bug"

                                    text = text.replace(/^(\t|[ ]{1,4})/gm, "~0"); // attacklab: g_tab_width

                                    // attacklab: clean up hack
                                    text = text.replace(/~0/g, "");

                                    return text;
                        }

                        function _Detab(text) {
                                    if (!/\t/.test(text)) return text;

                                    var spaces = ["    ", "   ", "  ", " "],
                                        skew = 0,
                                        v;

                                    return text.replace(/[\n\t]/g, function (match, offset) {
                                                if (match === "\n") {
                                                            skew = offset + 1;
                                                            return match;
                                                }
                                                v = (offset - skew) % 4;
                                                skew = offset + 1;
                                                return spaces[v];
                                    });
                        }

                        //
                        //  attacklab: Utility functions
                        //

                        function attributeSafeUrl(url) {
                                    url = attributeEncode(url);
                                    url = escapeCharacters(url, "*_:()[]");
                                    return url;
                        }

                        function escapeCharacters(text, charsToEscape, afterBackslash) {
                                    // First we have to escape the escape characters so that
                                    // we can build a character class out of them
                                    var regexString = "([" + charsToEscape.replace(/([\[\]\\])/g, "\\$1") + "])";

                                    if (afterBackslash) {
                                                regexString = "\\\\" + regexString;
                                    }

                                    var regex = new RegExp(regexString, "g");
                                    text = text.replace(regex, escapeCharacters_callback);

                                    return text;
                        }

                        function escapeCharacters_callback(wholeMatch, m1) {
                                    var charCodeToEscape = m1.charCodeAt(0);
                                    return "~E" + charCodeToEscape + "E";
                        }
            }; // end of the Markdown.Converter constructor
})();

exports["default"] = Markdown;
module.exports = exports["default"];

},{}],19:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
  value: true
});
var Markdown = {};

(function () {
  // A quick way to make sure we're only keeping span-level tags when we need to.
  // This isn't supposed to be foolproof. It's just a quick way to make sure we
  // keep all span-level tags returned by a pagedown converter. It should allow
  // all span-level tags through, with or without attributes.
  var inlineTags = new RegExp(['^(<\\/?(a|abbr|acronym|applet|area|b|basefont|', 'bdo|big|button|cite|code|del|dfn|em|figcaption|', 'font|i|iframe|img|input|ins|kbd|label|map|', 'mark|meter|object|param|progress|q|ruby|rp|rt|s|', 'samp|script|select|small|span|strike|strong|', 'sub|sup|textarea|time|tt|u|var|wbr)[^>]*>|', '<(br)\\s?\\/?>)$'].join(''), 'i');

  /******************************************************************
   * Utility Functions                                              *
   *****************************************************************/

  // patch for ie7
  if (!Array.indexOf) {
    Array.prototype.indexOf = function (obj) {
      for (var i = 0; i < this.length; i++) {
        if (this[i] == obj) {
          return i;
        }
      }
      return -1;
    };
  }

  function trim(str) {
    return str.replace(/^\s+|\s+$/g, '');
  }

  function rtrim(str) {
    return str.replace(/\s+$/g, '');
  }

  // Remove one level of indentation from text. Indent is 4 spaces.
  function outdent(text) {
    return text.replace(new RegExp('^(\\t|[ ]{1,4})', 'gm'), '');
  }

  function contains(str, substr) {
    return str.indexOf(substr) != -1;
  }

  // Sanitize html, removing tags that aren't in the whitelist
  function sanitizeHtml(html, whitelist) {
    return html.replace(/<[^>]*>?/gi, function (tag) {
      return tag.match(whitelist) ? tag : '';
    });
  }

  // Merge two arrays, keeping only unique elements.
  function union(x, y) {
    var obj = {};
    for (var i = 0; i < x.length; i++) obj[x[i]] = x[i];
    for (i = 0; i < y.length; i++) obj[y[i]] = y[i];
    var res = [];
    for (var k in obj) {
      if (obj.hasOwnProperty(k)) res.push(obj[k]);
    }
    return res;
  }

  // JS regexes don't support \A or \Z, so we add sentinels, as Pagedown
  // does. In this case, we add the ascii codes for start of text (STX) and
  // end of text (ETX), an idea borrowed from:
  // https://github.com/tanakahisateru/js-markdown-extra
  function addAnchors(text) {
    if (text.charAt(0) != '\x02') text = '\x02' + text;
    if (text.charAt(text.length - 1) != '\x03') text = text + '\x03';
    return text;
  }

  // Remove STX and ETX sentinels.
  function removeAnchors(text) {
    if (text.charAt(0) == '\x02') text = text.substr(1);
    if (text.charAt(text.length - 1) == '\x03') text = text.substr(0, text.length - 1);
    return text;
  }

  // Convert markdown within an element, retaining only span-level tags
  function convertSpans(text, extra) {
    return sanitizeHtml(convertAll(text, extra), inlineTags);
  }

  // Convert internal markdown using the stock pagedown converter
  function convertAll(text, extra) {
    var result = extra.blockGamutHookCallback(text);
    // We need to perform these operations since we skip the steps in the converter
    result = unescapeSpecialChars(result);
    result = result.replace(/~D/g, "$$").replace(/~T/g, "~");
    result = extra.previousPostConversion(result);
    return result;
  }

  // Convert escaped special characters
  function processEscapesStep1(text) {
    // Markdown extra adds two escapable characters, `:` and `|`
    return text.replace(/\\\|/g, '~I').replace(/\\:/g, '~i');
  }

  function processEscapesStep2(text) {
    return text.replace(/~I/g, '|').replace(/~i/g, ':');
  }

  // Duplicated from PageDown converter
  function unescapeSpecialChars(text) {
    // Swap back in all the special characters we've hidden.
    text = text.replace(/~E(\d+)E/g, function (wholeMatch, m1) {
      var charCodeToReplace = parseInt(m1);
      return String.fromCharCode(charCodeToReplace);
    });
    return text;
  }

  function slugify(text) {
    return text.toLowerCase().replace(/\s+/g, '-') // Replace spaces with -
    .replace(/[^\w\-]+/g, '') // Remove all non-word chars
    .replace(/\-\-+/g, '-') // Replace multiple - with single -
    .replace(/^-+/, '') // Trim - from start of text
    .replace(/-+$/, ''); // Trim - from end of text
  }

  /*****************************************************************************
   * Markdown.Extra *
   ****************************************************************************/

  Markdown.Extra = function () {
    // For converting internal markdown (in tables for instance).
    // This is necessary since these methods are meant to be called as
    // preConversion hooks, and the Markdown converter passed to init()
    // won't convert any markdown contained in the html tags we return.
    this.converter = null;

    // Stores html blocks we generate in hooks so that
    // they're not destroyed if the user is using a sanitizing converter
    this.hashBlocks = [];

    // Stores footnotes
    this.footnotes = {};
    this.usedFootnotes = [];

    // Special attribute blocks for fenced code blocks and headers enabled.
    this.attributeBlocks = false;

    // Fenced code block options
    this.googleCodePrettify = false;
    this.highlightJs = false;

    // Table options
    this.tableClass = '';

    this.tabWidth = 4;
  };

  Markdown.Extra.init = function (converter, options) {
    // Each call to init creates a new instance of Markdown.Extra so it's
    // safe to have multiple converters, with different options, on a single page
    var extra = new Markdown.Extra();
    var postNormalizationTransformations = [];
    var preBlockGamutTransformations = [];
    var postSpanGamutTransformations = [];
    var postConversionTransformations = ["unHashExtraBlocks"];

    options = options || {};
    options.extensions = options.extensions || ["all"];
    if (contains(options.extensions, "all")) {
      options.extensions = ["tables", "fenced_code_gfm", "def_list", "attr_list", "footnotes", "smartypants", "strikethrough", "newlines"];
    }
    preBlockGamutTransformations.push("wrapHeaders");
    if (contains(options.extensions, "attr_list")) {
      postNormalizationTransformations.push("hashFcbAttributeBlocks");
      preBlockGamutTransformations.push("hashHeaderAttributeBlocks");
      postConversionTransformations.push("applyAttributeBlocks");
      extra.attributeBlocks = true;
    }
    if (contains(options.extensions, "fenced_code_gfm")) {
      // This step will convert fcb inside list items and blockquotes
      preBlockGamutTransformations.push("fencedCodeBlocks");
      // This extra step is to prevent html blocks hashing and link definition/footnotes stripping inside fcb
      postNormalizationTransformations.push("fencedCodeBlocks");
    }
    if (contains(options.extensions, "tables")) {
      preBlockGamutTransformations.push("tables");
    }
    if (contains(options.extensions, "def_list")) {
      preBlockGamutTransformations.push("definitionLists");
    }
    if (contains(options.extensions, "footnotes")) {
      postNormalizationTransformations.push("stripFootnoteDefinitions");
      preBlockGamutTransformations.push("doFootnotes");
      postConversionTransformations.push("printFootnotes");
    }
    if (contains(options.extensions, "smartypants")) {
      postConversionTransformations.push("runSmartyPants");
    }
    if (contains(options.extensions, "strikethrough")) {
      postSpanGamutTransformations.push("strikethrough");
    }
    if (contains(options.extensions, "newlines")) {
      postSpanGamutTransformations.push("newlines");
    }

    converter.hooks.chain("postNormalization", function (text) {
      return extra.doTransform(postNormalizationTransformations, text) + '\n';
    });

    converter.hooks.chain("preBlockGamut", function (text, blockGamutHookCallback) {
      // Keep a reference to the block gamut callback to run recursively
      extra.blockGamutHookCallback = blockGamutHookCallback;
      text = processEscapesStep1(text);
      text = extra.doTransform(preBlockGamutTransformations, text) + '\n';
      text = processEscapesStep2(text);
      return text;
    });

    converter.hooks.chain("postSpanGamut", function (text) {
      return extra.doTransform(postSpanGamutTransformations, text);
    });

    // Keep a reference to the hook chain running before doPostConversion to apply on hashed extra blocks
    extra.previousPostConversion = converter.hooks.postConversion;
    converter.hooks.chain("postConversion", function (text) {
      text = extra.doTransform(postConversionTransformations, text);
      // Clear state vars that may use unnecessary memory
      extra.hashBlocks = [];
      extra.footnotes = {};
      extra.usedFootnotes = [];
      return text;
    });

    if ("highlighter" in options) {
      extra.googleCodePrettify = options.highlighter === 'prettify';
      extra.highlightJs = options.highlighter === 'highlight';
    }

    if ("table_class" in options) {
      extra.tableClass = options.table_class;
    }

    extra.converter = converter;

    // Caller usually won't need this, but it's handy for testing.
    return extra;
  };

  // Do transformations
  Markdown.Extra.prototype.doTransform = function (transformations, text) {
    for (var i = 0; i < transformations.length; i++) text = this[transformations[i]](text);
    return text;
  };

  // Return a placeholder containing a key, which is the block's index in the
  // hashBlocks array. We wrap our output in a <p> tag here so Pagedown won't.
  Markdown.Extra.prototype.hashExtraBlock = function (block) {
    return '\n<p>~X' + (this.hashBlocks.push(block) - 1) + 'X</p>\n';
  };
  Markdown.Extra.prototype.hashExtraInline = function (block) {
    return '~X' + (this.hashBlocks.push(block) - 1) + 'X';
  };

  // Replace placeholder blocks in `text` with their corresponding
  // html blocks in the hashBlocks array.
  Markdown.Extra.prototype.unHashExtraBlocks = function (text) {
    var self = this;

    function recursiveUnHash() {
      var hasHash = false;
      text = text.replace(/(?:<p>)?~X(\d+)X(?:<\/p>)?/g, function (wholeMatch, m1) {
        hasHash = true;
        var key = parseInt(m1, 10);
        return self.hashBlocks[key];
      });
      if (hasHash === true) {
        recursiveUnHash();
      }
    }

    recursiveUnHash();
    return text;
  };

  // Wrap headers to make sure they won't be in def lists
  Markdown.Extra.prototype.wrapHeaders = function (text) {
    function wrap(text) {
      return '\n' + text + '\n';
    }

    text = text.replace(/^.+[ \t]*\n=+[ \t]*\n+/gm, wrap);
    text = text.replace(/^.+[ \t]*\n-+[ \t]*\n+/gm, wrap);
    text = text.replace(/^\#{1,6}[ \t]*.+?[ \t]*\#*\n+/gm, wrap);
    return text;
  };

  /******************************************************************
   * Attribute Blocks                                               *
   *****************************************************************/

  // TODO: use sentinels. Should we just add/remove them in doConversion?
  // TODO: better matches for id / class attributes
  var attrBlock = "\\{[ \\t]*((?:[#.][-_:a-zA-Z0-9]+[ \\t]*)+)\\}";
  var hdrAttributesA = new RegExp("^(#{1,6}.*#{0,6})[ \\t]+" + attrBlock + "[ \\t]*(?:\\n|0x03)", "gm");
  var hdrAttributesB = new RegExp("^(.*)[ \\t]+" + attrBlock + "[ \\t]*\\n" + "(?=[\\-|=]+\\s*(?:\\n|0x03))", "gm"); // underline lookahead
  var fcbAttributes = new RegExp("^(```[ \\t]*[^{\\s]*)[ \\t]+" + attrBlock + "[ \\t]*\\n" + "(?=([\\s\\S]*?)\\n```[ \\t]*(\\n|0x03))", "gm");

  // Extract headers attribute blocks, move them above the element they will be
  // applied to, and hash them for later.
  Markdown.Extra.prototype.hashHeaderAttributeBlocks = function (text) {

    var self = this;

    function attributeCallback(wholeMatch, pre, attr) {
      return '<p>~XX' + (self.hashBlocks.push(attr) - 1) + 'XX</p>\n' + pre + "\n";
    }

    text = text.replace(hdrAttributesA, attributeCallback); // ## headers
    text = text.replace(hdrAttributesB, attributeCallback); // underline headers
    return text;
  };

  // Extract FCB attribute blocks, move them above the element they will be
  // applied to, and hash them for later.
  Markdown.Extra.prototype.hashFcbAttributeBlocks = function (text) {
    // TODO: use sentinels. Should we just add/remove them in doConversion?
    // TODO: better matches for id / class attributes

    var self = this;

    function attributeCallback(wholeMatch, pre, attr) {
      return '<p>~XX' + (self.hashBlocks.push(attr) - 1) + 'XX</p>\n' + pre + "\n";
    }

    return text.replace(fcbAttributes, attributeCallback);
  };

  Markdown.Extra.prototype.applyAttributeBlocks = function (text) {
    var self = this;
    var blockRe = new RegExp('<p>~XX(\\d+)XX</p>[\\s]*' + '(?:<(h[1-6]|pre)(?: +class="(\\S+)")?(>[\\s\\S]*?</\\2>))', "gm");
    text = text.replace(blockRe, function (wholeMatch, k, tag, cls, rest) {
      if (!tag) // no following header or fenced code block.
        return '';

      // get attributes list from hash
      var key = parseInt(k, 10);
      var attributes = self.hashBlocks[key];

      // get id
      var id = attributes.match(/#[^\s#.]+/g) || [];
      var idStr = id[0] ? ' id="' + id[0].substr(1, id[0].length - 1) + '"' : '';

      // get classes and merge with existing classes
      var classes = attributes.match(/\.[^\s#.]+/g) || [];
      for (var i = 0; i < classes.length; i++) // Remove leading dot
      classes[i] = classes[i].substr(1, classes[i].length - 1);

      var classStr = '';
      if (cls) classes = union(classes, [cls]);

      if (classes.length > 0) classStr = ' class="' + classes.join(' ') + '"';

      return "<" + tag + idStr + classStr + rest;
    });

    return text;
  };

  /******************************************************************
   * Tables                                                         *
   *****************************************************************/

  // Find and convert Markdown Extra tables into html.
  Markdown.Extra.prototype.tables = function (text) {
    var self = this;

    var leadingPipe = new RegExp(['^', '[ ]{0,3}', // Allowed whitespace
    '[|]', // Initial pipe
    '(.+)\\n', // $1: Header Row

    '[ ]{0,3}', // Allowed whitespace
    '[|]([ ]*[-:]+[-| :]*)\\n', // $2: Separator

    '(', // $3: Table Body
    '(?:[ ]*[|].*\\n?)*', // Table rows
    ')', '(?:\\n|$)' // Stop at final newline
    ].join(''), 'gm');

    var noLeadingPipe = new RegExp(['^', '[ ]{0,3}', // Allowed whitespace
    '(\\S.*[|].*)\\n', // $1: Header Row

    '[ ]{0,3}', // Allowed whitespace
    '([-:]+[ ]*[|][-| :]*)\\n', // $2: Separator

    '(', // $3: Table Body
    '(?:.*[|].*\\n?)*', // Table rows
    ')', '(?:\\n|$)' // Stop at final newline
    ].join(''), 'gm');

    text = text.replace(leadingPipe, doTable);
    text = text.replace(noLeadingPipe, doTable);

    // $1 = header, $2 = separator, $3 = body
    function doTable(match, header, separator, body, offset, string) {
      // remove any leading pipes and whitespace
      header = header.replace(/^ *[|]/m, '');
      separator = separator.replace(/^ *[|]/m, '');
      body = body.replace(/^ *[|]/gm, '');

      // remove trailing pipes and whitespace
      header = header.replace(/[|] *$/m, '');
      separator = separator.replace(/[|] *$/m, '');
      body = body.replace(/[|] *$/gm, '');

      // determine column alignments
      var alignspecs = separator.split(/ *[|] */);
      var align = [];
      for (var i = 0; i < alignspecs.length; i++) {
        var spec = alignspecs[i];
        if (spec.match(/^ *-+: *$/m)) align[i] = ' align="right"';else if (spec.match(/^ *:-+: *$/m)) align[i] = ' align="center"';else if (spec.match(/^ *:-+ *$/m)) align[i] = ' align="left"';else align[i] = '';
      }

      // TODO: parse spans in header and rows before splitting, so that pipes
      // inside of tags are not interpreted as separators
      var headers = header.split(/ *[|] */);
      var colCount = headers.length;

      // build html
      var cls = self.tableClass ? ' class="' + self.tableClass + '"' : '';
      var html = ['<table', cls, '>\n', '<thead>\n', '<tr>\n'].join('');

      // build column headers.
      for (i = 0; i < colCount; i++) {
        var headerHtml = convertSpans(trim(headers[i]), self);
        html += ["  <th", align[i], ">", headerHtml, "</th>\n"].join('');
      }
      html += "</tr>\n</thead>\n";

      // build rows
      var rows = body.split('\n');
      for (i = 0; i < rows.length; i++) {
        if (rows[i].match(/^\s*$/)) // can apply to final row
          continue;

        // ensure number of rowCells matches colCount
        var rowCells = rows[i].split(/ *[|] */);
        var lenDiff = colCount - rowCells.length;
        for (var j = 0; j < lenDiff; j++) rowCells.push('');

        html += "<tr>\n";
        for (j = 0; j < colCount; j++) {
          var colHtml = convertSpans(trim(rowCells[j]), self);
          html += ["  <td", align[j], ">", colHtml, "</td>\n"].join('');
        }
        html += "</tr>\n";
      }

      html += "</table>\n";

      // replace html with placeholder until postConversion step
      return self.hashExtraBlock(html);
    }

    return text;
  };

  /******************************************************************
   * Footnotes                                                      *
   *****************************************************************/

  // Strip footnote, store in hashes.
  Markdown.Extra.prototype.stripFootnoteDefinitions = function (text) {
    var self = this;

    text = text.replace(/\n[ ]{0,3}\[\^(.+?)\]\:[ \t]*\n?([\s\S]*?)\n{1,2}((?=\n[ ]{0,3}\S)|$)/g, function (wholeMatch, m1, m2) {
      m1 = slugify(m1);
      m2 += "\n";
      m2 = m2.replace(/^[ ]{0,3}/g, "");
      self.footnotes[m1] = m2;
      return "\n";
    });

    return text;
  };

  // Find and convert footnotes references.
  Markdown.Extra.prototype.doFootnotes = function (text) {
    var self = this;
    if (self.isConvertingFootnote === true) {
      return text;
    }

    var footnoteCounter = 0;
    text = text.replace(/\[\^(.+?)\]/g, function (wholeMatch, m1) {
      var id = slugify(m1);
      var footnote = self.footnotes[id];
      if (footnote === undefined) {
        return wholeMatch;
      }
      footnoteCounter++;
      self.usedFootnotes.push(id);
      var html = '<a href="#fn_' + id + '" id="fnref_' + id + '" title="See footnote" class="footnote">' + footnoteCounter + '</a>';
      return self.hashExtraInline(html);
    });

    return text;
  };

  // Print footnotes at the end of the document
  Markdown.Extra.prototype.printFootnotes = function (text) {
    var self = this;

    if (self.usedFootnotes.length === 0) {
      return text;
    }

    text += '\n\n<div class="footnotes">\n<hr>\n<ol>\n\n';
    for (var i = 0; i < self.usedFootnotes.length; i++) {
      var id = self.usedFootnotes[i];
      var footnote = self.footnotes[id];
      self.isConvertingFootnote = true;
      var formattedfootnote = convertSpans(footnote, self);
      delete self.isConvertingFootnote;
      text += '<li id="fn_' + id + '">' + formattedfootnote + ' <a href="#fnref_' + id + '" title="Return to article" class="reversefootnote">&#8617;</a></li>\n\n';
    }
    text += '</ol>\n</div>';
    return text;
  };

  /******************************************************************
   * Fenced Code Blocks  (gfm)                                       *
   ******************************************************************/

  // Find and convert gfm-inspired fenced code blocks into html.
  Markdown.Extra.prototype.fencedCodeBlocks = function (text) {
    function encodeCode(code) {
      code = code.replace(/&/g, "&amp;");
      code = code.replace(/</g, "&lt;");
      code = code.replace(/>/g, "&gt;");
      // These were escaped by PageDown before postNormalization
      code = code.replace(/~D/g, "$$");
      code = code.replace(/~T/g, "~");
      return code;
    }

    var self = this;
    text = text.replace(/(?:^|\n)```[ \t]*(\S*)[ \t]*\n([\s\S]*?)\n```[ \t]*(?=\n)/g, function (match, m1, m2) {
      var language = m1,
          codeblock = m2;

      // adhere to specified options
      var preclass = self.googleCodePrettify ? ' class="prettyprint linenums"' : '';
      var codeclass = '';
      if (language) {
        if (self.googleCodePrettify || self.highlightJs) {
          // use html5 language- class names. supported by both prettify and highlight.js
          codeclass = ' class="language-' + language + '"';
        } else {
          codeclass = ' class="' + language + '"';
        }
      }

      var html = ['<pre', preclass, '><code', codeclass, '>', encodeCode(codeblock), '</code></pre>'].join('');

      // replace codeblock with placeholder until postConversion step
      return self.hashExtraBlock(html);
    });

    return text;
  };

  /******************************************************************
   * SmartyPants                                                     *
   ******************************************************************/

  Markdown.Extra.prototype.educatePants = function (text) {
    var self = this;
    var result = '';
    var blockOffset = 0;
    // Here we parse HTML in a very bad manner
    text.replace(/(?:<!--[\s\S]*?-->)|(<)([a-zA-Z1-6]+)([^\n]*?>)([\s\S]*?)(<\/\2>)/g, function (wholeMatch, m1, m2, m3, m4, m5, offset) {
      var token = text.substring(blockOffset, offset);
      result += self.applyPants(token);
      self.smartyPantsLastChar = result.substring(result.length - 1);
      blockOffset = offset + wholeMatch.length;
      if (!m1) {
        // Skip commentary
        result += wholeMatch;
        return;
      }
      // Skip special tags
      if (!/code|kbd|pre|script|noscript|iframe|math|ins|del|pre/i.test(m2)) {
        m4 = self.educatePants(m4);
      } else {
        self.smartyPantsLastChar = m4.substring(m4.length - 1);
      }
      result += m1 + m2 + m3 + m4 + m5;
    });
    var lastToken = text.substring(blockOffset);
    result += self.applyPants(lastToken);
    self.smartyPantsLastChar = result.substring(result.length - 1);
    return result;
  };

  function revertPants(wholeMatch, m1) {
    var blockText = m1;
    blockText = blockText.replace(/&\#8220;/g, "\"");
    blockText = blockText.replace(/&\#8221;/g, "\"");
    blockText = blockText.replace(/&\#8216;/g, "'");
    blockText = blockText.replace(/&\#8217;/g, "'");
    blockText = blockText.replace(/&\#8212;/g, "---");
    blockText = blockText.replace(/&\#8211;/g, "--");
    blockText = blockText.replace(/&\#8230;/g, "...");
    return blockText;
  }

  Markdown.Extra.prototype.applyPants = function (text) {
    // Dashes
    text = text.replace(/---/g, "&#8212;").replace(/--/g, "&#8211;");
    // Ellipses
    text = text.replace(/\.\.\./g, "&#8230;").replace(/\.\s\.\s\./g, "&#8230;");
    // Backticks
    text = text.replace(/``/g, "&#8220;").replace(/''/g, "&#8221;");

    if (/^'$/.test(text)) {
      // Special case: single-character ' token
      if (/\S/.test(this.smartyPantsLastChar)) {
        return "&#8217;";
      }
      return "&#8216;";
    }
    if (/^"$/.test(text)) {
      // Special case: single-character " token
      if (/\S/.test(this.smartyPantsLastChar)) {
        return "&#8221;";
      }
      return "&#8220;";
    }

    // Special case if the very first character is a quote
    // followed by punctuation at a non-word-break. Close the quotes by brute force:
    text = text.replace(/^'(?=[!"#\$\%'()*+,\-.\/:;<=>?\@\[\\]\^_`{|}~]\B)/, "&#8217;");
    text = text.replace(/^"(?=[!"#\$\%'()*+,\-.\/:;<=>?\@\[\\]\^_`{|}~]\B)/, "&#8221;");

    // Special case for double sets of quotes, e.g.:
    //   <p>He said, "'Quoted' words in a larger quote."</p>
    text = text.replace(/"'(?=\w)/g, "&#8220;&#8216;");
    text = text.replace(/'"(?=\w)/g, "&#8216;&#8220;");

    // Special case for decade abbreviations (the '80s):
    text = text.replace(/'(?=\d{2}s)/g, "&#8217;");

    // Get most opening single quotes:
    text = text.replace(/(\s|&nbsp;|--|&[mn]dash;|&\#8211;|&\#8212;|&\#x201[34];)'(?=\w)/g, "$1&#8216;");

    // Single closing quotes:
    text = text.replace(/([^\s\[\{\(\-])'/g, "$1&#8217;");
    text = text.replace(/'(?=\s|s\b)/g, "&#8217;");

    // Any remaining single quotes should be opening ones:
    text = text.replace(/'/g, "&#8216;");

    // Get most opening double quotes:
    text = text.replace(/(\s|&nbsp;|--|&[mn]dash;|&\#8211;|&\#8212;|&\#x201[34];)"(?=\w)/g, "$1&#8220;");

    // Double closing quotes:
    text = text.replace(/([^\s\[\{\(\-])"/g, "$1&#8221;");
    text = text.replace(/"(?=\s)/g, "&#8221;");

    // Any remaining quotes should be opening ones.
    text = text.replace(/"/ig, "&#8220;");
    return text;
  };

  // Find and convert markdown extra definition lists into html.
  Markdown.Extra.prototype.runSmartyPants = function (text) {
    this.smartyPantsLastChar = '';
    text = this.educatePants(text);
    // Clean everything inside html tags (some of them may have been converted due to our rough html parsing)
    text = text.replace(/(<([a-zA-Z1-6]+)\b([^\n>]*?)(\/)?>)/g, revertPants);
    return text;
  };

  /******************************************************************
   * Definition Lists                                                *
   ******************************************************************/

  // Find and convert markdown extra definition lists into html.
  Markdown.Extra.prototype.definitionLists = function (text) {
    var wholeList = new RegExp(['(\\x02\\n?|\\n\\n)', '(?:', '(', // $1 = whole list
    '(', // $2
    '[ ]{0,3}', '((?:[ \\t]*\\S.*\\n)+)', // $3 = defined term
    '\\n?', '[ ]{0,3}:[ ]+', // colon starting definition
    ')', '([\\s\\S]+?)', '(', // $4
    '(?=\\0x03)', // \z
    '|', '(?=', '\\n{2,}', '(?=\\S)', '(?!', // Negative lookahead for another term
    '[ ]{0,3}', '(?:\\S.*\\n)+?', // defined term
    '\\n?', '[ ]{0,3}:[ ]+', // colon starting definition
    ')', '(?!', // Negative lookahead for another definition
    '[ ]{0,3}:[ ]+', // colon starting definition
    ')', ')', ')', ')', ')'].join(''), 'gm');

    var self = this;
    text = addAnchors(text);

    text = text.replace(wholeList, function (match, pre, list) {
      var result = trim(self.processDefListItems(list));
      result = "<dl>\n" + result + "\n</dl>";
      return pre + self.hashExtraBlock(result) + "\n\n";
    });

    return removeAnchors(text);
  };

  // Process the contents of a single definition list, splitting it
  // into individual term and definition list items.
  Markdown.Extra.prototype.processDefListItems = function (listStr) {
    var self = this;

    var dt = new RegExp(['(\\x02\\n?|\\n\\n+)', // leading line
    '(', // definition terms = $1
    '[ ]{0,3}', // leading whitespace
    '(?![:][ ]|[ ])', // negative lookahead for a definition
    //   mark (colon) or more whitespace
    '(?:\\S.*\\n)+?', // actual term (not whitespace)
    ')', '(?=\\n?[ ]{0,3}:[ ])' // lookahead for following line feed
    ].join(''), //   with a definition mark
    'gm');

    var dd = new RegExp(['\\n(\\n+)?', // leading line = $1
    '(', // marker space = $2
    '[ ]{0,3}', // whitespace before colon
    '[:][ ]+', // definition mark (colon)
    ')', '([\\s\\S]+?)', // definition text = $3
    '(?=\\n*', // stop at next definition mark,
    '(?:', // next term or end of text
    '\\n[ ]{0,3}[:][ ]|', '<dt>|\\x03', // \z
    ')', ')'].join(''), 'gm');

    listStr = addAnchors(listStr);
    // trim trailing blank lines:
    listStr = listStr.replace(/\n{2,}(?=\\x03)/, "\n");

    // Process definition terms.
    listStr = listStr.replace(dt, function (match, pre, termsStr) {
      var terms = trim(termsStr).split("\n");
      var text = '';
      for (var i = 0; i < terms.length; i++) {
        var term = terms[i];
        // process spans inside dt
        term = convertSpans(trim(term), self);
        text += "\n<dt>" + term + "</dt>";
      }
      return text + "\n";
    });

    // Process actual definitions.
    listStr = listStr.replace(dd, function (match, leadingLine, markerSpace, def) {
      if (leadingLine || def.match(/\n{2,}/)) {
        // replace marker with the appropriate whitespace indentation
        def = Array(markerSpace.length + 1).join(' ') + def;
        // process markdown inside definition
        // TODO?: currently doesn't apply extensions
        def = outdent(def) + "\n\n";
        def = "\n" + convertAll(def, self) + "\n";
      } else {
        // convert span-level markdown inside definition
        def = rtrim(def);
        def = convertSpans(outdent(def), self);
      }

      return "\n<dd>" + def + "</dd>\n";
    });

    return removeAnchors(listStr);
  };

  /***********************************************************
   * Strikethrough                                            *
   ************************************************************/

  Markdown.Extra.prototype.strikethrough = function (text) {
    // Pretty much duplicated from _DoItalicsAndBold
    return text.replace(/([\W_]|^)~T~T(?=\S)([^\r]*?\S[\*_]*)~T~T([\W_]|$)/g, "$1<del>$2</del>$3");
  };

  /***********************************************************
   * New lines                                                *
   ************************************************************/

  Markdown.Extra.prototype.newlines = function (text) {
    // We have to ignore already converted newlines and line breaks in sub-list items
    return text.replace(/(<(?:br|\/li)>)?\n/g, function (wholeMatch, previousTag) {
      return previousTag ? wholeMatch : " <br>\n";
    });
  };
})();

exports['default'] = Markdown.Extra;
module.exports = exports['default'];

},{}],20:[function(require,module,exports){
/**
 * markdown & mathjax 渲染处理
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _commonXss = require('./../common/xss');

var _commonXss2 = _interopRequireDefault(_commonXss);

var _commonDependLoader = require('./../common/dependLoader');

var _commonDependLoader2 = _interopRequireDefault(_commonDependLoader);

var _commonScriptLoader = require('./../common/scriptLoader');

var _commonScriptLoader2 = _interopRequireDefault(_commonScriptLoader);

var _MarkdownConverter = require('./Markdown.Converter');

var _MarkdownConverter2 = _interopRequireDefault(_MarkdownConverter);

var _MarkdownExtra = require('./Markdown.Extra');

var _MarkdownExtra2 = _interopRequireDefault(_MarkdownExtra);

var isMathjax = false;
var WizToc = '#wizToc';

var defalutCB = {
    markdown: function markdown() {
        Render.Win.prettyPrint();
        Render.tocRender();
        Render.flowRender();
        Render.sequenceRender();
    },
    mathJax: function mathJax() {}
};

var MarkdownRender = {
    init: function init() {
        Render.Win = _commonEnv2['default'].win;
        Render.Document = _commonEnv2['default'].doc;
        Render.Dependency = _commonEnv2['default'].dependency;

        return MarkdownRender;
    },
    markdown: function markdown(callback) {
        if (callback) {
            Render.callback.markdown = Render.addCb(defalutCB.markdown, callback.markdown);
            Render.callback.mathJax = Render.addCb(defalutCB.mathJax, callback.mathJax);
        }

        _commonDependLoader2['default'].loadCss(Render.Document, Render.getDependencyFiles('css', 'markdown'));

        _commonDependLoader2['default'].loadJs(Render.Document, Render.getDependencyFiles('js', 'markdown'), function () {
            Render.markdownConvert({});
            if (isMathjax) {
                Render.mathJaxRender();
            }
        });
    },
    mathJax: function mathJax(callback) {
        if (callback) {
            Render.callback.mathJax = Render.addCb(defalutCB.mathJax, callback);
        }
        Render.mathJaxRender();
    }
};

var Render = {
    Utils: _commonUtils2['default'],
    Win: null,
    Document: null,
    Dependency: null,
    callback: {
        markdown: null,
        mathJax: null
    },
    getDependencyFiles: function getDependencyFiles(type, id) {
        var i, j, g, ii, jj, gg, group;
        var markdownFiles = [];
        for (i = 0, j = Render.Dependency[type][id].length; i < j; i++) {
            g = Render.Dependency[type][id][i];
            if (type == 'css') {
                markdownFiles.push(Render.Dependency.files[type][g]);
            } else {
                group = [];
                for (ii = 0, jj = g.length; ii < jj; ii++) {
                    gg = g[ii];
                    group.push(Render.Dependency.files[type][gg]);
                }
                markdownFiles.push(group);
            }
        }
        return markdownFiles;
    },
    addCb: function addCb(defaultCb, newCb) {
        if (newCb) {
            return function () {
                defaultCb.apply(this, arguments);
                newCb.apply(this, arguments);
            };
        } else {
            return defaultCb;
        }
    },
    cb: function cb(callback, params) {
        if (callback) {
            callback.apply(this, params ? params : []);
        }
    },
    getBodyTxt: function getBodyTxt(body) {
        var text = body.innerText;
        if (!text) {
            // FF自己解析innerText
            text = Render.Utils.getInnerText($body[0]);
        }
        // 替换unicode160的空格为unicode为32的空格，否则pagedown无法识别
        return text.replace(/\u00a0/g, " ");
    },
    markdownConvert: function markdownConvert(frame) {
        var start, end, last, blocks, math, braces;
        var SPLIT = /(\$\$?|\\(?:begin|end)\{[a-z]*\*?\}|\\[\\{}$]|[{}]|(?:\n\s*)+|@@\d+@@)/i;

        var $doc = $(Render.Document);
        var $body = frame.container ? frame.container : $doc.find('body');

        $body.addClass('markdown-body');
        var converter = new _MarkdownConverter2['default'].Converter({
            nonAsciiLetters: true,
            asteriskIntraWordEmphasis: true
        });
        _MarkdownExtra2['default'].init(converter, { extensions: "all", highlighter: "prettify" });

        var text;
        try {
            Render.Utils.markdownPreProcess($body[0]);
            text = Render.tocReady(Render.getBodyTxt($body[0]));

            // 判断是否含有mathjax语法
            var judgeMathjaxText = text.replace(/\n/g, '\\n').replace(/\r\n?/g, "\n").replace(/```(.*\n)+?```/gm, '');
            isMathjax = /(\$\$?)[^$\n]+\1/.test(judgeMathjaxText);

            if (isMathjax) {
                text = removeMath(text);
            }

            text = converter.makeHtml(text);
            if (isMathjax) {
                text = replaceMath(text);
            }
            text = Render.xssFilter(text);
            $body[0].innerHTML = text;
        } catch (e) {
            console.error(e);
        }
        try {
            Render.cb(Render.callback.markdown, [isMathjax]);
        } catch (e) {
            console.error(e);
        }

        function replaceMath(text) {
            text = text.replace(/@@(\d+)@@/g, function (match, n) {
                return math[n];
            });
            math = null;
            return text;
        }

        function processMath(i, j) {
            var block = blocks.slice(i, j + 1).join("").replace(/&/g, "&amp;") // use HTML entity for &
            .replace(/</g, "&lt;") // use HTML entity for <
            .replace(/>/g, "&gt;"); // use HTML entity for
            while (j > i) {
                blocks[j] = "";
                j--;
            }
            blocks[i] = "@@" + math.length + "@@";
            math.push(block);
            start = end = last = null;
        }

        function removeMath(text) {
            start = end = last = null; // for tracking math delimiters
            math = []; // stores math strings for latter

            blocks = text.replace(/\r\n?/g, "\n").split(SPLIT);
            for (var i = 1, m = blocks.length; i < m; i += 2) {
                var block = blocks[i];
                if (block.charAt(0) === "@") {

                    blocks[i] = "@@" + math.length + "@@";
                    math.push(block);
                } else if (start) {

                    if (block === end) {
                        if (braces) {
                            last = i;
                        } else {
                            processMath(start, i);
                        }
                    } else if (block.match(/\n.*\n/)) {
                        if (last) {
                            i = last;
                            processMath(start, i);
                        }
                        start = end = last = null;
                        braces = 0;
                    } else if (block === "{") {
                        braces++;
                    } else if (block === "}" && braces) {
                        braces--;
                    }
                } else {
                    //
                    // Look for math start delimiters and when
                    // found, set up the end delimiter.
                    //
                    if (block === "$$") {
                        start = i;
                        end = block;
                        braces = 0;
                    } else if (block.substr(1, 5) === "begin") {
                        start = i;
                        end = "\\end" + block.substr(6);
                        braces = 0;
                    }
                }
            }
            if (last) {
                processMath(start, last);
            }
            return blocks.join("");
        }
    },
    tocReady: function tocReady(markdownStr) {
        return markdownStr.replace(/(^[ ]*)\[toc\]([ ]*(\n|$))/igm, '$1[](' + WizToc + ')$2');
    },
    tocRender: function tocRender() {
        var tocHtml = [],
            min = 6;
        $('h1,h2,h3,h4,h5,h6', Render.Document.body).each(function (index, item) {
            var n = parseInt(item.tagName.charAt(1));
            if (n < min) {
                min = n;
            }
        });

        $('h1,h2,h3,h4,h5,h6', Render.Document.body).each(function (index, item) {
            var id = 'wiz_toc_' + index;
            var n = parseInt(item.tagName.charAt(1));
            var $item = $(item);
            $item.attr('id', id);
            tocHtml.push('<a class="wiz_toc ' + 'h' + (n - min + 1) + '" href="#' + id + '">' + $item.text() + '</a>');
        });
        tocHtml = '<div class="wiz_toc_layer">' + tocHtml.join('<br/>') + '</div>';

        $('a', Render.Document.body).each(function (index, item) {
            item = $(item);
            if (item.attr('href') == WizToc) {
                item.before(tocHtml);
            }
        });
    },
    flowRender: function flowRender() {
        var f = $('.language-flow', Render.Document.body).parents('pre');
        f.each(function (fIndex, fObj) {
            var id = 'wiz-flow-' + fIndex;
            var line = $('li', fObj);
            var flowStr = '';
            line.each(function (index, obj) {
                var s = $(obj).text();
                if (s.length > 0) {
                    flowStr += s + '\n';
                }
            });
            if (flowStr.length > 0) {
                try {
                    fObj.style.display = 'none';
                    var diagram = Render.Win.flowchart.parse(flowStr);
                    var flowLayer = Render.Document.createElement('div');
                    flowLayer.id = id;
                    fObj.parentNode.insertBefore(flowLayer, fObj);
                    diagram.drawSVG(id);

                    //修正 svg 保证手机端自动适应大小
                    if (_commonEnv2['default'].client.isPhone) {
                        //pc、mac 客户端 取消height 设置后， 会导致height 变为0，从而不显示
                        var s = $('svg', flowLayer);
                        if (s.attr('width')) {
                            s.css({
                                'max-width': s.attr('width')
                            }).attr({
                                'height': null,
                                'width': '95%'
                            });
                        }
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        });
    },
    sequenceRender: function sequenceRender() {
        var f = $('.language-sequence', Render.Document.body).parents('pre');
        f.each(function (fIndex, fObj) {
            var id = 'wiz-sequence-' + fIndex;
            var line = $('li', fObj);
            var seqStr = '';
            line.each(function (index, obj) {
                var s = $(obj).text();
                if (s.length > 0) {
                    seqStr += s + '\n';
                }
            });
            if (seqStr.length > 0) {
                try {
                    fObj.style.display = 'none';
                    var diagram = Render.Win.Diagram.parse(seqStr);
                    var seqLayer = Render.Document.createElement('div');
                    seqLayer.id = id;
                    fObj.parentNode.insertBefore(seqLayer, fObj);
                    diagram.drawSVG(id, { theme: 'simple' });

                    //修正 svg 保证手机端自动适应大小
                    if (_commonEnv2['default'].client.isPhone) {
                        //pc、mac 客户端 取消height 设置后， 会导致height 变为0，从而不显示
                        var s = $('svg', seqLayer);
                        if (s.attr('width')) {
                            s.get(0).setAttribute('viewBox', '0 0 ' + s.attr('width') + ' ' + s.attr('height'));
                            s.css({
                                'max-width': s.attr('width')
                            }).attr({
                                'preserveAspectRatio': 'xMidYMid meet',
                                'height': null,
                                'width': '95%'
                            });
                        }
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        });
    },
    mathJaxRender: function mathJaxRender() {
        var config = 'MathJax.Hub.Config({\
                            skipStartupTypeset: true,\
                            "HTML-CSS": {\
                                preferredFont: "TeX",\
                                availableFonts: [\
                                    "STIX",\
                                    "TeX"\
                                ],\
                                linebreaks: {\
                                    automatic: true\
                                },\
                                EqnChunk: 10,\
                                imageFont: null\
                            },\
                            tex2jax: {\
                                inlineMath: [["$","$"],["\\\\\\\\(","\\\\\\\\)"]],\
                                displayMath: [["$$","$$"],["\\\\[","\\\\]"]],\
                                processEscapes: true },\
                            TeX: {\
                                equationNumbers: {\
                                    autoNumber: "AMS"\
                                },\
                                noUndefined: {\
                                    attributes: {\
                                        mathcolor: "red",\
                                        mathbackground: "#FFEEEE",\
                                        mathsize: "90%"\
                                    }\
                                },\
                                Safe: {\
                                    allow: {\
                                        URLs: "safe",\
                                        classes: "safe",\
                                        cssIDs: "safe",\
                                        styles: "safe",\
                                        fontsize: "all"\
                                    }\
                                }\
                            },\
                            messageStyle: "none"\
                        });';

        _commonScriptLoader2['default'].appendJsCode(Render.Document, 'MathJax = null', 'text/javascript');
        _commonScriptLoader2['default'].appendJsCode(Render.Document, config, 'text/x-mathjax-config');
        _commonDependLoader2['default'].loadJs(Render.Document, Render.getDependencyFiles('js', 'mathJax'), _render);

        function _render() {
            Render.Win._wizMathJaxCallback = function () {
                Render.cb(Render.callback.mathJax);
            };
            var runMath = 'MathJax.Hub.Queue(' + '["Typeset", MathJax.Hub, document.body, _wizMathJaxCallback]);';
            _commonScriptLoader2['default'].appendJsCode(Render.Document, runMath, 'text/javascript');
        }
    },
    xssFilter: (function () {
        if (typeof _commonXss2['default'] == 'undefined') {
            return null;
        }
        var xss = new _commonXss2['default'].FilterXSS({
            onIgnoreTag: function onIgnoreTag(tag, html, options) {
                //针对白名单之外的 tag 处理
                if (/script/ig.test(tag)) {
                    return _commonXss2['default'].escapeAttrValue(html);
                }
                if (options.isClosing) {
                    return '</' + tag + '>';
                }

                var x = _commonXss2['default'].parseAttr(html, function (name, value) {
                    value = _commonXss2['default'].safeAttrValue(tag, name, value, xss);
                    if (/^on/i.test(name)) {
                        return '';
                    } else if (value) {
                        return name + '="' + value + '"';
                    } else {
                        return name;
                    }
                });

                if (/^<!/i.test(html)) {
                    //<!doctype html>
                    x = '<!' + x;
                } else {
                    x = '<' + x;
                }

                if (html[html.length - 2] === '/') {
                    x += '/';
                }
                x += '>';
                return x;
            },
            onIgnoreTagAttr: function onIgnoreTagAttr(tag, name, value, isWhiteAttr) {
                if (!!value && /^(id|class|style|data|width|height)/i.test(name)) {
                    return name + '="' + value + '"';
                }
                return '';
            },
            safeAttrValue: function safeAttrValue(tag, name, value) {
                // 自定义过滤属性值函数，如果为a标签的href属性，则先判断是否以wiz://开头
                if (tag === 'a' && name === 'href') {
                    if (/^((file|wiz(note)?):\/\/)/.test(value) || /^(#|index_files\/)/.test(value)) {
                        return _commonXss2['default'].escapeAttrValue(value);
                    }
                } else if (name === 'src') {
                    if (/^(file:\/\/)/.test(value) || /^(index_files\/|[\.]*\/)/.test(value)) {
                        return _commonXss2['default'].escapeAttrValue(value);
                    } else if (!/^(http[s]?|ftp|):\/\//.test(value)) {
                        return './' + _commonXss2['default'].escapeAttrValue(value);
                    }
                }
                // 其他情况，使用默认的safeAttrValue处理函数
                return _commonXss2['default'].safeAttrValue(tag, name, value);
            }
        });
        return function (html) {
            return xss.process(html);
        };
    })()
};
exports['default'] = MarkdownRender;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/dependLoader":5,"./../common/env":6,"./../common/scriptLoader":9,"./../common/utils":10,"./../common/xss":13,"./Markdown.Converter":18,"./Markdown.Extra":19}],21:[function(require,module,exports){
/**
 * 夜间模式的基本方法集合
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonWizStyle = require('../common/wizStyle');

var _commonWizStyle2 = _interopRequireDefault(_commonWizStyle);

var _color = '#7990b6',
    _bk_color = '#1f2126',
    _brightness = '50%',
    _style_id = 'wiz_night_mode_style';

var nightModeUtils = {
    on: function on(color, bgColor, brightness) {
        if (color) {
            _color = color;
        }
        if (bgColor) {
            _bk_color = bgColor;
        }
        if (brightness) {
            _brightness = brightness;
        }

        nightModeUtils.off();

        var map = {},
            arr = [];

        checkElement('', _commonEnv2['default'].doc.body, map);

        var baseStyle = '{' + 'color:' + _color + ' !important; ' + 'background-color:' + _bk_color + ' !important; ' + 'background-image: none !important; ' + 'box-shadow: none !important; ' + 'border-color:' + _color + ' !important; ' + '}';

        for (var key in map) {
            if (map.hasOwnProperty(key)) {
                arr.push(key);
            }
        }

        var cssText = arr.join(", ");
        cssText += baseStyle;
        //image brightness
        cssText += 'img{filter: brightness(' + _brightness + ');-webkit-filter: brightness(' + _brightness + ');}';

        _commonWizStyle2['default'].insertStyle({
            id: _style_id,
            name: _commonConst2['default'].NAME.TMP_STYLE
        }, cssText);
    },
    off: function off() {
        var style = _commonEnv2['default'].doc.getElementById(_style_id);
        if (style) {
            style.remove();
        }
    }
};

function checkElement(pId, e, map) {
    addItemAttrToMap(pId, e, map);
    var elements = e.children;
    for (var i = 0; i < elements.length; i++) {
        var child = elements[i];
        checkElement(e.id ? e.id : pId, child, map);
    }
}

function addItemAttrToMap(pId, e, map) {
    if (!e) return;
    var tagName = e.tagName;

    if (/^(style|script|link|meta|img)$/ig.test(tagName)) {
        return;
    }

    var className = e.className;
    if (className && className.length > 0) {
        var arr = className.split(" ");
        for (var i = 0; i < arr.length; i++) {
            var name = arr[i];
            if (name.length == 0) {
                continue;
            }
            //if (!!pId) {
            //    addKeyToMap('#' + pId + " ." + name, map);
            //} else {
            addKeyToMap("." + name, map);
            //}
        }
    }
    var id = e.id;
    if (id && id.length > 0) {
        addKeyToMap("#" + id, map);
    }
    //某些页面的控件给自己的特殊样式添加 !important，这些控件一般会在顶层 dom 设置 id ，所以都加上 id
    //为了减少 样式冗余，目前只给 tag 添加 id ， className 暂时不添加
    if (!!pId) {
        addKeyToMap('#' + pId + " " + tagName, map);
    } else {
        addKeyToMap(tagName, map);
    }
}

function addKeyToMap(key, map) {
    //只保留 非数字开头的 且 全部内容为 数字、英文字母、. - _ 的 key
    if (!map[key] && !/^(\.|#)?[\d]+/i.test(key) && /^(\.|#)?[\. \w-]+$/i.test(key)) {
        map[key] = "";
    }
}

exports['default'] = nightModeUtils;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/wizStyle":11}],22:[function(require,module,exports){
/**
 * 范围操作的基本方法集合
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomBase = require('./../domUtils/domBase');

var _domUtilsDomBase2 = _interopRequireDefault(_domUtilsDomBase);

//通用方法集合
var rangeUtils = {
    /**
     * 设置 光标到可视范围内（移动滚动条）
     */
    caretFocus: function caretFocus() {
        //getClientRects 方法 在 ios 的 safari 上 还有问题
        var range = rangeUtils.getRange(),
            rectList = range ? range.getClientRects() : null,
            rect = rectList && rectList.length > 0 ? rectList[0] : null,
            cH = _commonEnv2['default'].doc.documentElement.clientHeight,
            cW = _commonEnv2['default'].doc.documentElement.clientWidth;

        if (rect && rect.top < 0) {
            _commonEnv2['default'].doc.body.scrollTop += rect.top;
        } else if (rect && rect.top + rect.height > cH) {
            _commonEnv2['default'].doc.body.scrollTop += rect.top + rect.height - cH;
        }

        if (rect && rect.left < 0) {
            _commonEnv2['default'].doc.body.scrollLeft += rect.left;
        } else if (rect && rect.left + rect.width > cW) {
            _commonEnv2['default'].doc.body.scrollLeft += rect.left + rect.width - cW;
        }
    },
    getRange: function getRange() {
        var sel = _commonEnv2['default'].doc.getSelection();
        return sel.rangeCount > 0 ? sel.getRangeAt(0) : null;
    },
    /**
     * 获取当前光标所在位置的dom元素
     * isCollapse = true 时 获取光标后面的第一个dom，如果当前在 textNode 内， 则直接返回 textNode
     * isCollapse = false， isBackward = false 获取 光标区间第一个 dom
     * isCollapse = false， isBackward = true 获取 光标区间最后一个 dom
     * @param isBackward
     * @returns {*}
     */
    getRangeAnchor: function getRangeAnchor(isBackward) {
        var range = rangeUtils.getRange();
        if (!range) {
            return null;
        }
        var rangeContainer = isBackward ? range.startContainer : range.endContainer,
            rangeOffset = isBackward ? range.startOffset : range.endOffset;

        if (!range.collapsed && !isBackward) {
            if (rangeContainer.nodeType === 3 && rangeOffset > 0) {
                return rangeContainer;
            } else if (rangeContainer.nodeType === 3) {
                return _domUtilsDomBase2['default'].getPreviousNode(rangeContainer, false, null);
            }

            if (rangeOffset > 0) {
                return _domUtilsDomBase2['default'].getLastDeepChild(rangeContainer.childNodes[rangeOffset - 1]);
            } else {
                return _domUtilsDomBase2['default'].getPreviousNode(rangeContainer, false, null);
            }
        }

        if (rangeContainer.nodeType === 3 && rangeOffset < rangeContainer.nodeValue.length) {
            return rangeContainer;
        } else if (rangeContainer.nodeType === 3) {
            return _domUtilsDomBase2['default'].getNextNode(rangeContainer, false, null);
        }

        if (rangeContainer.childNodes.length === 0) {
            return rangeContainer;
        } else if (rangeOffset == rangeContainer.childNodes.length) {
            return _domUtilsDomBase2['default'].getNextNode(rangeContainer.childNodes[rangeOffset - 1], false, null);
        } else {
            return _domUtilsDomBase2['default'].getFirstDeepChild(rangeContainer.childNodes[rangeOffset]);
        }
    },
    /**
     * 根据 获取 光标选中范围内的 dom 集合
     * @param options {noSplit: Boolean}
     * @returns {*}
     */
    getRangeDomList: function getRangeDomList(options) {
        var range = rangeUtils.getRange();
        if (!range) {
            return null;
        }
        var startDom = range.startContainer,
            startOffset = range.startOffset,
            endDom = range.endContainer,
            endOffset = range.endOffset;
        return _domUtilsDomBase2['default'].getDomListA2B({
            startDom: startDom,
            startOffset: startOffset,
            endDom: endDom,
            endOffset: endOffset,
            noSplit: !!options.noSplit
        });
    },
    /**
     * 获取 光标范围内 Dom 共同的父节点
     * @returns {*}
     */
    getRangeParentRoot: function getRangeParentRoot() {
        var range = rangeUtils.getRange(),
            startDom,
            endDom;
        if (!range) {
            return null;
        }
        startDom = range.startContainer;
        endDom = range.endContainer;
        return _domUtilsDomBase2['default'].getParentRoot([startDom, endDom]);
    },
    /**
     * 检验 dom 是否为 selection 的 边缘
     * @param dom
     */
    isRangeEdge: function isRangeEdge(dom) {
        var result = {
            isStart: false,
            isEnd: false
        };

        var range = rangeUtils.getRange();
        if (!range) {
            return;
        }
        result.isCollapsed = range.collapsed;
        result.startDom = range.startContainer;
        result.startOffset = range.startOffset;
        result.endDom = range.endContainer;
        result.endOffset = range.endOffset;

        var tmpStartDom, tmpEndDom;
        if (result.startDom.nodeType == 1 && result.startOffset < result.startDom.childNodes.length) {
            tmpStartDom = _domUtilsDomBase2['default'].getFirstDeepChild(result.startDom.childNodes[result.startOffset]);
        } else if (result.startDom.nodeType == 1) {
            tmpStartDom = _domUtilsDomBase2['default'].getNextNode(result.startDom.childNodes[result.startOffset - 1], false, null);
        }
        if (result.endDom.nodeType == 1 && result.endOffset > 0) {
            tmpEndDom = _domUtilsDomBase2['default'].getLastDeepChild(result.endDom.childNodes[result.endOffset - 1]);
        } else if (result.endDom.nodeType == 1) {
            tmpEndDom = _domUtilsDomBase2['default'].getPreviousNode(result.endDom, false, null);
        }
        result.isStart = result.startDom == dom || result.startDom == tmpStartDom;

        result.isEnd = result.endDom == dom || result.endDom == tmpEndDom;

        return result;
    },
    /**
     * 选中指定的 dom 元素
     * @param el
     */
    selectElementContents: function selectElementContents(el) {
        var range = _commonEnv2['default'].doc.createRange();
        range.selectNodeContents(el);
        var sel = _commonEnv2['default'].doc.getSelection();
        sel.removeAllRanges();
        sel.addRange(range);
    },
    /**
     * 在光标位置选中单个字符，遇到 Fill-Char 特殊字符需要一直选取
     * @param isBackward
     */
    selectCharIncludeFillChar: function selectCharIncludeFillChar(isBackward) {
        var sel = _commonEnv2['default'].doc.getSelection(),
            range = sel.getRangeAt(0),
            direction = isBackward ? 'backward' : 'forward';

        var tmpCurDom, tmpOffset, tmpNextDom, s;
        if (range.startContainer.nodeType === 1) {
            tmpCurDom = rangeUtils.getRangeAnchor(false);
            //range.startContainer !== tmpCurDom 的时候， 往往不是在空行的最前面，而是在 前一个 dom 的最后面
            if (range.startContainer == tmpCurDom && _domUtilsDomBase2['default'].isTag(tmpCurDom, 'br') && _domUtilsDomBase2['default'].isEmptyDom(tmpCurDom.parentNode)) {
                if (tmpCurDom.parentNode.nextSibling) {
                    rangeUtils.setRange(tmpCurDom.parentNode, 0, tmpCurDom.parentNode.nextSibling, 0);
                } else {
                    sel.modify('move', 'forward', 'character');
                    sel.modify('extend', 'backward', 'character');
                    if (tmpCurDom.nextSibling) {
                        sel.modify('extend', 'backward', 'character');
                    }
                }
                return;
            } else if (_domUtilsDomBase2['default'].isTag(tmpCurDom, 'br')) {
                sel.modify('extend', direction, 'character');
            }
        }

        sel.modify('extend', direction, 'character');
        range = sel.getRangeAt(0);
        s = range.toString();
        tmpCurDom = rangeUtils.getRangeAnchor(isBackward);

        if (!tmpCurDom) {
            //当没有文字，且只剩下空标签 和 自闭合标签时，有时候会不存在 tmpCurDom
            return;
        }
        if (isBackward && tmpCurDom == range.startContainer) {
            tmpOffset = range.startOffset;
        } else if (!isBackward && tmpCurDom == range.endContainer) {
            tmpOffset = range.endOffset;
        } else {
            //只要 tmpCurDom 不是range 的原始 dom ，就直接设置 tmpOffset 为 -1
            tmpOffset = -1;
        }

        //如果光标在某个 textNode 中间， 则前后都是当前这个 textNode
        if (tmpCurDom.nodeType === 3 && tmpOffset > 0 && tmpOffset < tmpCurDom.nodeValue.length) {
            tmpNextDom = tmpCurDom;
        } else {
            tmpNextDom = isBackward ? _domUtilsDomBase2['default'].getPreviousNode(tmpCurDom, false, null) : _domUtilsDomBase2['default'].getNextNode(tmpCurDom, false, null);
        }

        if (s.length === 0) {
            //如果当前未选中 自闭合标签（br）且下一个字符是 自闭合标签 则 扩展选中区域
            if (tmpCurDom && !_domUtilsDomBase2['default'].isSelfClosingTag(tmpCurDom) && tmpNextDom && (tmpNextDom.nodeType !== 1 || tmpNextDom.nodeType === 1 && _domUtilsDomBase2['default'].isSelfClosingTag(tmpNextDom))) {
                sel.modify('extend', direction, 'character');
            }
        } else if (s.indexOf(_commonConst2['default'].FILL_CHAR) > -1 && s.replace(_commonConst2['default'].FILL_CHAR_REG, '') === '') {
            //如果当前选中了 文本 但文本未占位字符，则扩展选中区域
            sel.modify('extend', direction, 'character');
        }
    },
    /**
     * 根据 起始 Dom 位置设定 光标选择范围
     * @param start
     * @param startOffset
     * @param end
     * @param endOffset
     */
    setRange: function setRange(start, startOffset, end, endOffset) {
        if (!start && !end) {
            return;
        }
        var maxStart = _domUtilsDomBase2['default'].getDomEndOffset(start),
            maxEnd = _domUtilsDomBase2['default'].getDomEndOffset(end);
        if (startOffset < 0) {
            startOffset = 0;
        } else if (startOffset > maxStart) {
            startOffset = maxStart;
        }
        if (endOffset < 0) {
            endOffset = _domUtilsDomBase2['default'].getDomEndOffset(end);
        } else if (endOffset > maxEnd) {
            endOffset = maxEnd;
        }
        var sel = _commonEnv2['default'].doc.getSelection();
        if (!start) {
            start = _commonEnv2['default'].doc.body;
            startOffset = 0;
        }
        sel.collapse(start, startOffset);
        if (end) {
            sel.extend(end, endOffset);
        }
    }
};

exports['default'] = rangeUtils;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10,"./../domUtils/domBase":14}],23:[function(require,module,exports){
/**
 * 范围操作的基本方法集合
 */

'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('./../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('./../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeBase = require('./rangeBase');

var _rangeBase2 = _interopRequireDefault(_rangeBase);

var rangeBackup;
_rangeBase2['default'].backupCaret = function () {
    var range = _rangeBase2['default'].getRange();
    if (!range) {
        if (rangeBackup) {
            return true;
        }

        _domUtilsDomExtend2['default'].focus();
        range = _rangeBase2['default'].getRange();
        if (!range) {
            return false;
        }
    }
    rangeBackup = _rangeBase2['default'].getRange();
    return true;
    //rangeBackup.setEnd(rangeBackup.startContainer, rangeBackup.startOffset);
};

_rangeBase2['default'].restoreCaret = function () {
    if (!rangeBackup) {
        return false;
    }
    var sel = _commonEnv2['default'].doc.getSelection();
    if (sel.rangeCount == 0) {
        _domUtilsDomExtend2['default'].focus();
    }
    sel.removeAllRanges();
    sel.addRange(rangeBackup);
    rangeBackup = null;

    return true;
};

/**
 * 在 光标（isCollapse=true）所在位置创建 指定样式的 span
 * make new span when selection's isCollapsed == true
 * @param style
 * @param attr
 */
_rangeBase2['default'].modifyCaretStyle = function (style, attr) {
    var sel = _commonEnv2['default'].doc.getSelection();
    var focusNode = sel.focusNode;
    var range,
        key,
        value,
        hasSameStyle = true,
        n;

    //get the focus's element.
    if (focusNode.nodeType == 3) {
        focusNode = focusNode.parentNode;
    }
    //check if the current dom is same as the style which is needed.
    for (key in style) {
        if (style.hasOwnProperty(key) && typeof key == 'string') {
            value = style[key];
            if (focusNode.style[key] !== value) {
                hasSameStyle = false;
            }
        }
    }
    if (hasSameStyle) {
        return;
    }

    //if current dom is empty, so don't create span.
    if (_domUtilsDomExtend2['default'].isTag(focusNode, 'span') && _commonUtils2['default'].isEmpty(focusNode.innerHTML)) {
        _domUtilsDomExtend2['default'].modifyStyle(focusNode, style, attr);
        n = focusNode;
    } else {
        range = sel.getRangeAt(0);
        range.deleteContents();
        n = _domUtilsDomExtend2['default'].createSpan();
        n.innerHTML = _commonConst2['default'].FILL_CHAR;
        range.insertNode(n);
        _domUtilsDomExtend2['default'].modifyStyle(n, style, attr);
    }

    //put the cursor's position to the target dom
    //range = ENV.doc.createRange();
    //range.setStart(n.childNodes[0], 1);
    //range.setEnd(n.childNodes[0], 1);

    //clear redundant span & TextNode
    //var p = focusNode;
    var p = focusNode.parentNode ? focusNode.parentNode : focusNode;
    _domUtilsDomExtend2['default'].clearChild(p, [n]);

    //reset the selection's range
    _rangeBase2['default'].setRange(n.childNodes[0], 1, n.childNodes[0], 1);
    //sel.removeAllRanges();
    //sel.addRange(range);
};
_rangeBase2['default'].modifyDomsStyle = function (domList, style, attr, excludeList) {
    //modify style
    _domUtilsDomExtend2['default'].modifyNodesStyle(domList, style, attr);
    //clear redundant span & TextNode
    var ps = [],
        i,
        j,
        t,
        tempAmend;
    for (i = 0, j = domList.length; i < j; i++) {
        t = domList[i].parentNode;
        if (!t) {
            continue;
        }
        if (ps.indexOf(t) < 0) {
            ps.push(t);
        }
    }
    //获取需要重构的 dom 集合共同的 parent 节点
    t = _domUtilsDomExtend2['default'].getParentRoot(ps);
    //如果是 修订节点，则找到修订节点的 父节点进行清理操作
    tempAmend = _domUtilsDomExtend2['default'].getWizAmendParent(t);
    t = tempAmend ? tempAmend.parentNode : t;
    _domUtilsDomExtend2['default'].clearChild(t, excludeList);
};
/**
 * 在 光标（isCollapse=false）选择范围内修改所有 dom内容，设置为指定样式
 * modify the style when selection's isCollapsed == false
 * @param style
 * @param attr
 */
_rangeBase2['default'].modifyRangeStyle = function (style, attr) {
    var rangeResult, rangeList, rangeLength;
    //get the RangeList
    rangeResult = _rangeBase2['default'].getRangeDomList({
        noSplit: false
    });
    if (!rangeResult) {
        return;
    }
    rangeList = rangeResult.list;
    rangeLength = rangeList.length;
    if (rangeLength === 0) {
        return;
    }

    //modify style
    _rangeBase2['default'].modifyDomsStyle(rangeList, style, attr, [rangeResult.startDomBak, rangeResult.endDomBak]);

    //reset the selection's range
    //自闭合标签 需要特殊处理
    var isStartBak = !rangeResult.startDom.parentNode,
        isEndBak = !rangeResult.endDom.parentNode,
        isSelfCloseEnd = _domUtilsDomExtend2['default'].isSelfClosingTag(rangeResult.endDom);
    //修正 Bak 的Dom
    if (isStartBak && _domUtilsDomExtend2['default'].isSelfClosingTag(rangeResult.startDomBak)) {
        rangeResult.startDomBak = _domUtilsDomExtend2['default'].getNextNode(rangeResult.startDomBak, false, rangeResult.endDomBak);
        rangeResult.startOffsetBak = 0;
    }
    _rangeBase2['default'].setRange(isStartBak ? rangeResult.startDomBak : rangeResult.startDom, isStartBak ? rangeResult.startOffsetBak : rangeResult.startOffset, isEndBak || isSelfCloseEnd ? rangeResult.endDomBak : rangeResult.endDom, isEndBak || isSelfCloseEnd ? rangeResult.endOffsetBak : rangeResult.endOffset);
};
/**
 * 修改 光标范围内的 Dom 样式 & 属性
 * @param style
 * @param attr
 */
_rangeBase2['default'].modifySelectionDom = function (style, attr) {
    var range = _rangeBase2['default'].getRange();
    if (!range) {
        return;
    }
    if (range.collapsed) {
        _rangeBase2['default'].modifyCaretStyle(style, attr);
    } else {
        _rangeBase2['default'].modifyRangeStyle(style, attr);
    }
};

exports['default'] = _rangeBase2['default'];
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":10,"./../domUtils/domExtend":15,"./rangeBase":22}],24:[function(require,module,exports){
/**
 * 阅读器 基础工具包
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _tableUtilsTableCore = require('../tableUtils/tableCore');

var _tableUtilsTableCore2 = _interopRequireDefault(_tableUtilsTableCore);

var _commonWizStyle = require('../common/wizStyle');

var _commonWizStyle2 = _interopRequireDefault(_commonWizStyle);

var _readerEvent = require('./readerEvent');

var _readerEvent2 = _interopRequireDefault(_readerEvent);

var reader = {
    init: function init(options) {
        _commonWizStyle2['default'].insertTmpReaderStyle();
        if (!options.ignoreTable) {
            _tableUtilsTableCore2['default'].setOptions({
                readonly: true
            });
        }
        _readerEvent2['default'].init();

        //禁用 输入框（主要用于 九宫格 处理）
        setDomReadOnly('input', true);
        setDomReadOnly('textarea', true);
    },
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        _commonWizStyle2['default'].insertDefaultStyle(isReplace, customCss);
    },
    on: function on(options) {
        if (!options.ignoreTable) {
            _tableUtilsTableCore2['default'].on();
        }
    },
    off: function off() {
        _tableUtilsTableCore2['default'].off();
    }
};

function setDomReadOnly(tag, readonly) {
    var domList = _commonEnv2['default'].doc.getElementsByTagName(tag),
        i,
        obj;
    for (i = 0; i < domList.length; i++) {
        obj = domList[i];
        obj.readOnly = readonly;
    }
}

exports['default'] = reader;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/utils":10,"../common/wizStyle":11,"../domUtils/domExtend":15,"../tableUtils/tableCore":26,"./readerEvent":25}],25:[function(require,module,exports){
/**
 * editor 使用的基本事件处理
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _imgUtilsImgUtils = require('../imgUtils/imgUtils');

var _imgUtilsImgUtils2 = _interopRequireDefault(_imgUtilsImgUtils);

var ReaderEvent = {
    init: function init() {
        if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isAndroid) {
            ReaderEvent.bindTouch();
        }
    },
    bindTouch: function bindTouch() {
        _commonEnv2['default'].doc.addEventListener('click', handler.onTouch);
    },
    unbind: function unbind() {
        _commonEnv2['default'].doc.removeEventListener('click', handler.onTouch);
    }
};

var handler = {
    onTouch: function onTouch(e) {
        var target = e.target;
        if (!target || !_domUtilsDomExtend2['default'].isTag(target, 'img') || target.className.indexOf('wiz-todo') > -1) {
            return;
        }

        //对于 超链接内的 img 不阻止点击事件，因为响应 超链接 更重要
        var p = _domUtilsDomExtend2['default'].getParentByFilter(target, function (node) {
            return node && _domUtilsDomExtend2['default'].isTag(node, 'a') && /^(http|https|wiz|wiznote|wiznotecmd):/.test(node.getAttribute('href'));
        }, true);

        if (p) {
            return;
        }

        _commonEnv2['default'].client.sendCmdToWiznote(_commonConst2['default'].CLIENT_EVENT.wizReaderClickImg, {
            src: target.src,
            imgList: _commonEnv2['default'].client.type.isAndroid ? _imgUtilsImgUtils2['default'].getAll(true).join(',') : null
        });
        _commonUtils2['default'].stopEvent(e);
        return false;
    }
};

exports['default'] = ReaderEvent;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/utils":10,"../domUtils/domExtend":15,"../imgUtils/imgUtils":17}],26:[function(require,module,exports){
/**
 * 表格操作核心包 core
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _tableUtils = require('./tableUtils');

var _tableUtils2 = _interopRequireDefault(_tableUtils);

var _tableMenu = require('./tableMenu');

var _tableMenu2 = _interopRequireDefault(_tableMenu);

var _tableZone = require('./tableZone');

var _tableZone2 = _interopRequireDefault(_tableZone);

var _domUtilsDomExtend = require('../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

var _commonHistoryUtils = require('../common/historyUtils');

var _commonHistoryUtils2 = _interopRequireDefault(_commonHistoryUtils);

//TODO 所有配色 要考虑到 夜间模式
var readonly = false;

var _event = {
    bind: function bind() {
        _event.unbind();
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_DRAG_START, _event.handler.onDragStart);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_KEY_UP, _tableUtils2['default'].fixSelection);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_OVER, _event.handler.onMouseOver);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_UP, _tableUtils2['default'].fixSelection);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
    },
    unbind: function unbind() {
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_DRAG_START, _event.handler.onDragStart);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_KEY_UP, _tableUtils2['default'].fixSelection);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_OVER, _event.handler.onMouseOver);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_UP, _tableUtils2['default'].fixSelection);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
    },
    handler: {
        afterRestoreHistory: function afterRestoreHistory() {
            //恢复历史后，需要修正 zone
            var tmpCells,
                cells = [],
                cell,
                i,
                j;

            tmpCells = _commonEnv2['default'].doc.getElementsByClassName(_commonConst2['default'].CLASS.SELECTED_CELL);

            if (tmpCells.length === 0) {
                _tableZone2['default'].clear();
                return;
            }

            for (i = 0, j = tmpCells.length; i < j; i++) {
                cells.push(tmpCells[i]);
            }

            var table = _domUtilsDomExtend2['default'].getParentByTagName(cells[0], 'table', true, null);
            if (!table) {
                _tableZone2['default'].clear();
                return;
            }

            _tableZone2['default'].setStart(cells[0]);

            var zone = _tableZone2['default'].getZone();
            var endCell = cells[cells.length - 1],
                endCellRange = _tableUtils2['default'].getRangeByCellData(_tableUtils2['default'].getCellData(zone.grid, endCell)),
                cellRange;

            for (i = 1; i < cells.length - 1; i++) {
                cell = cells[i];
                if (cell.rowSpan == 1) {
                    continue;
                }
                cellRange = _tableUtils2['default'].getRangeByCellData(_tableUtils2['default'].getCellData(zone.grid, cell));
                if (cellRange.maxY > endCellRange.maxY || (cellRange.maxY = endCellRange.maxY && cellRange.maxX > endCellRange.maxX)) {
                    endCell = cell;
                    endCellRange = cellRange;
                }
            }

            _tableZone2['default'].setEnd(endCell);

            //修正 Menu
            _tableMenu2['default'].show();
        },
        onDragStart: function onDragStart(e) {
            //表格内禁止拖拽操作
            var table = _domUtilsDomExtend2['default'].getParentByTagName(e.target, 'table', true, null);
            if (table) {
                _commonUtils2['default'].stopEvent(e);
            }
        },
        onKeyDown: function onKeyDown(e) {
            var zone = _tableZone2['default'].getZone();
            if (!zone.range) {
                return;
            }
            var code = e.keyCode || e.which,
                direct;
            switch (code) {
                case 37:
                    //left
                    direct = { x: -1, y: 0 };
                    break;
                case 38:
                    //up
                    direct = { x: 0, y: -1 };
                    break;
                case 9:
                    //Tab
                    if (!e.shiftKey) {
                        direct = { x: 1, y: 0, canChangeRow: true };
                    }
                    break;
                case 39:
                    //right
                    direct = { x: 1, y: 0 };
                    break;
                case 40:
                    //down
                    direct = { x: 0, y: 1 };
                    break;
            }

            var last;
            if (e.shiftKey) {
                last = zone.end || zone.start;
            } else {
                last = zone.start;
            }

            var cellData = _tableZone2['default'].switchCell(last, direct);
            if (cellData) {
                if (e.shiftKey) {
                    _tableZone2['default'].setEnd(cellData.cell, true);
                } else {
                    _tableZone2['default'].setStart(cellData.cell, cellData.x, cellData.y).setEnd(cellData.cell);
                }
                _commonUtils2['default'].stopEvent(e);
            }
        },
        onMouseDown: function onMouseDown(e) {
            var isLeft = e.button === 0 || e.button === 1;
            if (!isLeft) {
                _tableMenu2['default'].hide();
                return;
            }

            var isMenu = _tableMenu2['default'].isMenu(e.target);
            if (isMenu) {
                return;
            }

            var cell = _domUtilsDomExtend2['default'].getParentByTagName(e.target, ['th', 'td'], true, null);
            var table = cell ? _domUtilsDomExtend2['default'].getParentByTagName(cell, 'table', false, null) : null;
            var pos = _tableUtils2['default'].getEventPosition(e, table);
            var isZoneBorder = _tableZone2['default'].isZoneBorder(e);

            if (isZoneBorder.isRight) {
                _tableZone2['default'].startDragColLine(e.target, pos.x);
                return;
            }
            if (isZoneBorder.isBottom) {
                _tableZone2['default'].startDragRowLine(e.target, pos.y);
                return;
            }
            if (isZoneBorder.isDot) {
                console.log('isDot');
                return;
            }

            if (isZoneBorder.isBorder || isZoneBorder.isScroll) {
                return;
            }

            _tableZone2['default'].setStart(cell);
            _tableMenu2['default'].show();
        },
        onMouseOver: function onMouseOver(e) {
            var end = _domUtilsDomExtend2['default'].getParentByTagName(e.target, ['td', 'th'], true, null);
            _tableZone2['default'].modify(end);
        },
        onMouseUp: function onMouseUp(e) {
            var isLeft = e.button === 0 || e.button === 1;
            if (!isLeft) {
                return;
            }
            var isMenu, isZoneBorder;
            var zone = _tableZone2['default'].getZone();
            //当前正在选择单元格时， 不考虑 up 的位置是否 menu 等
            if (!zone.active) {
                isMenu = _tableMenu2['default'].isMenu(e.target);
                if (isMenu) {
                    return;
                }

                isZoneBorder = _tableZone2['default'].isZoneBorder(e);
                if (isZoneBorder.isRight && !_tableZone2['default'].isRangeActiving()) {
                    return;
                }
                if (isZoneBorder.isBottom && !_tableZone2['default'].isRangeActiving()) {
                    return;
                }
                if (isZoneBorder.isDot) {
                    console.log('isDot');
                    return;
                }
                if (isZoneBorder.isBorder || isZoneBorder.isScroll) {
                    return;
                }
            }
            var cell = _domUtilsDomExtend2['default'].getParentByTagName(e.target, ['td', 'th'], true, null);
            _tableZone2['default'].setEnd(cell);
            _tableMenu2['default'].show();
        }
    }
};

var tableCore = {
    on: function on() {
        if (!readonly) {
            _event.bind();
            _tableMenu2['default'].init(tableCore);
        }
        _tableUtils2['default'].checkTableContainer(null, readonly);
        _tableZone2['default'].clear();
    },
    off: function off() {
        _tableZone2['default'].clear();
    },
    setOptions: function setOptions(options) {
        readonly = !!options.readonly;
    },
    canCreateTable: function canCreateTable() {
        return _tableUtils2['default'].canCreateTable(_tableZone2['default'].getZone());
    },
    clearCellValue: function clearCellValue() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].clearCellValue(zone.grid, zone.range);
    },
    deleteCols: function deleteCols() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        if (zone.range.minX === 0 && zone.range.maxX === zone.grid[0].length - 1) {
            tableCore.deleteTable();
            return;
        }

        _commonHistoryUtils2['default'].saveSnap(false);
        var i;
        for (i = zone.range.maxX; i >= zone.range.minX; i--) {
            _tableUtils2['default'].deleteCols(zone.grid, i);
        }
        _tableZone2['default'].clear();
    },
    deleteRows: function deleteRows() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        if (zone.range.minY === 0 && zone.range.maxY === zone.grid.length - 1) {
            tableCore.deleteTable();
            return;
        }

        _commonHistoryUtils2['default'].saveSnap(false);
        var i;
        for (i = zone.range.maxY; i >= zone.range.minY; i--) {
            _tableUtils2['default'].deleteRows(zone.grid, i);
        }
        _tableZone2['default'].clear();
    },
    deleteTable: function deleteTable() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.table) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);

        var parent = zone.table.parentNode;
        if (parent) {
            parent.removeChild(zone.table);
        }
        _tableMenu2['default'].remove();
        _tableZone2['default'].remove();
        parent = _domUtilsDomExtend2['default'].getParentByFilter(parent, function (dom) {
            return _domUtilsDomExtend2['default'].hasClass(dom, _commonConst2['default'].CLASS.TABLE_CONTAINER);
        }, true);

        var enter;
        if (parent) {
            enter = _commonEnv2['default'].doc.createElement('br');
            parent.parentNode.insertBefore(enter, parent);
            parent.parentNode.removeChild(parent);
            _rangeUtilsRangeExtend2['default'].setRange(enter, 0);
        }
    },
    distributeCols: function distributeCols() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].distributeCols(zone.table, zone.grid);
        _tableZone2['default'].updateGrid();
    },
    insertCol: function insertCol(before) {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].insertCol(zone.grid, before ? zone.range.minX : zone.range.maxX + 1);
        _tableZone2['default'].updateGrid();
    },
    insertRow: function insertRow(before) {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].insertRow(zone.grid, before ? zone.range.minY : zone.range.maxY + 1);
        _tableZone2['default'].updateGrid();
    },
    insertTable: function insertTable(col, row) {
        _commonHistoryUtils2['default'].saveSnap(false);
        var range = _rangeUtilsRangeExtend2['default'].getRange();
        var tmpCell;

        if (!tableCore.canCreateTable()) {
            return;
        }
        if (range) {
            range.deleteContents();
            range = _rangeUtilsRangeExtend2['default'].getRange();
        }
        var table = _tableUtils2['default'].createTable(col, row);
        // var fillChar = domUtils.createSpan();
        // fillChar.innerHTML = CONST.FILL_CHAR + '234';
        var br = _commonEnv2['default'].doc.createElement('div');
        br.appendChild(_commonEnv2['default'].doc.createElement('br'));

        if (range) {
            // if (ENV.doc.queryCommandSupported('insertHTML')) {
            //     ENV.doc.execCommand('insertHTML', false, fillChar.outerHTML + table.outerHTML + br.outerHTML);
            // } else {
            _commonEnv2['default'].doc.execCommand('insertparagraph');
            range = _rangeUtilsRangeExtend2['default'].getRange();
            range.insertNode(table);
            range.insertNode(br);
            // }
        } else {
                _commonEnv2['default'].doc.body.appendChild(table);
                _commonEnv2['default'].doc.body.appendChild(br);
            }
        _tableUtils2['default'].checkTableContainer(null, readonly);

        //修正 光标
        range = _rangeUtilsRangeExtend2['default'].getRange();
        tmpCell = _domUtilsDomExtend2['default'].getParentByTagName(range.startContainer, ['tbody'], true, null);
        if (tmpCell) {
            _rangeUtilsRangeExtend2['default'].setRange(_domUtilsDomExtend2['default'].getFirstDeepChild(tmpCell), 0);
        }
    },
    merge: function merge() {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        var cell = _tableUtils2['default'].mergeCell(zone.grid, zone.range);
        if (cell) {
            _tableZone2['default'].updateGrid();
            _tableZone2['default'].setStart(cell).setEnd(cell);
        }
    },
    setCellAlign: function setCellAlign(align, valign) {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].setCellAlign(zone.grid, zone.range, {
            align: align,
            valign: valign
        });
        _tableZone2['default'].setStartRange();
    },
    setCellBg: function setCellBg(bgColor) {
        var zone = _tableZone2['default'].getZone();
        if (!zone.range) {
            return;
        }
        _commonHistoryUtils2['default'].saveSnap(false);
        _tableUtils2['default'].setCellBg(zone.grid, zone.range, bgColor);
        _tableZone2['default'].setStartRange();
    },
    split: function split() {
        var zone = _tableZone2['default'].getZone();
        var range = _tableUtils2['default'].splitCell(zone.table, zone.grid, zone.range);
        if (range) {
            _commonHistoryUtils2['default'].saveSnap(false);
            _tableZone2['default'].updateGrid();
            zone = _tableZone2['default'].getZone();
            _tableZone2['default'].setStart(zone.grid[range.minY][range.minX].cell).setEnd(zone.grid[range.maxY][range.maxX].cell);
        }
    }
};

exports['default'] = tableCore;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/historyUtils":7,"../common/utils":10,"../domUtils/domExtend":15,"../rangeUtils/rangeExtend":23,"./tableMenu":27,"./tableUtils":28,"./tableZone":29}],27:[function(require,module,exports){
/*
 表格菜单 控制
 */
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

// import utils from '../common/utils';

var _tableUtils = require('./tableUtils');

var _tableUtils2 = _interopRequireDefault(_tableUtils);

var _tableZone = require('./tableZone');

var _tableZone2 = _interopRequireDefault(_tableZone);

var _domUtilsDomExtend = require('../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

//import wizStyle from '../common/wizStyle';

var colorPadDemo;
var _id = {
    col: 'wiz-menu-col',
    align: 'wiz-menu-align',
    bg: 'wiz-menu-bg',
    bgDemo: 'wiz-menu-bg-demo',
    cells: 'wiz-menu-cells',
    more: 'wiz-menu-more'
};
var _class = {
    active: 'active',
    disabled: 'disabled',
    clickItem: 'click-item',
    colorPadItem: 'wiz-table-color-pad-item',
    alignItem: 'wiz-table-cell-align-item'
};
var _subType = {
    list: 1,
    custom: 2
};

var tableCore;
var menuObj;

function createMenu() {
    var menu = _commonEnv2['default'].doc.querySelector('.' + _commonConst2['default'].CLASS.TABLE_TOOLS);
    if (menu) {
        return menu;
    }

    var menuData = [{
        id: _id.col,
        exClass: 'icon-insert editor-icon',
        subMenu: {
            type: _subType.list,
            data: [{
                type: _commonConst2['default'].TYPE.TABLE.INSERT_ROW_UP,
                name: _commonLang2['default'].Table.InsertRowUp,
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.INSERT_ROW_DOWN,
                name: _commonLang2['default'].Table.InsertRowDown,
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.INSERT_COL_LEFT,
                name: _commonLang2['default'].Table.InsertColLeft,
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.INSERT_COL_RIGHT,
                name: _commonLang2['default'].Table.InsertColRight,
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.DELETE_ROW,
                name: _commonLang2['default'].Table.DeleteRow,
                isSplit: true
            }, {
                type: _commonConst2['default'].TYPE.TABLE.DELETE_COL,
                name: _commonLang2['default'].Table.DeleteCol,
                isSplit: false
            }]
        }
    }, {
        id: _id.align,
        exClass: 'icon-align editor-icon',
        subMenu: {
            type: _subType.custom,
            make: function make() {
                var typeList = [['top', 'middle', 'bottom'], ['left', 'center', 'right']];
                var i, j, dataAlignType;
                var str = '<div class="wiz-table-menu-sub wiz-table-cell-align">';
                for (i = 0; i < typeList.length; i++) {
                    str += '<div>';
                    for (j = 0; j < typeList[i].length; j++) {
                        dataAlignType = i === 0 ? 'valign' : 'align';
                        str += '<div class="' + _class.alignItem + ' ' + _class.clickItem + '" data-type="' + _commonConst2['default'].TYPE.TABLE.SET_CELL_ALIGN + '" data-align-type="' + dataAlignType + '" data-align-value="' + typeList[i][j] + '">';
                        if (i === 0) {
                            str += '<i class="editor-icon icon-box"></i>';
                            str += '<i class="editor-icon valign icon-valign_' + typeList[i][j] + '"></i>';
                        } else {
                            str += '<i class="editor-icon align icon-align_' + typeList[i][j] + '"></i>';
                        }

                        str += '</div>';
                    }
                    str += '</div>';
                }
                str += '</div>';

                return str;
            }
        }
    }, {
        id: _id.bg,
        exClass: 'icon-box editor-icon',
        subMenu: {
            type: _subType.custom,
            make: function make() {
                var colors = [['', '#f7b6ff', '#fecf9c'], ['#acf3fe', '#b2ffa1', '#b6caff'], ['#ffc7c8', '#eeeeee', '#fef49c']];
                var i, j;
                var str = '<div class="wiz-table-menu-sub wiz-table-color-pad">';
                for (i = 0; i < colors.length; i++) {
                    str += '<div>';
                    for (j = 0; j < colors[i].length; j++) {
                        str += '<div class="' + _class.colorPadItem + ' ' + _class.clickItem + '" data-color="' + colors[i][j] + '" data-type="' + _commonConst2['default'].TYPE.TABLE.SET_CELL_BG + '">';
                        str += '<i class="editor-icon icon-box"></i>';
                        if (i === 0 && j === 0) {
                            str += '<i class="pad-demo editor-icon icon-oblique_line"></i>';
                        } else {
                            str += '<i class="pad-demo editor-icon icon-inner_box" style="color:' + colors[i][j] + ';"></i>';
                        }
                        str += '</div>';
                    }
                    str += '</div>';
                }
                str += '</div>';
                return str;
            }
        }
    }, {
        id: _id.cells,
        exClass: 'icon-merge editor-icon',
        subMenu: {
            type: _subType.list,
            data: [{
                type: _commonConst2['default'].TYPE.TABLE.MERGE_CELL,
                name: _commonLang2['default'].Table.MergeCell,
                // exClass: tableZone.isSingleCell() ? 'disabled' : '',
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.SPLIT_CELL,
                name: _commonLang2['default'].Table.SplitCell,
                // exClass: tableZone.hasMergeCell() ? '' : 'disabled',
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.CLEAR_CELL,
                name: _commonLang2['default'].Table.ClearCell,
                isSplit: false
            }]
        }
    }, {
        id: _id.more,
        exClass: 'icon-more editor-icon',
        subMenu: {
            type: _subType.list,
            data: [{
                type: _commonConst2['default'].TYPE.TABLE.DISTRIBUTE_COLS,
                name: _commonLang2['default'].Table.DistrbuteCols,
                exClass: '',
                isSplit: false
            }, {
                type: _commonConst2['default'].TYPE.TABLE.DELETE_TABLE,
                name: _commonLang2['default'].Table.DeleteTable,
                exClass: '',
                isSplit: true
            }]
        }
    }];

    var i, m;

    menu = _commonEnv2['default'].doc.createElement(_commonConst2['default'].TAG.TMP_TAG);
    _domUtilsDomExtend2['default'].addClass(menu, _commonConst2['default'].CLASS.TABLE_TOOLS);

    var menuHtml = '<ul>';
    for (i = 0; i < menuData.length; i++) {
        m = menuData[i];
        menuHtml += '<li id="' + m.id + '" class="' + _commonConst2['default'].CLASS.TABLE_MENU_ITEM + '">' + '<div class="' + _commonConst2['default'].CLASS.TABLE_MENU_BUTTON + '">' + '<i class="' + m.exClass + '"></i>';
        if (m.id === _id.bg) {
            menuHtml += '<i id="' + _id.bgDemo + '" class="editor-icon icon-inner_box"></i>';
        }
        menuHtml += '</div>';
        if (m.subMenu.type === _subType.list) {
            menuHtml += createSubMenuForList(m.subMenu.data);
        } else {
            menuHtml += m.subMenu.make();
        }
        menuHtml += '</li>';
    }
    menuHtml += '</ul>';
    menu.innerHTML = menuHtml;

    colorPadDemo = menu.querySelector('#' + _id.bgDemo);
    if (colorPadDemo) {
        colorPadDemo.style.color = '#fff';
    }

    return menu;
}

function createSubMenuForList(data) {
    var i,
        m,
        html = '<ul class="wiz-table-menu-sub">';
    for (i = 0; i < data.length; i++) {
        m = data[i];
        html += '<li class="wiz-table-menu-sub-item ' + _class.clickItem;
        if (m.isSplit) {
            html += ' split';
        }
        html += '" data-type="' + m.type + '">' + m.name;
        html += '</li>';
    }
    html += '</ul>';
    return html;
}

function getMenuTop() {
    var top,
        tableBody = menuObj.parentNode.querySelector('.' + _commonConst2['default'].CLASS.TABLE_BODY),
        tableBodyTop = tableBody ? tableBody.offsetTop : 0;
    top = tableBodyTop - menuObj.offsetHeight - 5;
    return top + 'px';
}
function fixMenuPos() {
    var container = menuObj.parentNode,
        offset = _domUtilsDomExtend2['default'].getOffset(container),
        scrollTop = _commonEnv2['default'].doc.body.scrollTop;

    if (scrollTop > offset.top - 30 && scrollTop < container.offsetHeight + offset.top - menuObj.offsetHeight * 2.5) {
        _domUtilsDomExtend2['default'].css(menuObj, {
            position: 'fixed',
            top: '0',
            left: offset.left + 'px'
        });
    } else {
        _domUtilsDomExtend2['default'].css(menuObj, {
            position: '',
            top: getMenuTop(),
            left: ''
        });
    }
}

var _event = {
    bind: function bind() {
        _event.unbind();
        if (menuObj) {
            menuObj.addEventListener('click', _event.handler.onClick);
            menuObj.addEventListener('mouseover', _event.handler.onMouseOver);
        }
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_SCROLL, _event.handler.onScroll);
    },
    unbind: function unbind() {
        if (menuObj) {
            menuObj.removeEventListener('click', _event.handler.onClick);
            menuObj.removeEventListener('mouseover', _event.handler.onMouseOver);
        }
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_SCROLL, _event.handler.onScroll);
    },
    handler: {
        onBeforeSaveSnap: function onBeforeSaveSnap() {
            // 目前不在保存快照前处理
            // tableMenu.hideSub();
        },
        onClick: function onClick(e) {
            //点击 一级菜单
            var item = _domUtilsDomExtend2['default'].getParentByFilter(e.target, function (dom) {
                return _domUtilsDomExtend2['default'].hasClass(dom, _commonConst2['default'].CLASS.TABLE_MENU_BUTTON);
            }, true);
            if (item) {
                tableMenu.showSub(item.parentNode);
                return;
            }

            //点击 菜单具体功能
            var container;
            item = _domUtilsDomExtend2['default'].getParentByFilter(e.target, function (dom) {
                return _domUtilsDomExtend2['default'].hasClass(dom, _class.clickItem);
            }, true);
            if (!item || _domUtilsDomExtend2['default'].hasClass(item, _class.disabled)) {
                return;
            }
            var type = item.getAttribute('data-type');
            var todo = true;
            switch (type) {
                case _commonConst2['default'].TYPE.TABLE.CLEAR_CELL:
                    tableCore.clearCellValue();
                    break;
                case _commonConst2['default'].TYPE.TABLE.MERGE_CELL:
                    tableCore.merge();
                    break;
                case _commonConst2['default'].TYPE.TABLE.SPLIT_CELL:
                    tableCore.split();
                    break;
                case _commonConst2['default'].TYPE.TABLE.INSERT_ROW_UP:
                    tableCore.insertRow(true);
                    break;
                case _commonConst2['default'].TYPE.TABLE.INSERT_ROW_DOWN:
                    tableCore.insertRow();
                    break;
                case _commonConst2['default'].TYPE.TABLE.INSERT_COL_LEFT:
                    tableCore.insertCol(true);
                    break;
                case _commonConst2['default'].TYPE.TABLE.INSERT_COL_RIGHT:
                    tableCore.insertCol();
                    break;
                case _commonConst2['default'].TYPE.TABLE.DELETE_ROW:
                    tableCore.deleteRows();
                    break;
                case _commonConst2['default'].TYPE.TABLE.DELETE_COL:
                    tableCore.deleteCols();
                    break;
                case _commonConst2['default'].TYPE.TABLE.SET_CELL_BG:
                    var bg = item.getAttribute('data-color');
                    tableCore.setCellBg(bg);
                    container = _domUtilsDomExtend2['default'].getParentByFilter(item, function (dom) {
                        return _domUtilsDomExtend2['default'].hasClass(dom, 'wiz-table-color-pad');
                    }, false);
                    _domUtilsDomExtend2['default'].removeClass(container.querySelectorAll('.wiz-table-color-pad .' + _class.colorPadItem + '.' + _class.active), _class.active);
                    _domUtilsDomExtend2['default'].addClass(item, _class.active);
                    colorPadDemo.setAttribute('data-last-color', bg);
                    break;
                case _commonConst2['default'].TYPE.TABLE.SET_CELL_ALIGN:
                    //设置 对齐方式 时，不自动隐藏二级菜单
                    var align = null,
                        valign = null;
                    if (item.getAttribute('data-align-type') == 'align') {
                        align = item.getAttribute('data-align-value');
                    } else {
                        valign = item.getAttribute('data-align-value');
                    }
                    tableCore.setCellAlign(align, valign);

                    container = item.parentNode;
                    _domUtilsDomExtend2['default'].removeClass(container.querySelectorAll('.' + _class.active), _class.active);
                    _domUtilsDomExtend2['default'].addClass(item, _class.active);
                    todo = false;
                    break;
                case _commonConst2['default'].TYPE.TABLE.DELETE_TABLE:
                    tableCore.deleteTable();
                    break;
                case _commonConst2['default'].TYPE.TABLE.DISTRIBUTE_COLS:
                    tableCore.distributeCols();
                    break;
                default:
                    todo = false;
            }

            if (todo) {
                tableMenu.hideSub();
            }
        },
        onMouseOver: function onMouseOver(e) {
            var colorItem = _domUtilsDomExtend2['default'].getParentByFilter(e.target, function (dom) {
                return _domUtilsDomExtend2['default'].hasClass(dom, _class.colorPadItem);
            }, true);
            if (colorItem && colorPadDemo) {
                colorPadDemo.style.color = colorItem.getAttribute('data-color') || '#fff';
            }
        },
        onScroll: function onScroll(e) {
            if (!menuObj || menuObj.style.display == 'none') {
                return;
            }
            fixMenuPos();
        }
    }
};

var tableMenu = {
    init: function init(_tableCore) {
        tableCore = _tableCore;
    },
    hide: function hide() {
        if (menuObj) {
            menuObj.style.display = 'none';
        }
        _event.unbind();
    },
    hideSub: function hideSub() {
        if (!menuObj) {
            return;
        }
        var sub = menuObj.querySelectorAll('.' + _commonConst2['default'].CLASS.TABLE_MENU_ITEM + '.' + _class.active);
        _domUtilsDomExtend2['default'].removeClass(sub, _class.active);

        if (colorPadDemo) {
            colorPadDemo.style.color = colorPadDemo.getAttribute('data-last-color') || '#fff';
        }
    },
    isMenu: function isMenu(dom) {
        if (!dom) {
            return false;
        }
        return !!_domUtilsDomExtend2['default'].getParentByFilter(dom, function (p) {
            return _domUtilsDomExtend2['default'].hasClass(p, _commonConst2['default'].CLASS.TABLE_TOOLS);
        }, true);
    },
    remove: function remove() {
        if (menuObj) {
            menuObj.parentNode.removeChild(menuObj);
            menuObj = null;
        }
    },
    show: function show() {
        if (_commonEnv2['default'].client.type.isPhone || _commonEnv2['default'].client.type.isPad) {
            return;
        }
        var zone = _tableZone2['default'].getZone();
        if (!zone.grid || !zone.range) {
            tableMenu.hide();
            return;
        }

        var container = _domUtilsDomExtend2['default'].getParentByFilter(zone.table, function (dom) {
            return _domUtilsDomExtend2['default'].hasClass(dom, _commonConst2['default'].CLASS.TABLE_CONTAINER);
        }, false);
        menuObj = createMenu();
        tableMenu.hideSub();
        container.appendChild(menuObj);
        _domUtilsDomExtend2['default'].css(menuObj, {
            top: getMenuTop()
        });
        menuObj.style.display = 'block';

        fixMenuPos();
        _event.bind();
    },
    showSub: function showSub(item) {
        if (_domUtilsDomExtend2['default'].hasClass(item, _class.active)) {
            _domUtilsDomExtend2['default'].removeClass(item, _class.active);
            return;
        }

        //控制二级菜单 默认值
        var canMerge,
            canSplit,
            cellAlign,
            subItem,
            zone = _tableZone2['default'].getZone();
        if (item.id === _id.cells) {
            canMerge = _tableUtils2['default'].canMerge(zone.grid, zone.range);
            canSplit = _tableUtils2['default'].canSplit(zone.grid, zone.range);

            subItem = item.querySelector('[data-type=' + _commonConst2['default'].TYPE.TABLE.MERGE_CELL + ']');
            if (subItem && canMerge) {
                _domUtilsDomExtend2['default'].removeClass(subItem, _class.disabled);
            } else if (subItem) {
                _domUtilsDomExtend2['default'].addClass(subItem, _class.disabled);
            }

            subItem = item.querySelector('[data-type=' + _commonConst2['default'].TYPE.TABLE.SPLIT_CELL + ']');
            if (subItem && canSplit) {
                _domUtilsDomExtend2['default'].removeClass(subItem, _class.disabled);
            } else if (subItem) {
                _domUtilsDomExtend2['default'].addClass(subItem, _class.disabled);
            }
        } else if (item.id === _id.align) {
            cellAlign = _tableUtils2['default'].getAlign(zone.grid, zone.range);
            subItem = item.querySelector('.' + _class.alignItem + '.' + _class.active + '[data-align-type=align]');
            if (subItem && (!cellAlign.align || subItem.getAttribute('data-align-value').toLowerCase() !== cellAlign.align)) {
                _domUtilsDomExtend2['default'].removeClass(subItem, _class.active);
                subItem = null;
            }
            if (!subItem && cellAlign.align) {
                subItem = item.querySelector('[data-align-value=' + cellAlign.align + ']');
                _domUtilsDomExtend2['default'].addClass(subItem, _class.active);
            }

            subItem = item.querySelector('.' + _class.alignItem + '.' + _class.active + '[data-align-type=valign]');
            if (subItem && (!cellAlign.valign || subItem.getAttribute('data-align-value').toLowerCase() !== cellAlign.valign)) {
                _domUtilsDomExtend2['default'].removeClass(subItem, _class.active);
                subItem = null;
            }
            if (!subItem && cellAlign.valign) {
                subItem = item.querySelector('[data-align-value=' + cellAlign.valign + ']');
                _domUtilsDomExtend2['default'].addClass(subItem, _class.active);
            }
        }

        tableMenu.hideSub();
        _domUtilsDomExtend2['default'].addClass(item, _class.active);
    }
};

exports['default'] = tableMenu;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/lang":8,"../domUtils/domExtend":15,"./tableUtils":28,"./tableZone":29}],28:[function(require,module,exports){
/**
 * 表格操作的基本方法集合
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

// import utils from './../common/utils';

var _domUtilsDomExtend = require('./../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

/**
 * table 相关的 默认值
 * @type {{ColWidth: number, ColWidthMin: number, RowHeightMin: number}}
 */
var Default = {
    ColWidth: 120, //默认列宽
    ColWidthMin: 30, //最小列宽
    RowHeightMin: 33 //最小行高
};

var tableUtils = {
    Default: Default,
    /**
     * 初始化 默认值
     * @param options
     */
    init: function init(options) {
        if (!options) {
            return;
        }
        if (options.colWidth) {
            Default.ColWidth = options.colWidth;
        }
        if (options.colWidthMin) {
            Default.ColWidthMin = options.colWidthMin;
        }
        if (options.rowHeightMin) {
            Default.RowHeightMin = options.rowHeightMin;
        }
    },
    /**
     * 判断当前是否允许新建表格
     * @param zone
     * @returns {boolean}
     */
    canCreateTable: function canCreateTable(zone) {
        var range = _rangeUtilsRangeExtend2['default'].getRange(),
            tmpCell;
        if (range) {
            tmpCell = _domUtilsDomExtend2['default'].getParentByTagName(range.startContainer, ['table'], true, null) || _domUtilsDomExtend2['default'].getParentByTagName(range.endContainer, ['table'], true, null);
            if (tmpCell) {
                return false;
            }
        }
        return !zone.range;
    },
    /**
     * 判断选择的单元格是否能进行 合并操作
     * @param grid
     * @param range
     * @returns {*|boolean}
     */
    canMerge: function canMerge(grid, range) {
        return grid && range && grid[range.minY][range.minX].cell !== grid[range.maxY][range.maxX].cell;
    },
    /**
     * 判断选择的单元格是否能被拆分
     * @param grid
     * @param range
     * @returns {*}
     */
    canSplit: function canSplit(grid, range) {
        if (!grid || !range) {
            return false;
        }
        var key;
        var splitMap = {},
            canSplit = false;
        tableUtils.eachRange(grid, range, function (cellData) {
            key = cellData.x_src + '_' + cellData.y_src;
            if (cellData.fake && !splitMap[key]) {
                splitMap[key] = grid[cellData.y_src][cellData.x_src];
                canSplit = true;
            }
        });
        return canSplit ? splitMap : false;
    },
    /**
     * 检查 并 初始化表格容器
     * @param _table
     */
    checkTableContainer: function checkTableContainer(_table, readonly) {
        var tableList = _table ? [_table] : _commonEnv2['default'].doc.querySelectorAll('table'),
            table,
            container,
            tableBody,
            i,
            j;

        for (i = 0, j = tableList.length; i < j; i++) {
            table = tableList[i];
            tableBody = checkParent(table, function (parent) {
                return _domUtilsDomExtend2['default'].hasClass(parent, _commonConst2['default'].CLASS.TABLE_BODY);
            });
            container = checkParent(tableBody, function (parent) {
                return _domUtilsDomExtend2['default'].hasClass(parent, _commonConst2['default'].CLASS.TABLE_CONTAINER);
            });

            _domUtilsDomExtend2['default'].addClass(container, _commonConst2['default'].CLASS.TABLE_CONTAINER);
            //避免 编辑、阅读状态切换时， 表格位置闪动，所以做成 inline 模式
            _domUtilsDomExtend2['default'].css(container, {
                position: 'relative',
                padding: '15px 0 5px'
            });
            _domUtilsDomExtend2['default'].addClass(tableBody, _commonConst2['default'].CLASS.TABLE_BODY);
            _domUtilsDomExtend2['default'].removeClass(tableBody, _commonConst2['default'].CLASS.TABLE_MOVING);
            if (!readonly) {
                container.setAttribute('contenteditable', 'false');
                setTdEditable(table, 'td');
                setTdEditable(table, 'th');
            }
            //直接设置 table contenteditable 会导致 chrome 检查时报错
            // table.setAttribute('contenteditable', 'true');
        }

        function setTdEditable(table, tdType) {
            var tdList = table.querySelectorAll(tdType),
                i;
            for (i = tdList.length - 1; i >= 0; i--) {
                tdList[i].setAttribute('contenteditable', 'true');
            }
        }

        function checkParent(obj, filter) {
            var parent = obj.parentNode;
            if (!filter(parent)) {
                parent = _commonEnv2['default'].doc.createElement('div');
                _domUtilsDomExtend2['default'].insert(obj, parent);
                parent.appendChild(obj);
            }
            return parent;
        }
    },
    /**
     * 清空选中区域单元格内的数据
     * @param grid
     * @param range
     */
    clearCellValue: function clearCellValue(grid, range) {
        if (!grid || !range) {
            return;
        }
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                cellData.cell.innerHTML = '<br/>';
            }
        });
    },
    /**
     * 复制单元格 Dom
     * @param cell
     * @param isClear
     * @returns {Element}
     */
    cloneCell: function cloneCell(cell, isClear) {
        var newCell = _commonEnv2['default'].doc.createElement(cell.tagName);
        newCell.style.cssText = cell.style.cssText;
        if (isClear) {
            newCell.innerHTML = '<br/>';
        } else {
            newCell.colSpan = cell.colSpan;
            newCell.rowSpan = cell.rowSpan;
            newCell.innerHTML = cell.innerHTML;
        }
        // TODO 处理 已选中的 cell
        return newCell;
    },
    /**
     * 创建 单元格
     * @param width
     * @returns {Element}
     */
    createCell: function createCell(width) {
        var td = _commonEnv2['default'].doc.createElement('td');
        td.setAttribute('align', 'left');
        td.setAttribute('valign', 'middle');
        if (width) {
            td.setAttribute('style', 'width:' + width + 'px');
        }
        td.appendChild(_commonEnv2['default'].doc.createElement('br'));
        return td;
    },
    /**
     * 创建 表格
     * @param col
     * @param row
     * @returns {Element}
     */
    createTable: function createTable(col, row) {
        if (!col || !row) {
            return;
        }

        var table = _commonEnv2['default'].doc.createElement('table'),
            tbody = _commonEnv2['default'].doc.createElement('tbody'),
            tr,
            c,
            r;

        for (r = 0; r < row; r++) {
            tr = _commonEnv2['default'].doc.createElement('tr');
            for (c = 0; c < col; c++) {
                tr.appendChild(tableUtils.createCell(Default.ColWidth));
            }
            tbody.appendChild(tr);
        }

        table.appendChild(tbody);
        table.style.width = Default.ColWidth * col + 'px';
        return table;
    },
    /**
     * 删除指定的列
     * @param grid
     * @param col
     */
    deleteCols: function deleteCols(grid, col) {
        if (!grid || grid.length === 0 || col > grid[0].length) {
            return;
        }
        var table = _domUtilsDomExtend2['default'].getParentByTagName(grid[0][0].cell, 'table', false, null);

        var tmpCellList = [],
            width = Default.ColWidth;

        var y, g, cell;
        for (y = 0; y < grid.length; y++) {
            g = grid[y][col];
            if (g.y_src == y && g.cell.colSpan > 1) {
                g.cell.colSpan--;
                tmpCellList.push(g.cell);
            } else if (g.y_src == y) {
                width = tableUtils.getCellWidth(g.cell);
                g.cell.parentElement.removeChild(g.cell);
            }
            grid[y].splice(col, 1);
        }

        for (y = 0; y < tmpCellList.length; y++) {
            cell = tmpCellList[y];
            cell.style.width = tableUtils.getCellWidth(cell) - width + 'px';
        }

        //如果所有单元格都删除了，则删除表格
        if (!table.getElementsByTagName('td').length && !table.getElementsByTagName('th').length) {
            table.parentElement.removeChild(table);
        } else {
            tableUtils.fixTableWidth(table);
        }
    },
    /**
     * 删除指定的行
     * @param grid
     * @param row
     */
    deleteRows: function deleteRows(grid, row) {
        if (!grid || grid.length === 0 || row > grid.length) {
            return;
        }

        var table = _domUtilsDomExtend2['default'].getParentByTagName(grid[0][0].cell, 'table', false, null),
            rows = table.rows;

        var x, g, cellData;
        for (x = grid[row].length - 1; x >= 0; x--) {
            g = grid[row][x];
            if (g.x_src == x && g.y_src < g.y) {
                g.cell.rowSpan--;
            } else if (g.x_src == x && g.y_src == g.y && g.cell.rowSpan > 1 && row + 1 < grid.length) {
                //row+1 防止表格异常的 rowSpan 设置

                g.cell.rowSpan--;
                cellData = tableUtils.getNextCellDataInRow(grid[row + 1], x);
                cellData = cellData ? cellData.cell : null;
                rows[row + 1].insertBefore(g.cell, cellData);
            }
        }
        grid.splice(row, 1);
        rows[row].parentElement.removeChild(rows[row]);

        //如果所有单元格都删除了，则删除表格
        if (!table.getElementsByTagName('tr').length) {
            table.parentElement.removeChild(table);
        } else {
            tableUtils.fixTableWidth(table);
        }
    },
    /**
     * 平均分配每列
     * @param table
     * @param grid
     */
    distributeCols: function distributeCols(table, grid) {
        if (!table || !grid) {
            return;
        }
        var colCount = grid[0].length;
        if (colCount === 0) {
            return;
        }

        var rows = table.rows,
            w = table.offsetWidth / colCount,
            y,
            x,
            cell;

        for (y = rows.length - 1; y >= 0; y--) {
            for (x = rows[y].cells.length - 1; x >= 0; x--) {
                cell = rows[y].cells[x];
                cell.style.width = w * cell.colSpan + 'px';
            }
        }
        table.style.width = table.offsetWidth + 'px';
    },
    /**
     * each 循环遍历 选中区域的单元格
     * @param grid
     * @param range
     * @param callback
     */
    eachRange: function eachRange(grid, range, callback) {
        if (!grid || !range || !callback || typeof callback !== 'function') {
            return;
        }

        var x,
            y,
            cbBreak = true;
        for (y = range.minY; cbBreak !== false && y < grid.length && y <= range.maxY; y++) {
            for (x = range.minX; cbBreak !== false && x < grid[y].length && x <= range.maxX; x++) {
                cbBreak = callback(grid[y][x]);
            }
        }
    },
    /**
     * 修正选中区域的 selection
     */
    fixSelection: function fixSelection() {
        //避免选择文本时， 选中到 表格内部
        var range = _rangeUtilsRangeExtend2['default'].getRange();
        if (!range || range.collapsed) {
            return;
        }

        var start = range.startContainer,
            startOffset = range.startOffset,
            end = range.endContainer,
            endOffset = range.endOffset,
            startTr = _domUtilsDomExtend2['default'].getParentByTagName(start, 'tr', true, null),
            endTr = _domUtilsDomExtend2['default'].getParentByTagName(end, 'tr', true, null);
        if (!startTr && !endTr || startTr && endTr) {
            return;
        }

        var table,
            target = startTr ? startTr : endTr;

        while (table = _domUtilsDomExtend2['default'].getParentByTagName(target, 'table', true, null)) {
            if (startTr) {
                target = _domUtilsDomExtend2['default'].getNextNode(target, false, end);
            } else {
                target = _domUtilsDomExtend2['default'].getPreviousNode(target, false, start);
            }
        }

        if (startTr) {
            start = target ? target : end;
            startOffset = 0;
        } else {
            end = target ? target : start;
            endOffset = _domUtilsDomExtend2['default'].getDomEndOffset(end);
        }

        if (startTr) {
            _rangeUtilsRangeExtend2['default'].setRange(end, endOffset, start, startOffset);
        } else {
            _rangeUtilsRangeExtend2['default'].setRange(start, startOffset, end, endOffset);
        }
    },
    /**
     * 修正 table 宽度
     * @param table
     */
    fixTableWidth: function fixTableWidth(table) {
        if (!table) {
            return;
        }
        var rows = table.rows,
            i,
            cell,
            w,
            tableWidth = 0;
        for (i = 0; i < rows[0].cells.length; i++) {
            cell = rows[0].cells[i];
            w = tableUtils.getCellWidth(cell);
            tableWidth += w;
        }
        table.style.width = tableWidth + 'px';
    },
    /**
     * 获取 选中区域内综合的单元格对齐方式
     * @param grid
     * @param range
     * @returns {*}
     */
    getAlign: function getAlign(grid, range) {
        if (!grid || !range) {
            return false;
        }
        var align,
            valign,
            cell,
            result = {
            align: '',
            valign: ''
        };
        tableUtils.eachRange(grid, range, function (cellData) {
            cell = cellData.cell;
            if (!cellData.fake) {
                align = cell.align.toLowerCase();
                valign = cell.vAlign.toLowerCase();
            }

            if (result.align === '') {
                result.align = align;
                result.valign = valign;
            }

            if (result.align !== null) {
                result.align = result.align === align ? align : null;
            }
            if (result.valign !== null) {
                result.valign = result.valign === valign ? valign : null;
            }

            return result.align !== null || result.valign !== null;
        });

        return result;
    },
    /**
     * 获取单元格宽度
     * @param cell
     * @returns {Number}
     */
    getCellWidth: function getCellWidth(cell) {
        return parseInt(cell.style.width || cell.offsetWidth, 10);
    },
    /**
     * 根据 单元格 dom 获取 grid 内对应的 data 数据
     * @param grid
     * @param cell
     * @returns {*}
     */
    getCellData: function getCellData(grid, cell) {
        if (!grid || !cell) {
            return null;
        }
        var i, j, g;
        for (i = 0; i < grid.length; i++) {
            for (j = 0; j < grid[i].length; j++) {
                g = grid[i][j];
                if (g.cell === cell) {
                    return g;
                }
            }
        }
        return null;
    },
    /**
     * 根据 rang 范围 获取 单元格的 data 列表
     * @param grid
     * @param range
     * @returns {Array}
     */
    getCellsByRange: function getCellsByRange(grid, range) {
        var cellList = [];
        if (!grid || !range) {
            return cellList;
        }
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                cellList.push(cellData.cell);
            }
        });
        return cellList;
    },
    /**
     * 从 cell 集合中遍历获取 cell 内的叶子节点集合
     * @param cellList
     * @returns {Array}
     */
    getDomsByCellList: function getDomsByCellList(cellList) {
        var i,
            j,
            cell,
            tmpList,
            domList = [];
        if (!cellList) {
            return domList;
        }
        for (i = 0, j = cellList.length; i < j; i++) {
            cell = cellList[i];
            tmpList = _domUtilsDomExtend2['default'].getDomListA2B({
                startDom: cell.firstChild,
                startOffset: 0,
                endDom: cell.lastChild,
                endOffset: 1,
                noSplit: true
            });
            domList = domList.concat(tmpList.list);
        }
        return domList;
    },
    /**
     * 在表格内根据指定单元格 获取 下一个 单元格，到达最后一列时，自动从下一行查找
     * @param cell
     * @returns {*}
     */
    getNextCellInTable: function getNextCellInTable(cell) {
        var nextCell = cell.nextElementSibling;
        if (nextCell) {
            return nextCell;
        }
        var tr = cell.parentNode.nextElementSibling;
        while (tr) {
            if (tr.cells.length > 0) {
                return tr.cells[0];
            }
            tr = tr.nextElementSibling;
        }
        return null;
    },
    /**
     * 在一行内 根据列数获取下一个单元格
     * @param gridRow
     * @param col
     * @returns {*}
     */
    getNextCellDataInRow: function getNextCellDataInRow(gridRow, col) {
        if (!gridRow) {
            return null;
        }
        var i;
        for (i = col; i < gridRow.length; i++) {
            if (!gridRow[i].fake) {
                return gridRow[i];
            }
        }
        return null;
    },
    /**
     * 根据 mouse 相关事件，获取 对应的 坐标位置
     * @param e
     * @param table
     * @returns {{clientX: *, clientY: *}}
     */
    getEventPosition: function getEventPosition(e, table) {
        if (!table) {
            table = e.target ? _domUtilsDomExtend2['default'].getParentByTagName(e.target, 'table', false, null) : null;
        }
        var clientX = e.clientX + _commonEnv2['default'].doc.body.scrollLeft + (table ? table.parentNode.scrollLeft : 0);
        var clientY = e.clientY + _commonEnv2['default'].doc.body.scrollTop + (table ? table.parentNode.scrollTop : 0);
        return {
            x: clientX,
            y: clientY
        };
    },
    /**
     * 在一行内 根据列数获取上一个单元格
     * @param gridRow
     * @param col
     * @returns {*}
     */
    getPreviousCellDataInRow: function getPreviousCellDataInRow(gridRow, col) {
        if (!gridRow) {
            return null;
        }
        var i;
        for (i = col; i >= 0; i--) {
            if (!gridRow[i].fake) {
                return gridRow[i];
            }
        }
        return null;
    },
    /**
     * 根据 节点的 cellData 获取单元格所占面积
     * @param cellData
     * @returns {*}
     */
    getRangeByCellData: function getRangeByCellData(cellData) {
        if (!cellData) {
            return {
                minX: 0,
                minY: 0,
                maxX: 0,
                maxY: 0
            };
        }
        return {
            minX: cellData.x_src,
            minY: cellData.y_src,
            maxX: cellData.x_src + cellData.cell.colSpan - 1,
            maxY: cellData.y_src + cellData.cell.rowSpan - 1
        };
    },
    /**
     * 根据起始单元格的 data 数据 获取 grid 中的 range
     * @param grid
     * @param startData
     * @param endData
     * @returns {*}
     */
    getRangeByCellsData: function getRangeByCellsData(grid, startData, endData) {
        if (!grid || !startData || !endData) {
            return null;
        }

        var startRange = tableUtils.getRangeByCellData(startData);
        if (startData.cell === endData.cell) {
            return startRange;
        }
        var endRange = tableUtils.getRangeByCellData(endData);

        var minX = Math.min(startRange.minX, endRange.minX),
            minY = Math.min(startRange.minY, endRange.minY),
            maxX = Math.max(startRange.maxX, endRange.maxX),
            maxY = Math.max(startRange.maxY, endRange.maxY),
            _minX,
            _minY,
            _maxX,
            _maxY;

        var x,
            y,
            g,
            gRange,
            k,
            cellMap = {},
            changeRange = true;

        // console.log(minX + ',' + minY + ' - ' + maxX + ',' + maxY);
        while (changeRange) {
            changeRange = false;
            _minX = minX;
            _minY = minY;
            _maxX = maxX;
            _maxY = maxY;
            for (y = minY; y <= maxY; y++) {
                for (x = minX; x <= maxX; x++) {
                    // console.log('['+x+','+y+']' +minX + ',' + minY + ' - ' + maxX + ',' + maxY);
                    //遍历范围时，只需要寻找边缘的 Cell 即可
                    if (y > minY && y < maxY && x < maxX - 1) {
                        x = maxX - 1;
                        continue;
                    }

                    g = grid[y][x];
                    k = g.x_src + '_' + g.y_src;
                    if (cellMap[k]) {
                        //如果该 Cell 已经被计算过，则不需要重新计算
                        continue;
                    }

                    gRange = tableUtils.getRangeByCellData(g);
                    minX = Math.min(minX, gRange.minX);
                    minY = Math.min(minY, gRange.minY);
                    maxX = Math.max(maxX, gRange.maxX);
                    maxY = Math.max(maxY, gRange.maxY);

                    if (minX !== _minX || minY !== _minY || maxX !== _maxX || maxY !== _maxY) {
                        changeRange = true;
                        break;
                    }
                }
                if (changeRange) {
                    break;
                }
            }
        }

        return {
            minX: minX,
            minY: minY,
            maxX: maxX,
            maxY: maxY
        };
    },
    /**
     * 根据 表格 获取 grid
     * @param table
     * @returns {*}
     */
    getTableGrid: function getTableGrid(table) {
        if (!table || !_domUtilsDomExtend2['default'].isTag(table, 'table')) {
            return null;
        }
        var grid = [];
        var c, r, rows, row, cells, cell, colSpan, rowSpan, i, j, x, y, x_src, y_src, startX;

        rows = table.rows;
        for (r = 0; r < rows.length; r++) {
            row = rows[r];
            cells = row.cells;

            if (!grid[r]) {
                grid[r] = [];
            }
            for (c = 0; c < cells.length; c++) {
                cell = cells[c];
                colSpan = cell.colSpan;
                rowSpan = cell.rowSpan;

                startX = getX(c, r);
                for (i = 0; i < rowSpan; i++) {
                    if (!grid[r + i]) {
                        grid[r + i] = [];
                    }
                    for (j = 0; j < colSpan; j++) {
                        y = r + i;
                        x = getX(startX + j, y);
                        if (i == 0 && j == 0) {
                            x_src = x;
                            y_src = y;
                        }
                        grid[y][x] = {
                            cell: cell,
                            x: x,
                            y: y,
                            x_src: x_src,
                            y_src: y_src,
                            fake: i > 0 || j > 0
                        };
                    }
                }
            }
        }

        return grid;

        function getX(index, y) {
            while (grid[y][index]) {
                index++;
            }
            return index;
        }
    },
    /**
     * 分析 并处理 剪切板内得到的 html 代码
     * @param html
     * @returns {{isTable: boolean, pasteDom: *}}
     */
    getTemplateByHtmlForPaste: function getTemplateByHtmlForPaste(html) {
        var pasteTables,
            pasteTable,
            pasteIsTable = false,
            pasteDom,
            i,
            j,
            template = _commonEnv2['default'].doc.createElement('div');

        //excel 复制时， </html>后面有乱码，需要过滤
        if (html.indexOf('</html>') > -1) {
            html = html.substr(0, html.indexOf('</html>') + 7);
        }

        template.innerHTML = html;
        //清理无效dom
        _domUtilsDomExtend2['default'].childNodesFilter(template);

        pasteTables = template.querySelectorAll('table');
        if (pasteTables.length == 1) {
            pasteTable = pasteTables[0];
            pasteTable.parentNode.removeChild(pasteTable);
            if (_domUtilsDomExtend2['default'].isEmptyDom(template)) {
                pasteIsTable = true;
                pasteDom = pasteTable;
            } else {
                //如果不是单一表格，恢复 innerHTML 便于后面标准处理
                template.innerHTML = html;
            }
        }

        if (!pasteIsTable) {
            pasteTables = template.querySelectorAll('table');
            //表格内 禁止粘贴表格，所以需要把表格全部变为 text
            for (i = pasteTables.length - 1; i >= 0; i--) {
                pasteTable = pasteTables[i];
                _domUtilsDomExtend2['default'].insert(pasteTable, _commonEnv2['default'].doc.createTextNode(pasteTable.innerText));
                pasteTable.parentNode.removeChild(pasteTable);
            }
            //清理 template 内的多余 nodeType
            for (i = template.childNodes.length - 1; i >= 0; i--) {
                j = template.childNodes[i];
                if (j.nodeType !== 1 && j.nodeType !== 3 && _domUtilsDomExtend2['default'].isEmptyDom(j)) {
                    template.removeChild(j);
                }
            }
            pasteDom = template;
        }
        return {
            isTable: pasteIsTable,
            pasteDom: pasteDom
        };
    },
    /**
     * 分析 并处理 剪切板内得到的 text 代码
     * @param txt
     * @returns {{isTable: boolean, pasteDom: Element}}
     */
    getTemplateByTxtForPaste: function getTemplateByTxtForPaste(txt) {
        txt = (txt || '').trim();
        var rows = txt.split('\n'),
            x,
            y,
            cols,
            table = _commonEnv2['default'].doc.createElement('table'),
            tbody = _commonEnv2['default'].doc.createElement('tbody'),
            tr,
            td,
            maxX = 0;

        table.appendChild(tbody);
        for (y = 0; y < rows.length; y++) {
            cols = rows[y].split('\t');
            tr = _commonEnv2['default'].doc.createElement('tr');
            for (x = 0; x < cols.length; x++) {
                td = tableUtils.createCell();
                if (cols[x]) {
                    td.innerHTML = '';
                    td.appendChild(_commonEnv2['default'].doc.createTextNode(cols[x]));
                }
                tr.appendChild(td);
            }
            maxX = Math.max(maxX, tr.cells.length);
            tbody.appendChild(tr);
        }

        //避免 table 列数不一致
        rows = table.rows;
        for (y = 0; y < rows.length; y++) {
            tr = rows[y];
            cols = tr.cells;
            for (x = cols.length; x < maxX; x++) {
                tr.appendChild(tableUtils.createCell());
            }
        }

        return {
            isTable: true,
            pasteDom: table
        };
    },
    /**
     * 针对 html 源码隐藏表格的高亮信息，主要用于保存操作
     * @param html
     * @returns {*}
     */
    hideTableFromHtml: function hideTableFromHtml(html) {
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
            if ((' ' + str + ' ').indexOf(' ' + _commonConst2['default'].CLASS.SELECTED_CELL + ' ') > -1) {
                reg = new RegExp(' ' + _commonConst2['default'].CLASS.SELECTED_CELL + ' ', 'ig');
                str = (' ' + str + ' ').replace(reg, '').trim();
            }

            result.push(html.substr(lastIndex, m.index - lastIndex), m[1], str, m[3]);

            lastIndex = m.index + m[0].length;
            //console.log(m);
        }
        result.push(html.substr(lastIndex));
        return result.join('');
    },
    /**
     * 初始化 表格 样式
     * @param table
     */
    initTable: function initTable(table) {
        var i,
            j,
            cell,
            needInit = false;
        if (table.style.width.indexOf('%') > -1) {
            needInit = true;
        } else {
            for (j = table.rows[0].cells.length - 1; j >= 0; j--) {
                cell = table.rows[0].cells[j];
                if (cell.style.width.indexOf('%') > -1) {
                    needInit = true;
                }
            }
        }
        if (!needInit) {
            return;
        }

        for (i = table.rows.length - 1; i >= 0; i--) {
            for (j = table.rows[i].cells.length - 1; j >= 0; j--) {
                cell = table.rows[i].cells[j];
                if (cell.style.width.indexOf('%') > -1) {
                    cell.style.width = cell.offsetWidth + 'px';
                }
            }
        }
        table.style.width = table.offsetWidth + 'px';
    },
    /**
     * 在指定的位置插入列
     * @param grid
     * @param col
     */
    insertCol: function insertCol(grid, col) {
        if (!grid) {
            return;
        }
        col = col || 0;
        var y, gRow, g, cell, newCell, nextCellData;
        var table = _domUtilsDomExtend2['default'].getParentByTagName(grid[0][0].cell, 'table', false, null),
            rows = table.rows,
            lastCell = null;
        for (y = 0; y < grid.length; y++) {
            gRow = grid[y];

            if (gRow.length > col) {
                g = grid[y][col];
                cell = g.cell;
            } else {
                g = null;
                cell = null;
            }

            if (cell && cell !== lastCell && g.x_src < col) {
                //cell.colSpan > 1
                g.cell.colSpan++;

                // 需要调整 style
                g.cell.style.width = tableUtils.getCellWidth(g.cell) + Default.ColWidth + 'px';
            } else if (!cell || cell && g.x_src == col) {

                newCell = tableUtils.createCell(Default.ColWidth);
                if (cell && g.y_src < g.y) {
                    //cell.rowSpan > 1
                    nextCellData = tableUtils.getNextCellDataInRow(grid[y], col);
                    rows[y].insertBefore(newCell, nextCellData ? nextCellData.cell : null);
                } else {
                    rows[y].insertBefore(newCell, cell);
                }
            }
            lastCell = g ? g.cell : null;
        }

        tableUtils.fixTableWidth(table);
    },
    /**
     * 在指定的位置插入行
     * @param grid
     * @param row
     */
    insertRow: function insertRow(grid, row) {
        if (!grid) {
            return;
        }
        row = row || 0;
        var x, g, newCell;
        var table = _domUtilsDomExtend2['default'].getParentByTagName(grid[0][0].cell, 'table', false, null),
            tr = _commonEnv2['default'].doc.createElement('tr');
        var gRow = grid[grid.length > row ? row : grid.length - 1];
        for (x = 0; x < gRow.length; x++) {
            g = gRow[x];

            if (grid.length > row && g.y_src < g.y && g.x_src == g.x) {
                //cell.rowSpan > 1
                g.cell.rowSpan++;
                // TODO 需要调整 style( height)
            } else if (grid.length <= row || g.y_src == g.y) {
                    newCell = tableUtils.cloneCell(g.cell, true);
                    if (g.cell.colSpan > 1) {
                        newCell.style.width = g.cell.offsetWidth / g.cell.colSpan + 'px';
                    }
                    tr.appendChild(newCell);
                }
        }

        var target = gRow[0].cell.parentElement,
            parent = target.parentElement;
        if (grid.length <= row) {
            target = null;
        }
        parent.insertBefore(tr, target);
    },
    /**
     * 将指定的单元格范围进行合并
     * @param grid
     * @param range
     * @returns {*}
     */
    mergeCell: function mergeCell(grid, range) {
        if (!tableUtils.canMerge(grid, range)) {
            return null;
        }

        var dy = range.maxY - range.minY + 1,
            dx = range.maxX - range.minX + 1;

        var target = grid[range.minY][range.minX].cell;
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake && cellData.cell != target) {
                cellData.cell.parentNode.removeChild(cellData.cell);
            }
        });
        target.rowSpan = dy;
        target.colSpan = dx;
        return target;
    },
    /**
     * 设置单元格对齐方式
     * @param grid
     * @param range
     * @param _alignType
     */
    setCellAlign: function setCellAlign(grid, range, _alignType) {
        if (!grid || !range) {
            return;
        }

        var alignType = {};
        if (_alignType.align != null) {
            alignType.align = _alignType.align || 'left';
        }
        if (_alignType.valign != null) {
            alignType.valign = _alignType.valign || 'middle';
        }

        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                if (alignType.align) {
                    _domUtilsDomExtend2['default'].css(cellData.cell, { 'text-align': '' });
                }
                if (alignType.valign) {
                    _domUtilsDomExtend2['default'].css(cellData.cell, { 'text-valign': '' });
                }
                _domUtilsDomExtend2['default'].attr(cellData.cell, alignType);
            }
        });
    },
    /**
     * 设置单元格背景颜色
     * @param grid
     * @param range
     * @param bgColor
     */
    setCellBg: function setCellBg(grid, range, bgColor) {
        if (!grid || !range) {
            return;
        }

        bgColor = bgColor || '';
        if (bgColor.toLowerCase() === 'transparent') {
            bgColor = '';
        }

        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                _domUtilsDomExtend2['default'].css(cellData.cell, {
                    'background-color': bgColor
                });
            }
        });
    },
    /**
     * 设置列宽
     * @param table
     * @param grid
     * @param col
     * @param dx
     */
    setColWidth: function setColWidth(table, grid, col, dx) {
        dx = fixDx();
        var tableWidth = table.offsetWidth + dx;
        var i,
            j,
            g,
            key,
            cells = [],
            cellMap = {};
        for (i = 0, j = grid.length; i < j; i++) {
            g = grid[i][col];
            key = getKey(g);
            if (!cellMap[key]) {
                cellMap[key] = g.cell.offsetWidth + dx;
                cells.push(g);
            }
        }
        table.style.width = tableWidth + 'px';
        for (i = 0, j = cells.length; i < j; i++) {
            g = cells[i];
            g.cell.style.width = cellMap[getKey(g)] + 'px';
        }

        function getKey(g) {
            return g.x_src + '_' + g.y_src;
        }

        function fixDx() {
            var y,
                g,
                cell,
                maxDx = dx,
                tmpDx;
            for (y = 0; y < grid.length; y++) {
                g = grid[y][col];
                tmpDx = Default.ColWidthMin - g.cell.offsetWidth;
                if (g.cell.colSpan == 1) {
                    maxDx = tmpDx;
                    cell = g.cell;
                    break;
                }
                if (maxDx < tmpDx) {
                    maxDx = tmpDx;
                    cell = g.cell;
                }
            }

            if (dx < maxDx) {
                return maxDx;
            } else {
                return dx;
            }
        }
    },
    /**
     * 设置行高
     * @param table
     * @param grid
     * @param row
     * @param dy
     */
    setRowHeight: function setRowHeight(table, grid, row, dy) {
        var x,
            g,
            cell,
            maxDy = dy,
            tmpDy;
        for (x = 0; x < grid[row].length; x++) {
            g = grid[row][x];
            tmpDy = Default.RowHeightMin - g.cell.offsetHeight;
            if (g.cell.rowSpan == 1) {
                maxDy = tmpDy;
                cell = g.cell;
                break;
            }
            if (maxDy < tmpDy) {
                maxDy = tmpDy;
                cell = g.cell;
            }
        }

        if (cell) {
            if (dy < maxDy) {
                cell.parentNode.style.height = Default.RowHeightMin + 'px';
            } else {
                cell.parentNode.style.height = g.cell.offsetHeight + dy + 'px';
            }
        }
    },
    /**
     * 拆分单元格
     * @param table
     * @param grid
     * @param range
     * @returns {*}
     */
    splitCell: function splitCell(table, grid, range) {
        var x, y, g, key, dx, dy;
        var splitMap = tableUtils.canSplit(grid, range);

        if (!splitMap) {
            return null;
        }
        var item, nextCell, newCell;
        for (key in splitMap) {
            if (splitMap.hasOwnProperty(key)) {
                g = splitMap[key];
                dy = g.cell.rowSpan;
                dx = g.cell.colSpan;
                for (y = g.y_src; y < g.y_src + dy; y++) {
                    for (x = g.x_src; x < g.x_src + dx; x++) {
                        item = grid[y][x];
                        if (item.fake) {
                            nextCell = tableUtils.getNextCellDataInRow(grid[y], x);
                            nextCell = nextCell ? nextCell.cell : null;
                            newCell = tableUtils.cloneCell(item.cell, true);
                            table.rows[y].insertBefore(newCell, nextCell);
                            item.fake = false;
                            item.cell = newCell;
                            item.y_src = y;
                            item.x_src = x;
                        } else {
                            item.cell.rowSpan = 1;
                            item.cell.colSpan = 1;
                        }

                        //TODO 需要多做测试检测这样是否可行
                        item.cell.style.width = '';
                    }
                }
            }
        }
        return range;
    }
};

exports['default'] = tableUtils;
module.exports = exports['default'];

},{"../rangeUtils/rangeExtend":23,"./../common/const":4,"./../common/env":6,"./../domUtils/domExtend":15}],29:[function(require,module,exports){
/*
 表格选择区域 控制
 */
'use strict';

Object.defineProperty(exports, '__esModule', {
    value: true
});

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('../common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('../common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonUtils = require('../common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _commonHistoryUtils = require('../common/historyUtils');

var _commonHistoryUtils2 = _interopRequireDefault(_commonHistoryUtils);

var _tableUtils = require('./tableUtils');

var _tableUtils2 = _interopRequireDefault(_tableUtils);

var _domUtilsDomExtend = require('../domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _rangeUtilsRangeExtend = require('../rangeUtils/rangeExtend');

var _rangeUtilsRangeExtend2 = _interopRequireDefault(_rangeUtilsRangeExtend);

//import wizStyle from '../common/wizStyle';

var updateRenderTimer, updateRenderTimes, domModifiedTimer;

var zone = {
    active: false,
    table: null,
    start: null,
    end: null,
    range: null,
    grid: null
};
function initZone(table) {
    zone.table = table;
    zone.grid = _tableUtils2['default'].getTableGrid(zone.table);
}

function clearSelectedCell() {
    if (!zone.table) {
        return;
    }
    var cells = zone.table.getElementsByClassName(_commonConst2['default'].CLASS.SELECTED_CELL);
    var i;
    for (i = cells.length - 1; i >= 0; i--) {
        _domUtilsDomExtend2['default'].removeClass(cells[i], _commonConst2['default'].CLASS.SELECTED_CELL);
    }
}

function getCellsDataByRange() {
    if (!zone.grid || !zone.range) {
        return null;
    }
    var cells = [];
    _tableUtils2['default'].eachRange(zone.grid, zone.range, function (cellData) {
        if (!cellData.fake) {
            cells.push(cellData);
        }
    });
    return cells;
}
function getDomById(parent, id, tagName) {
    var dom = parent.querySelector('#' + id);
    if (!dom) {
        dom = _commonEnv2['default'].doc.createElement(tagName);
        dom.id = id;
        parent.appendChild(dom);
    }
    return dom;
}
function hasMergeCell() {
    if (!zone.grid || !zone.range) {
        return false;
    }
    var hasMerge = false;
    _tableUtils2['default'].eachRange(zone.grid, zone.range, function (cellData) {
        hasMerge = cellData.fake;
        return !hasMerge;
    });
    return false;
}
function isSingleCell() {
    if (!zone.grid || !zone.range) {
        return false;
    }
    var cellA = zone.grid[zone.range.minY][zone.range.minX],
        cellB = zone.grid[zone.range.maxY][zone.range.maxX],
        start = zone.start;
    return cellA.cell == cellB.cell && cellB.cell == start.cell;
}
function isStartFocus() {
    var range = _rangeUtilsRangeExtend2['default'].getRange();
    if (!range) {
        return true;
    }
    var start, end, endOffset;
    if (zone.grid && zone.start) {
        start = _domUtilsDomExtend2['default'].getParentByTagName(range.startContainer, ['th', 'td'], true, null);
        end = range.collapsed ? start : _domUtilsDomExtend2['default'].getParentByTagName(range.endContainer, ['th', 'td'], true, null);
    }

    //当前没有选中单元格  或  当前已选中该单元格时 true
    if (!zone.start || zone.start.cell == start && start == end) {
        return true;
    }
    if (!range.collapsed && zone.start.cell == start && start != end && range.endOffset === 0 && end == _tableUtils2['default'].getNextCellInTable(start)) {
        //如果单元格不是该行最后一个，全选时，endContainer 为下一个 td，且 endOffset 为 0
        //如果单元格是该行最后一个，全选时，endContainer 为下一行的第一个 td
        //这时候必须修正 range， 否则由于 amendUtils.splitAmendDomByRange 的修正，会导致输入的 第1个字符进入到下一个 td 内
        end = start.lastChild;
        endOffset = _domUtilsDomExtend2['default'].getDomEndOffset(end);
        //如果不延迟，会导致 选中的区域异常
        setTimeout(function () {
            _rangeUtilsRangeExtend2['default'].setRange(range.startContainer, range.startOffset, end, endOffset);
        }, 200);

        return true;
    }
    return false;
}
function selectCellsData(cellsData) {
    if (!cellsData) {
        return;
    }
    var i, j;
    for (i = 0, j = cellsData.length; i < j; i++) {
        _domUtilsDomExtend2['default'].addClass(cellsData[i].cell, _commonConst2['default'].CLASS.SELECTED_CELL);
    }
}

function colLineRender(x) {
    if (!zone.table) {
        return;
    }
    var rangeBorder = getRangeBorder();
    var minX = rangeBorder.colLine.minLeft;
    if (x < minX) {
        x = minX;
    }
    _domUtilsDomExtend2['default'].css(rangeBorder.colLine, {
        top: zone.table.offsetTop + 'px',
        left: x + 'px',
        height: zone.table.offsetHeight + 'px',
        display: 'block'
    }, false);

    rangeBorder.container.style.display = 'block';
}
function rowLineRender(y) {
    if (!zone.table) {
        return;
    }

    var rangeBorder = getRangeBorder();
    var minY = rangeBorder.rowLine.minTop;
    if (y < minY) {
        y = minY;
    }
    _domUtilsDomExtend2['default'].css(rangeBorder.rowLine, {
        left: zone.table.offsetLeft + 'px',
        top: y + 'px',
        width: zone.table.offsetWidth + 'px',
        display: 'block'
    }, false);

    rangeBorder.container.style.display = 'block';
}

function checkTableContainer(rangeBorder) {
    _tableUtils2['default'].checkTableContainer(zone.table, false);
    var tableBody = zone.table.parentNode;
    tableBody.appendChild(rangeBorder.container);
}

function rangeRender() {
    clearSelectedCell();
    selectCellsData(getCellsDataByRange());

    var rangeBorder = getRangeBorder();
    if (!zone.start || !zone.range) {
        rangeBorder.container.style.display = 'none';
        rangeBorder.start.dom.style.display = 'none';
        rangeBorder.range.dom.style.display = 'none';
        return;
    }
    // console.log(rangeBorder);
    checkTableContainer(rangeBorder);

    var topSrc = _commonEnv2['default'].doc.body.clientTop;
    var leftSrc = _commonEnv2['default'].doc.body.clientLeft;
    var sLeft, sTop, sWidth, sHeight;
    var rLeft, rTop, rWidth, rHeight;

    var rangeCellStart = zone.start ? zone.start.cell : null;
    var rangeCell_A = zone.grid[zone.range.minY][zone.range.minX];
    var rangeCell_B = zone.grid[zone.range.maxY][zone.range.maxX];
    if (!rangeCell_A || !rangeCell_B) {
        return;
    }
    rangeCell_A = rangeCell_A.cell;
    rangeCell_B = rangeCell_B.cell;

    if (rangeCellStart) {
        sTop = topSrc + rangeCellStart.offsetTop;
        sLeft = leftSrc + rangeCellStart.offsetLeft;
        sWidth = rangeCellStart.offsetWidth;
        sHeight = rangeCellStart.offsetHeight;
    }

    rTop = topSrc + rangeCell_A.offsetTop;
    rLeft = leftSrc + rangeCell_A.offsetLeft;
    if (rangeCell_A == rangeCell_B) {
        rWidth = rangeCell_A.offsetWidth;
        rHeight = rangeCell_A.offsetHeight;
    } else {
        rWidth = rangeCell_B.offsetLeft + rangeCell_B.offsetWidth - rLeft;
        rHeight = rangeCell_B.offsetTop + rangeCell_B.offsetHeight - rTop;
    }

    _domUtilsDomExtend2['default'].css(rangeBorder.start.dom, {
        top: sTop + 'px',
        left: sLeft + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.start.top, {
        width: sWidth + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.start.left, {
        height: sHeight + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.start.bottom, {
        top: sHeight - 1 + 'px',
        width: sWidth + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.start.right, {
        left: sWidth - 1 + 'px',
        height: sHeight + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.start.dot, {
        top: sHeight - 1 - 4 + 'px',
        left: sWidth - 1 - 4 + 'px'
    }, false);

    _domUtilsDomExtend2['default'].css(rangeBorder.range.dom, {
        top: rTop + 'px',
        left: rLeft + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.range.top, {
        width: rWidth + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.range.left, {
        height: rHeight + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.range.bottom, {
        top: rHeight + 'px',
        width: rWidth + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.range.right, {
        left: rWidth + 'px',
        height: rHeight + 'px'
    }, false);
    _domUtilsDomExtend2['default'].css(rangeBorder.range.dot, {
        top: rHeight - 4 + 'px',
        left: rWidth - 4 + 'px'
    }, false);

    rangeBorder.start.dom.style.display = 'block';
    if (isSingleCell()) {
        rangeBorder.start.dot.style.display = 'block';
        rangeBorder.range.dom.style.display = 'none';
    } else {
        rangeBorder.start.dot.style.display = 'none';
        rangeBorder.range.dom.style.display = 'block';
    }
    rangeBorder.container.style.display = 'block';

    //TODO 目前功能未制作，暂时隐藏，以后实现了再显示
    rangeBorder.start.dot.style.display = 'none';
    rangeBorder.range.dot.style.display = 'none';

    setStartRange();
}

function getRangeBorder() {
    var rangeBorder = {
        container: null,
        rowLine: null,
        colLine: null,
        start: {
            dom: null,
            top: null,
            right: null,
            bottom: null,
            left: null,
            dot: null
        },
        range: {
            dom: null,
            top: null,
            right: null,
            bottom: null,
            left: null,
            dot: null
        }
    };
    rangeBorder.container = getDomById(_commonEnv2['default'].doc.body, _commonConst2['default'].ID.TABLE_RANGE_BORDER, _commonConst2['default'].TAG.TMP_TAG);
    _domUtilsDomExtend2['default'].attr(rangeBorder.container, {
        contenteditable: 'false'
    });
    rangeBorder.colLine = getDomById(rangeBorder.container, _commonConst2['default'].ID.TABLE_COL_LINE, 'div');
    rangeBorder.rowLine = getDomById(rangeBorder.container, _commonConst2['default'].ID.TABLE_ROW_LINE, 'div');

    rangeBorder.start.dom = getDomById(rangeBorder.container, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start', 'div');
    rangeBorder.start.top = getDomById(rangeBorder.start.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_top', 'div');
    rangeBorder.start.right = getDomById(rangeBorder.start.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_right', 'div');
    rangeBorder.start.bottom = getDomById(rangeBorder.start.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_bottom', 'div');
    rangeBorder.start.left = getDomById(rangeBorder.start.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_left', 'div');
    rangeBorder.start.dot = getDomById(rangeBorder.start.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_dot', 'div');

    rangeBorder.range.dom = getDomById(rangeBorder.container, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range', 'div');
    rangeBorder.range.top = getDomById(rangeBorder.range.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_top', 'div');
    rangeBorder.range.right = getDomById(rangeBorder.range.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_right', 'div');
    rangeBorder.range.bottom = getDomById(rangeBorder.range.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_bottom', 'div');
    rangeBorder.range.left = getDomById(rangeBorder.range.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_left', 'div');
    rangeBorder.range.dot = getDomById(rangeBorder.range.dom, _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_dot', 'div');
    return rangeBorder;
}
function setStartRange() {
    var sel;
    //选中多个单元格时，取消光标
    if (zone.grid && zone.range && !isSingleCell()) {
        sel = _commonEnv2['default'].doc.getSelection();
        sel.empty();
        return;
    }
    if (!isStartFocus()) {
        _rangeUtilsRangeExtend2['default'].setRange(zone.start.cell, zone.start.cell.childNodes.length);
    }
}

var _event = {
    bind: function bind() {
        _event.unbind();
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_SELECT_CHANGE, _event.handler.onSelectionChange);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.UPDATE_RENDER, _event.handler.updateRender);
        if (zone.table) {
            //单元格内插入图片时， 因为加载图片时长为知，触发 modified 的时候， 图片还未加载完毕，
            //但加载完毕后，肯定会触发 body 的 resize事件
            zone.table.addEventListener('DOMSubtreeModified', _event.handler.onDomModified);
            _commonEnv2['default'].doc.body.addEventListener('resize', _event.handler.onDomModified);
        }
    },
    unbind: function unbind() {
        var zone = tableZone.getZone();
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_SELECT_CHANGE, _event.handler.onSelectionChange);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.UPDATE_RENDER, _event.handler.updateRender);
        if (zone.table) {
            zone.table.removeEventListener('DOMSubtreeModified', _event.handler.onDomModified);
            _commonEnv2['default'].doc.body.removeEventListener('resize', _event.handler.onDomModified);
        }
    },
    bindStopSelectStart: function bindStopSelectStart() {
        _event.unbindStopSelectStart();
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_SELECT_START, _event.handler.onStopSelectStart);
    },
    unbindStopSelectStart: function unbindStopSelectStart() {
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_SELECT_START, _event.handler.onStopSelectStart);
    },
    bindDragLine: function bindDragLine() {
        _event.unbindDragLine();
        _event.bindStopSelectStart();
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onDragLineMove);
        _commonEnv2['default'].event.add(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onDragLineEnd);
    },
    unbindDragLine: function unbindDragLine() {
        _event.unbindStopSelectStart();
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_MOVE, _event.handler.onDragLineMove);
        _commonEnv2['default'].event.remove(_commonConst2['default'].EVENT.ON_MOUSE_UP, _event.handler.onDragLineEnd);
    },
    handler: {
        onDragLineMove: function onDragLineMove(e) {
            var rangeBorder = getRangeBorder();
            var pos = _tableUtils2['default'].getEventPosition(e, zone.table);
            if (rangeBorder.colLine.style.display == 'block') {
                colLineRender(pos.x - rangeBorder.colLine.startMouse + rangeBorder.colLine.startLine);
            } else {
                rowLineRender(pos.y - rangeBorder.rowLine.startMouse + rangeBorder.rowLine.startLine);
            }
        },
        onDragLineEnd: function onDragLineEnd(e) {
            _event.unbindDragLine();
            var rangeBorder = getRangeBorder();
            var pos = _tableUtils2['default'].getEventPosition(e, zone.table);
            var cellData;

            var isDragCol = rangeBorder.colLine.style.display == 'block';
            var isDragRow = rangeBorder.rowLine.style.display == 'block';

            rangeBorder.colLine.style.display = 'none';
            rangeBorder.rowLine.style.display = 'none';
            _commonHistoryUtils2['default'].saveSnap(false);
            if (isDragCol && rangeBorder.colLine.startMouse !== pos.x) {
                cellData = rangeBorder.colLine.cellData;
                if (cellData) {
                    _tableUtils2['default'].initTable(zone.table);
                    _tableUtils2['default'].setColWidth(zone.table, zone.grid, cellData.x, pos.x - rangeBorder.colLine.startMouse);
                }
            } else if (isDragRow && rangeBorder.rowLine.startMouse !== pos.y) {
                cellData = rangeBorder.rowLine.cellData;
                if (cellData) {
                    _tableUtils2['default'].initTable(zone.table);
                    _tableUtils2['default'].setRowHeight(zone.table, zone.grid, cellData.y, pos.y - rangeBorder.rowLine.startMouse);
                }
            }

            rangeBorder.colLine.cellData = null;
            rangeBorder.colLine.minLeft = null;
            rangeBorder.colLine.startLine = null;
            rangeBorder.colLine.startMouse = null;
            rangeBorder.rowLine.cellData = null;
            rangeBorder.rowLine.minTop = null;
            rangeBorder.rowLine.startLine = null;
            rangeBorder.rowLine.startMouse = null;

            rangeRender();
        },
        onSelectionChange: function onSelectionChange(e) {
            //当选中单元格时，不允许 选中 start 单元格以外的任何内容
            var sel = _commonEnv2['default'].doc.getSelection();
            if (!isStartFocus()) {
                sel.empty();
                // rangeUtils.setRange(zone.start.cell, zone.start.cell.childNodes.length);
                _commonUtils2['default'].stopEvent(e);
            }
        },
        onDomModified: function onDomModified(e) {
            var needAutoRetry = e && e.type == 'DOMSubtreeModified' && e.target.nodeType === 1 && e.target.querySelector('img');
            if (domModifiedTimer) {
                clearTimeout(domModifiedTimer);
            }
            domModifiedTimer = setTimeout(function () {
                _event.handler.updateRender(e, needAutoRetry);
            }, 100);
        },
        onStopSelectStart: function onStopSelectStart(e) {
            _commonUtils2['default'].stopEvent(e);
            return false;
        },
        updateRender: function updateRender(e, needAutoRetry) {
            updateRenderTimes = 0;
            autoUpdate(needAutoRetry);

            function autoUpdate(needAutoRetry) {
                //单元格内容变化时，必须重绘，保证 高亮边框的高度与表格一致
                rangeRender();
                //如果 变化的内容里面有 img，则需要延迟监听，等 img 渲染完毕
                if (needAutoRetry && updateRenderTimes < 60) {
                    if (updateRenderTimer) {
                        clearTimeout(updateRenderTimer);
                    }
                    updateRenderTimer = setTimeout(function () {
                        updateRenderTimes++;
                        autoUpdate(needAutoRetry);
                    }, 500);
                }
            }
        }
    }
};

var tableZone = {
    clear: function clear() {
        zone.active = false;
        zone.start = null;
        zone.end = null;
        zone.range = null;
        zone.grid = null;

        rangeRender();

        var rangeBorder = getRangeBorder();
        rangeBorder.colLine.style.display = 'none';
        rangeBorder.rowLine.style.display = 'none';

        //table 必须最后清空，因为还要清除 table 里面 cell 的选择状态
        zone.table = null;
        _event.unbind();
        return tableZone;
    },
    /**
     * 为 复制/剪切 操作，准备 fragment
     */
    getFragmentForCopy: function getFragmentForCopy() {
        var fragment = null;
        //无选中单元格时，不进行任何操作
        if (!zone.range) {
            return fragment;
        }

        var x,
            y,
            g,
            table = _commonEnv2['default'].doc.createElement('table'),
            tbody = _commonEnv2['default'].doc.createElement('tbody'),
            tr,
            td;

        table.appendChild(tbody);
        for (y = zone.range.minY; y <= zone.range.maxY; y++) {
            tr = _commonEnv2['default'].doc.createElement('tr');
            for (x = zone.range.minX; x <= zone.range.maxX; x++) {
                g = zone.grid[y][x];
                if (!g.fake) {
                    td = _tableUtils2['default'].cloneCell(g.cell, false);
                    if (tr.children.length > 0) {
                        //保证 复制的纯文本 有 列间隔
                        tr.appendChild(_commonEnv2['default'].doc.createTextNode('\t'));
                    }
                    tr.appendChild(td);
                }
            }
            //保证 复制的纯文本 有 行间隔
            tr.appendChild(_commonEnv2['default'].doc.createTextNode('\n'));
            tbody.appendChild(tr);
        }

        fragment = _commonEnv2['default'].doc.createElement('div');
        fragment.appendChild(table);
        return fragment;
    },
    getRangeBorder: getRangeBorder,
    getSelectedCells: function getSelectedCells() {
        return _tableUtils2['default'].getCellsByRange(zone.grid, zone.range);
    },
    getZone: function getZone() {
        return {
            active: zone.active,
            table: zone.table,
            start: zone.start,
            end: zone.end,
            range: zone.range,
            grid: zone.grid
        };
    },
    hasMergeCell: hasMergeCell,
    isRangeActiving: function isRangeActiving() {
        return zone.start && zone.active;
    },
    isSingleCell: isSingleCell,
    isZoneBorder: function isZoneBorder(e) {
        var obj = e.target,
            x = e.offsetX,
            y = e.offsetY;
        var isScroll,
            isBorder = false,
            isRight = false,
            isBottom = false;

        var isDot = !!_domUtilsDomExtend2['default'].getParentByFilter(obj, function (dom) {
            return dom && dom.nodeType == 1 && (dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_dot' || dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_dot');
        }, true);

        if (!isDot) {
            isRight = !!_domUtilsDomExtend2['default'].getParentByFilter(obj, function (dom) {
                if (dom && dom.nodeType == 1 && (dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_right' || dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_right')) {
                    return true;
                }

                var minX, maxX;
                if (dom && dom.nodeType == 1 && _domUtilsDomExtend2['default'].isTag(dom, ['td', 'th'])) {
                    minX = dom.offsetWidth - 4;
                    maxX = dom.offsetWidth + 4;
                    return x >= minX && x <= maxX;
                }
                return false;
            }, true);
        }
        if (!isRight) {
            isBottom = !!_domUtilsDomExtend2['default'].getParentByFilter(obj, function (dom) {
                if (dom && dom.nodeType == 1 && (dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_bottom' || dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_bottom')) {
                    return true;
                }

                var minY, maxY;
                if (dom && dom.nodeType == 1 && _domUtilsDomExtend2['default'].isTag(dom, ['td', 'th'])) {
                    minY = dom.offsetHeight - 4;
                    maxY = dom.offsetHeight + 4;
                    return y >= minY && y <= maxY;
                }
                return false;
            }, true);
        }
        if (!isRight && !isBottom && !isDot) {
            isBorder = !!_domUtilsDomExtend2['default'].getParentByFilter(obj, function (dom) {
                return dom && dom.nodeType == 1 && dom.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER;
            }, true);
        }

        //span 等 行级元素 clientWidth / clientHeight 为 0
        isScroll = (e.target.clientWidth > 0 && e.target.clientWidth < e.offsetX || e.target.clientHeight > 0 && e.target.clientHeight < e.offsetY) && (e.target.offsetWidth >= e.offsetX || e.target.offsetHeight >= e.offsetY);

        return {
            isBorder: isBorder,
            isBottom: isBottom,
            isDot: isDot,
            isRight: isRight,
            isScroll: isScroll
        };
    },
    modify: function modify(endCell) {
        if (!zone.active || !endCell) {
            return tableZone;
        }
        // console.log('modify');
        var table = _domUtilsDomExtend2['default'].getParentByTagName(endCell, ['table'], true, null);
        if (!table || table !== zone.table) {
            return tableZone;
        }
        var endCellData = _tableUtils2['default'].getCellData(zone.grid, endCell);
        zone.range = _tableUtils2['default'].getRangeByCellsData(zone.grid, zone.start, endCellData);
        zone.end = endCellData;
        rangeRender();

        var tableBody = _domUtilsDomExtend2['default'].getParentByFilter(zone.table, function (dom) {
            return _domUtilsDomExtend2['default'].hasClass(dom, _commonConst2['default'].CLASS.TABLE_BODY);
        }, false);
        _domUtilsDomExtend2['default'].addClass(tableBody, _commonConst2['default'].CLASS.TABLE_MOVING);

        return tableZone;
    },
    remove: function remove() {
        tableZone.clear();
        var rangeBorder = getRangeBorder(),
            parent;
        if (rangeBorder) {
            parent = rangeBorder.container.parentNode;
            if (parent) {
                parent.removeChild(rangeBorder.container);
            }
        }
    },
    setEnd: function setEnd(endCell, isForced) {
        // console.log('setEnd');
        if (isForced) {
            zone.active = true;
        }
        tableZone.modify(endCell);
        zone.active = false;

        setStartRange();
        _commonEnv2['default'].event.call(_commonConst2['default'].EVENT.ON_SELECT_CHANGE, null);

        var tableBody = _domUtilsDomExtend2['default'].getParentByFilter(zone.table, function (dom) {
            return _domUtilsDomExtend2['default'].hasClass(dom, _commonConst2['default'].CLASS.TABLE_BODY);
        }, false);
        _domUtilsDomExtend2['default'].removeClass(tableBody, _commonConst2['default'].CLASS.TABLE_MOVING);
        return tableZone;

        // console.log(zone);
    },
    setStart: function setStart(startCell, curX, curY) {
        // console.log('setStart');
        if (!startCell) {
            tableZone.clear();
            return tableZone;
        }
        var table = _domUtilsDomExtend2['default'].getParentByTagName(startCell, ['table'], true, null);
        if (!table) {
            //防止异常的 标签
            tableZone.clear();
            return tableZone;
        }
        if (table !== zone.table) {
            tableZone.clear();
            initZone(table);
        }
        zone.active = true;
        zone.end = null;
        zone.start = _tableUtils2['default'].getCellData(zone.grid, startCell);
        if (typeof curX !== 'undefined' && typeof curY !== 'undefined') {
            try {
                var tmp = zone.grid[curY][curX];
                if (tmp && tmp.cell == zone.start.cell) {
                    zone.start = tmp;
                }
            } catch (e) {}
        }
        zone.range = _tableUtils2['default'].getRangeByCellsData(zone.grid, zone.start, zone.start);
        rangeRender();
        _event.bind();
        return tableZone;
    },
    setStartRange: setStartRange,
    startDragColLine: function startDragColLine(cell, x) {
        var table, cellData;
        if (cell && cell.nodeType == 1 && cell.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_right') {
            cellData = zone.start;
            cell = zone.start.cell;
            table = zone.table;
        } else if (cell && cell.nodeType == 1 && cell.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_right') {
            cellData = zone.grid[zone.range.maxY][zone.range.maxX];
            cell = cellData.cell;
            table = zone.table;
        } else {
            cell = _domUtilsDomExtend2['default'].getParentByTagName(cell, ['th', 'td'], true, null);
            if (!cell) {
                return;
            }
            table = _domUtilsDomExtend2['default'].getParentByTagName(cell, ['table'], true, null);
            if (!table) {
                return;
            }

            if (table !== zone.table) {
                clearSelectedCell();
                tableZone.clear();
            }
            if (!zone.grid) {
                initZone(table);
            }
            cellData = _tableUtils2['default'].getCellData(zone.grid, cell);
        }

        //如果 cell 是合并的单元格，需要找到 cell 所占的最后一列
        var col = cellData.x,
            nextCellData;
        while (col + 1 < zone.grid[cellData.y].length) {
            col++;
            nextCellData = zone.grid[cellData.y][col];
            if (nextCellData.cell != cell) {
                break;
            }
            cellData = nextCellData;
        }

        var startLeft = cell.offsetLeft + cell.offsetWidth;
        var rangeBorder = getRangeBorder();
        checkTableContainer(rangeBorder);
        rangeBorder.colLine.minLeft = table.offsetLeft;
        rangeBorder.colLine.startLine = startLeft;
        rangeBorder.colLine.startMouse = x;
        rangeBorder.colLine.cellData = cellData;
        colLineRender(startLeft);

        var sel = _commonEnv2['default'].doc.getSelection();
        sel.empty();
        _event.bindDragLine();
    },
    startDragRowLine: function startDragRowLine(cell, y) {
        var table, cellData;
        if (cell && cell.nodeType == 1 && cell.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_start_bottom') {
            cellData = zone.start;
            cell = zone.start.cell;
            table = zone.table;
        } else if (cell && cell.nodeType == 1 && cell.id == _commonConst2['default'].ID.TABLE_RANGE_BORDER + '_range_bottom') {
            cellData = zone.grid[zone.range.maxY][zone.range.maxX];
            cell = cellData.cell;
            table = zone.table;
        } else {
            cell = _domUtilsDomExtend2['default'].getParentByTagName(cell, ['th', 'td'], true, null);
            if (!cell) {
                return;
            }
            table = _domUtilsDomExtend2['default'].getParentByTagName(cell, ['table'], true, null);
            if (!table) {
                return;
            }

            if (table !== zone.table) {
                clearSelectedCell();
                tableZone.clear();
            }
            if (!zone.grid) {
                initZone(table);
            }
            cellData = _tableUtils2['default'].getCellData(zone.grid, cell);
        }

        //如果 cell 是合并的单元格，需要找到 cell 所占的最后一行
        var row = cellData.y,
            nextCellData;
        while (row + 1 < zone.grid.length) {
            row++;
            nextCellData = zone.grid[row][cellData.x];
            if (nextCellData.cell != cell) {
                break;
            }
            cellData = nextCellData;
        }

        var startTop = cell.offsetTop + cell.offsetHeight;
        var rangeBorder = getRangeBorder();
        checkTableContainer(rangeBorder);
        rangeBorder.rowLine.minTop = table.offsetTop;
        rangeBorder.rowLine.startLine = startTop;
        rangeBorder.rowLine.startMouse = y;
        rangeBorder.rowLine.cellData = cellData;
        rowLineRender(startTop);

        var sel = _commonEnv2['default'].doc.getSelection();
        sel.empty();
        _event.bindDragLine();
    },
    switchCell: function switchCell(target, direct) {
        if (!direct || !zone.start) {
            return null;
        }
        //目前不考虑 x、y 为任意值的情况， 只考虑移动一个单元格
        direct.x = !direct.x ? 0 : direct.x > 0 ? 1 : -1;
        direct.y = !direct.y ? 0 : direct.y > 0 ? 1 : -1;
        var x = target.x + direct.x;
        var y = target.y + direct.y;

        changeRowCheck();

        var cellData = target;
        while (y >= 0 && y < zone.grid.length && x >= 0 && x < zone.grid[y].length && cellData.cell == target.cell) {

            cellData = zone.grid[y][x];
            x += direct.x;
            y += direct.y;

            changeRowCheck();
        }

        return cellData;

        function changeRowCheck() {
            if (!!direct.canChangeRow && y >= 0 && y < zone.grid.length) {
                //允许折行
                if (x < 0) {
                    x = zone.grid[y].length - 1;
                    y -= 1;
                } else if (x >= zone.grid[y].length) {
                    x = 0;
                    y += 1;
                }
            }
        }
    },
    updateGrid: function updateGrid() {
        var rangeA, rangeB;
        if (zone.table) {
            if (zone.grid) {
                rangeA = zone.grid[zone.range.minY][zone.range.minX];
                rangeB = zone.grid[zone.range.maxY][zone.range.maxX];
            }
            initZone(zone.table);
            rangeA = _tableUtils2['default'].getCellData(zone.grid, rangeA.cell);
            rangeB = _tableUtils2['default'].getCellData(zone.grid, rangeB.cell);
            zone.range = _tableUtils2['default'].getRangeByCellsData(zone.grid, rangeA, rangeB);
            zone.start = _tableUtils2['default'].getCellData(zone.grid, zone.start.cell);
            if (zone.end) {
                zone.end = _tableUtils2['default'].getCellData(zone.grid, zone.end.cell);
            }
        }
        rangeRender();

        return tableZone;
        // console.log(zone);
    }
};

exports['default'] = tableZone;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/historyUtils":7,"../common/utils":10,"../domUtils/domExtend":15,"../rangeUtils/rangeExtend":23,"./tableUtils":28}],30:[function(require,module,exports){
'use strict';

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { 'default': obj }; }

var _commonEnv = require('./common/env');

var _commonEnv2 = _interopRequireDefault(_commonEnv);

var _commonConst = require('./common/const');

var _commonConst2 = _interopRequireDefault(_commonConst);

var _commonLang = require('./common/lang');

var _commonLang2 = _interopRequireDefault(_commonLang);

var _commonUtils = require('./common/utils');

var _commonUtils2 = _interopRequireDefault(_commonUtils);

var _domUtilsDomExtend = require('./domUtils/domExtend');

var _domUtilsDomExtend2 = _interopRequireDefault(_domUtilsDomExtend);

var _imgUtilsImgUtils = require('./imgUtils/imgUtils');

var _imgUtilsImgUtils2 = _interopRequireDefault(_imgUtilsImgUtils);

var _nightModeNightModeUtils = require('./nightMode/nightModeUtils');

var _nightModeNightModeUtils2 = _interopRequireDefault(_nightModeNightModeUtils);

var _amendAmendInfo = require('./amend/amendInfo');

var _amendAmendInfo2 = _interopRequireDefault(_amendAmendInfo);

var _amendAmendUser = require('./amend/amendUser');

var _amendAmendUser2 = _interopRequireDefault(_amendAmendUser);

var _readerBase = require('./reader/base');

var _readerBase2 = _interopRequireDefault(_readerBase);

var _markdownMarkdownRender = require('./markdown/markdownRender');

var _markdownMarkdownRender2 = _interopRequireDefault(_markdownMarkdownRender);

var WizReader = {
    /**
     * 初始化 修订编辑
     * @param options
     * {
     *   document, //document
     *   lang,      //语言 JSON
     *   userInfo,  //用户数据 JSON
     *   userData,  //kb内所有用户数据集合 Array[JSON]
     *   clientType,  //客户端类型,
     *   noAmend, //Boolean 是否显示 修订信息
     *   ignoreTable, //Boolean 是否忽略 table 的处理
     * }
     */
    init: function init(options) {
        _commonEnv2['default'].setDoc(options.document || window.document);
        (0, _commonLang.initLang)(options.lang);
        _commonEnv2['default'].client.setType(options.clientType);
        _commonEnv2['default'].dependency.files.init(options.dependencyCss, options.dependencyJs);
        _readerBase2['default'].init(options);
        _markdownMarkdownRender2['default'].init();

        _readerBase2['default'].on(options);
        if (!options.noAmend) {
            _amendAmendUser2['default'].initUser(options.userInfo);
            _amendAmendUser2['default'].setUsersData(options.usersData);
            WizReader.amendInfo.on();
        }

        if (_commonEnv2['default'].win.WizTemplate) {
            _commonEnv2['default'].win.WizTemplate.init({
                document: _commonEnv2['default'].doc,
                lang: options.lang,
                clientType: options.clientType
            }).on(true);
        }

        return WizReader;
    },
    on: function on(options) {

        _readerBase2['default'].on(options);

        if (!options.noAmend) {
            WizReader.amendInfo.on();
        }
        if (_commonEnv2['default'].win.WizTemplate) {
            _commonEnv2['default'].win.WizTemplate.on(true);
        }

        return WizReader;
    },
    off: function off() {
        _readerBase2['default'].off();
        WizReader.amendInfo.off();

        if (_commonEnv2['default'].win.WizTemplate) {
            _commonEnv2['default'].win.WizTemplate.off();
        }

        return WizReader;
    },
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        _readerBase2['default'].insertDefaultStyle(isReplace, customCss);

        return WizReader;
    },
    markdown: function markdown(callback, timeout) {
        timeout = timeout ? timeout : 30 * 1000;
        var hasCalled = false,
            cb = function cb() {
            if (callback && /^function$/i.test(typeof callback) && !hasCalled) {
                callback();
                hasCalled = true;
            }
        };
        _markdownMarkdownRender2['default'].markdown({
            markdown: function markdown(isMathjax) {
                //IOS 处理 todolist 应该可以删除了
                _commonEnv2['default'].client.sendCmdToWiznote(_commonConst2['default'].CLIENT_EVENT.wizMarkdownRender);
                if (!isMathjax) {
                    cb();
                } else {
                    setTimeout(cb, timeout);
                }
            },
            mathJax: function mathJax() {
                cb();
            }
        });
    },
    mathJax: function mathJax(callback, timeout) {
        timeout = timeout ? timeout : 30 * 1000;
        var hasCalled = false,
            cb = function cb() {
            if (callback && !hasCalled) {
                callback();
                hasCalled = true;
            }
        };

        setTimeout(cb, timeout);
        _markdownMarkdownRender2['default'].mathJax(function () {
            cb();
        });
    },
    amendInfo: {
        on: function on() {
            _amendAmendInfo2['default'].init({
                readonly: true
            }, {
                onAccept: null,
                onRefuse: null
            });

            return WizReader;
        },
        off: function off() {
            _amendAmendInfo2['default'].remove();

            return WizReader;
        }
    },
    img: {
        getAll: function getAll(onlyLocal) {
            //为了保证客户端使用方便，转换为字符串
            return _imgUtilsImgUtils2['default'].getAll(onlyLocal).join(',');
        }
    },
    nightMode: {
        on: function on(color, bgColor, brightness) {
            _nightModeNightModeUtils2['default'].on(color, bgColor, brightness);
        },
        off: function off() {
            _nightModeNightModeUtils2['default'].off();
        }
    }
};

window.WizReader = WizReader;

},{"./amend/amendInfo":1,"./amend/amendUser":2,"./common/const":4,"./common/env":6,"./common/lang":8,"./common/utils":10,"./domUtils/domExtend":15,"./imgUtils/imgUtils":17,"./markdown/markdownRender":20,"./nightMode/nightModeUtils":21,"./reader/base":24}]},{},[30]);
