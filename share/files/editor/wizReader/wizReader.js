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

        amendInfo.main.setAttribute('contenteditable', 'false');

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
            _commonEnv2['default'].doc.addEventListener('touchstart', _event.handler.onTouchstart);
            _commonEnv2['default'].doc.addEventListener('touchend', _event.handler.onMouseMove);
        } else {
            _commonEnv2['default'].doc.addEventListener('mousemove', _event.handler.onMouseMove);
        }
    },
    unbind: function unbind() {
        if (_commonEnv2['default'].client.type.isIOS || _commonEnv2['default'].client.type.isAndroid) {
            _commonEnv2['default'].doc.removeEventListener('touchstart', _event.handler.onTouchstart);
            _commonEnv2['default'].doc.removeEventListener('touchend', _event.handler.onMouseMove);
        } else {
            _commonEnv2['default'].doc.removeEventListener('mousemove', _event.handler.onMouseMove);
        }
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
        container.setAttribute('contenteditable', 'false');
        mask.appendChild(container);
        mask.id = _commonConst2['default'].ID.AMEND_INFO;
        _domUtilsDomBase2['default'].css(mask, {
            'position': 'absolute',
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
        return '<div id="' + _commonConst2['default'].ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' + '<img id="' + _commonConst2['default'].ID.AMEND_INFO_IMG + '" class="' + _commonConst2['default'].CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute; -webkit-border-radius: 40px;-moz-border-radius:40px;border-radius:40px;">' + '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' + '</li><li style="line-height: 18px;text-align: right;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' + '<p style="margin: 4px 16px;">' + _commonLang2['default'].Amend.MultiInfo + '</p>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_TOOLS + '" style="padding:0; box-sizing: border-box;">' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnRefuse + '</a></div>' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnAccept + '</a></div>' + '</div>';
    }

    //if (ENV.client.type.isWeb || ENV.client.type.isWin) {
    return '<div id="' + _commonConst2['default'].ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' + '<img id="' + _commonConst2['default'].ID.AMEND_INFO_IMG + '" class="' + _commonConst2['default'].CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute;">' + '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' + '</li><li style="line-height: 18px;text-align: right;">' + '<span id="' + _commonConst2['default'].ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' + '<p style="margin: 4px 16px;">' + _commonLang2['default'].Amend.MultiInfo + '</p>' + '</div>' + '<div id="' + _commonConst2['default'].ID.AMEND_INFO_TOOLS + '" style="padding:0; box-sizing: border-box; border-top: 1px solid #D8D8D8">' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;box-sizing: border-box;border-right: 1px solid #D8D8D8">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnAccept + '</a></div>' + '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;box-sizing: border-box;">' + '<a id="' + _commonConst2['default'].ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + _commonLang2['default'].Amend.BtnRefuse + '</a></div>' + '</div>';
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

},{"../common/const":4,"../common/env":6,"../common/lang":7,"../common/utils":8,"../common/wizUserAction":10,"../domUtils/domBase":11,"./amendUser":2,"./amendUtils/amendBase":3}],2:[function(require,module,exports){
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

},{"../common/const":4,"../common/env":6,"../common/utils":8}],3:[function(require,module,exports){
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

},{"../../common/const":4,"../../common/env":6,"../../common/utils":8,"../../domUtils/domBase":11,"../../rangeUtils/rangeBase":17,"./../amendUser":2}],4:[function(require,module,exports){
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
        IMG_NOT_DRAG: 'wiz-img-cannot-drag'
    },
    ATTR: {
        IMG: 'data-wiz-img',
        IMG_MASK: 'data-wiz-img-mask',
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
        AMEND_USER_INFO: 'wiz-amend-user'
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
        }
    },
    EVENT: {
        WizEditorPaste: 'wizEditorPaste',
        wizReaderClickImg: 'wizReaderClickImg',
        wizMarkdownRender: 'wizMarkdownRender',
        wizEditorTrackEvent: 'wizEditorTrackEvent'
    },
    //全局事件 id 集合
    GLOBAL_EVENT: {
        BEFORE_SAVESNAP: 'BEFORE_SAVESNAP',
        AFTER_RESTORE_HISTORY: 'AFTER_RESTORE_HISTORY'
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

function loadGroup(doc, group, callback) {
    _utils2['default'].loadJs(doc, group, callback);
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

},{"./utils":8}],6:[function(require,module,exports){
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
                mathJax: 'http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_HTML'
            },
            init: function init(cssFiles, jsFiles) {
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
                    if (cmd == _const2['default'].EVENT.wizReaderClickImg) {
                        url = WizNotCmdInditify + cmd + '?src=' + encodeURIComponent(options.src);
                    } else if (cmd == _const2['default'].EVENT.wizEditorTrackEvent) {
                        url = WizNotCmdInditify + cmd + '?id=' + encodeURIComponent(options.id) + '&e=' + encodeURIComponent(options.event);
                    } else {
                        url = WizNotCmdInditify + cmd;
                    }

                    var iframe = document.createElement("iframe");
                    iframe.setAttribute("src", url);
                    document.documentElement.appendChild(iframe);
                    iframe.parentNode.removeChild(iframe);
                    iframe = null;
                };
            } else if (type.indexOf('android') > -1) {
                ENV.client.type.isAndroid = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    if (cmd == _const2['default'].EVENT.wizReaderClickImg) {
                        window.WizNote.onClickImg(options.src, options.imgList);
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
    globalEvent: {
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
        add: function add(eventId, fun) {
            if (!eventId || !fun || ENV.globalEvent.checkFun(eventId, fun)) {
                return;
            }
            var eList = GlobalEvent[eventId];
            if (!eList) {
                eList = [];
            }
            eList.push(fun);
            GlobalEvent[eventId] = eList;
        },
        checkFun: function checkFun(eventId, fun) {
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
    Err: {
        Copy_Null: '無法複製已刪除的內容',
        Cut_Null: '無法剪切已刪除的內容'
    }
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

},{}],8:[function(require,module,exports){
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
        e.stopPropagation();
        e.preventDefault();
        //这个会阻止其他同event 的触发，过于野蛮
        //e.stopImmediatePropagation();
    },
    //-------------------- 以下内容修改需要 保证与 wizUI 中的 utils 内 对应方法一致 start ----------------------
    PcCustomTagClass: 'wiz-html-render-unsave', //此 class 专门用于 pc 端将 markdown 笔记选然后发email 或微博等处理
    loadCount: {},
    loadJs: function loadJs(doc, jsList, loadCallback) {
        if (!jsList) {
            return;
        }

        var i,
            j,
            s,
            id = new Date().valueOf(),
            allLoaded = true;
        for (i = 0, j = jsList.length; i < j; i++) {
            s = this.loadSingleJs(doc, jsList[i]);
            if (s !== true) {
                s.onload = this.makeLoadHandle(id, loadCallback);
                allLoaded = false;
            }
        }
        if (allLoaded) {
            loadCallback();
        }
    },
    makeLoadHandle: function makeLoadHandle(id, loadCallback) {
        if (!this.loadCount[id]) {
            this.loadCount[id] = 0;
        }
        this.loadCount[id]++;
        var _this = this;
        return function () {
            _this.loadCount[id]--;
            if (_this.loadCount[id] === 0) {
                _this.loadCount[id] = null;
                if (loadCallback) {
                    loadCallback();
                }
            }
        };
    },
    loadSingleCss: function loadSingleCss(doc, path) {
        var cssId = 'wiz_' + path;
        if (doc.getElementById(cssId)) {
            return true;
        }

        var s = doc.createElement('link');
        s.rel = 'stylesheet';
        s.href = path.replace(/\\/g, '/');
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    loadSingleJs: function loadSingleJs(doc, path) {
        var jsId = 'wiz_' + path;
        if (doc.getElementById(jsId)) {
            return true;
        }
        var s = doc.createElement('script');
        s.type = 'text/javascript';
        s.src = path.replace(/\\/g, '/');
        s.id = jsId;
        s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    appendJsCode: function appendJsCode(doc, jsStr, type) {
        var s = doc.createElement('script');
        s.type = type;
        s.text = jsStr;
        s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].appendChild(s);
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
            // 防止innerText后产生换行符
            var span = $("<span></span>");
            var parent = $(this).parent();
            span.text(htmlUnEncode(parent[0].outerHTML));
            span.insertAfter(parent);
            parent.remove();
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
            if (href && href.indexOf("wiz:") === 0) {
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

},{"./const":4}],9:[function(require,module,exports){
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

var TmpStyleName = 'wiz_tmp_editor_style',
    TmpEditorStyle = {
    phone: 'body {' + 'overflow-y:scroll;' + '-webkit-overflow-scrolling: touch;' + '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' + '}',
    pad: 'body {' + 'min-width: 90%;' + 'max-width: 100%;' + 'min-height: 100%;' + 'background: #ffffff;' + 'overflow-y:scroll;' + '-webkit-overflow-scrolling: touch;' + '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' + '}'
},
    TmpReaderStyle = {
    phone: 'img {' + 'max-width: 100%;' + 'height: auto !important;' + 'margin: 0px auto;' + '}' + 'a {' + 'word-wrap: break-word;' + '}' + 'body {' + 'word-wrap: break-word;' + '}'
},
    DefaultStyleId = 'wiz_custom_css',
    DefaultFont = 'Helvetica, "Hiragino Sans GB", "Microsoft Yahei", SimSun, SimHei, arial, sans-serif;',
    DefaultStyle = {
    common: 'html, body {' + 'font-size: 15px;' + '}' + 'body {' + 'font-family: ' + DefaultFont + 'line-height: 1.6;' + 'padding: 0;margin: 20px 36px;margin: 1.33rem 2.4rem;' + '}' + 'h1, h2, h3, h4, h5, h6 {margin:20px 0 10px;margin:1.33rem 0 0.667rem;padding: 0;font-weight: bold;}' + 'h1 {font-size:21px;font-size:1.4rem;}' + 'h2 {font-size:20px;font-size:1.33rem;}' + 'h3 {font-size:18px;font-size:1.2rem;}' + 'h4 {font-size:17px;font-size:1.13rem;}' + 'h5 {font-size:15px;font-size:1rem;}' + 'h6 {font-size:15px;font-size:1rem;color: #777777;margin: 1rem 0;}' + 'div, p, blockquote, ul, ol, dl, table, pre {margin:10px 0;margin:0.667rem 0;}' + 'ul, ol {padding-left:32px;padding-left:2.13rem;}' + 'blockquote {border-left:4px solid #dddddd;padding:0 12px;padding:0 0.8rem;color: #aaa;}' + 'blockquote > :first-child {margin-top:0;}' + 'blockquote > :last-child {margin-bottom:0;}' + 'img {border:0;max-width:100%;height:auto !important;}' + 'table {border-collapse:collapse;border:1px solid #bbbbbb;}' + 'td {border-collapse:collapse;border:1px solid #bbbbbb;}' + '@media screen and (max-width: 660px) {' + 'body {margin:20px 18px;margin:1.33rem 1.2rem;}' + '}' + '@media only screen and (-webkit-max-device-width: 1024px), only screen and (-o-max-device-width: 1024px), only screen and (max-device-width: 1024px), only screen and (-webkit-min-device-pixel-ratio: 3), only screen and (-o-min-device-pixel-ratio: 3), only screen and (min-device-pixel-ratio: 3) {' + 'html,body {font-size:17px;}' + 'body {line-height:1.7;margin:12px 15px;margin:0.75rem 0.9375rem;color:#353c47;text-align:justify;text-justify:inter-word;}' + 'h1 {font-size:34px;font-size:2.125rem;}' + 'h2 {font-size:30px;font-size:1.875rem;}' + 'h3 {font-size:26px;font-size:1.625rem;}' + 'h4 {font-size:22px;font-size:1.375rem;}' + 'h5 {font-size:18px;font-size:1.125rem;}' + 'h6 {color: inherit;}' + 'div, p, blockquote, ul, ol, dl, table, pre {margin:0;}' + 'ul, ol {padding-left:40px;padding-left:2.5rem;}' + 'blockquote {border-left:4px solid #c8d4e8;padding:0 15px;padding:0 0.9375rem;color: #b3c2dd;}' + '}'
};

function insertStyleById(id, css, isReplace) {
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
function insertStyleByName(name, css) {
    var s = _env2['default'].doc.createElement('style');
    s.name = name;
    _env2['default'].doc.getElementsByTagName('HEAD')[0].insertBefore(s, null);
    s.innerHTML = css;
}
function removeStyleByName(name) {
    var s = _env2['default'].doc.getElementsByName(name);
    var i, style;
    for (i = s.length - 1; i >= 0; i--) {
        style = s[i];
        style.parentNode.removeChild(style);
    }
}
function removeStyleByNameFromHtml(html, name) {
    var reg = new RegExp('<style[^<>]*[ ]+name *= *[\'"]' + name + '[\'"][^<>]*>[^<>]*<\/style>', 'ig');
    return html.replace(reg, '');
}

var WizStyle = {
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        insertStyleById(DefaultStyleId, DefaultStyle.common, isReplace);
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
            insertStyleByName(TmpStyleName, css);
        }
    },
    insertTmpEditorStyle: function insertTmpEditorStyle() {
        if (_env2['default'].client.type.isIOS && _env2['default'].client.type.isPhone) {
            insertStyleByName(TmpStyleName, TmpEditorStyle.phone);
        } else if (_env2['default'].client.type.isIOS && _env2['default'].client.type.isPad) {
            insertStyleByName(TmpStyleName, TmpEditorStyle.pad);
        } else if (_env2['default'].client.type.isWin) {
            insertStyleByName(TmpStyleName, TmpReaderStyle.win);
        }
    },
    insertTmpReaderStyle: function insertTmpReaderStyle() {
        if (_env2['default'].client.type.isIOS) {
            insertStyleByName(TmpStyleName, TmpReaderStyle.phone);
        } else if (_env2['default'].client.type.isWin) {
            insertStyleByName(TmpStyleName, TmpReaderStyle.win);
        }
    },
    removeTmpStyle: function removeTmpStyle() {
        removeStyleByName(TmpStyleName);
    },
    removeTmpStyleFromHtml: function removeTmpStyleFromHtml(html) {
        return removeStyleByNameFromHtml(html, TmpStyleName);
    }

};

exports['default'] = WizStyle;
module.exports = exports['default'];

},{"./env":6}],10:[function(require,module,exports){
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

},{"../common/env":6}],11:[function(require,module,exports){
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
                    nSpan.insertBefore(tmpDom);
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
        if (!dom || !style) {
            return;
        }
        onlyWizSpan = !!onlyWizSpan;
        var k, v;
        for (k in style) {
            if (style.hasOwnProperty(k) && typeof k == 'string') {
                v = style[k];
                if (onlyWizSpan && !v) {
                    domUtils.clearStyle(dom, k);
                } else if (v.indexOf('!important') > 0) {
                    //对于 具有 !important 的样式需要特殊添加
                    domUtils.clearStyle(dom, k);
                    dom.style.cssText += k + ':' + v;
                } else {
                    dom.style[k] = v;
                }
            }
        }
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
                startDom = domUtils.splitRangeText(startDom, startOffset, endOffset);
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
                startDom = domUtils.splitRangeText(startDom, startOffset, null);
                changeStart = true;
            } else if (startDom.nodeType == 1 && startDom.childNodes.length > 0 && startOffset < startDom.childNodes.length) {
                startDom = startDom.childNodes[startOffset];
                changeStart = true;
            }
            if (endDom.nodeType == 3 && endOffset > 0 && !endDom.parentNode.getAttribute(_commonConst2['default'].ATTR.SPAN_DELETE)) {
                endDom = domUtils.splitRangeText(endDom, 0, endOffset);
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
     * 获取 DOM 的下一个叶子节点（包括不相邻的情况），到达指定的 endDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getNextNode: function getNextNode(dom, onlyElement, endDom) {
        if (dom == endDom) {
            return null;
        }
        onlyElement = !!onlyElement;
        function next(d) {
            return onlyElement ? d.nextElementSibling : d.nextSibling;
        }
        function first(d) {
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
     * 获取 DOM 的前一个叶子节点（包括不相邻的情况），到达指定的 startDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param startDom
     * @returns {*}
     */
    getPreviousNode: function getPreviousNode(dom, onlyElement, startDom) {
        if (dom == startDom) {
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
     * 获取 td 单元格 在 table 中的 行列坐标
     * @param td
     */
    getTdIndex: function getTdIndex(td) {
        return { x: td.cellIndex, y: td.parentNode.rowIndex, maxX: td.parentNode.cells.length, maxY: td.parentNode.parentNode.rows.length };
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
     * @param tagName
     * @returns {boolean}
     */
    isTag: function isTag(dom, tagName) {
        return !!dom && dom.nodeType === 1 && dom.tagName.toLowerCase() === tagName.toLowerCase();
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
     * 修改子节点的样式（style） & 属性（attribute）
     * @param dom
     * @param style
     * @param attr
     */
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
            if (p.childNodes.length > 1) {
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
            node.nodeValue = v.substring(0, start);
            p.insertBefore(t, s.nextSibling);
        }
        return s;
    }
};

exports['default'] = domUtils;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":8}],12:[function(require,module,exports){
/**
 * Dom 操作工具包（扩展功能）
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
    var key, value;

    _domBase2['default'].css(d, style, false);
    if (!!attr) {
        for (key in attr) {
            if (attr.hasOwnProperty(key) && typeof key == 'string') {
                value = attr[key];
                if (!value) {
                    d.removeAttribute(key);
                } else {
                    d.setAttribute(key, value);
                }
            }
        }
    }
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
        if (pDom === _commonEnv2['default'].doc.body || _domBase2['default'].isTag(pDom, 'td')) {
            //如果 pDom 为 body | td 且为空， 则添加 br 标签
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

},{"./../common/const":4,"./../common/env":6,"./../common/utils":8,"./domBase":11}],13:[function(require,module,exports){
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

var imgUtils = {
    getAll: function getAll(onlyLocal) {
        var images = _commonEnv2['default'].doc.images,
            img,
            imageSrcs = [],
            tmp = {},
            src;
        for (img in images) {
            if (images.hasOwnProperty(img)) {
                src = images[img].src;
                if (imgFilter(images[img], onlyLocal) && !tmp[src]) {
                    imageSrcs.push(src);
                    tmp[src] = true;
                }
            }
        }
        return imageSrcs;
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

        main = _commonEnv2['default'].doc.createElement("div");
        main.style.margin = '15px auto';
        result.push(main);

        for (i = 0, j = paths.length; i < j; i++) {
            img = _commonEnv2['default'].doc.createElement("img");
            img.src = paths[i];
            img.style.maxWidth = '100%';
            main.insertBefore(img, null);

            if (i < j - 1) {
                main.insertBefore(_commonEnv2['default'].doc.createElement("br"), null);
            }
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

},{"./../common/const":4,"./../common/env":6,"./../common/utils":8,"./../domUtils/domExtend":12,"./../rangeUtils/rangeExtend":18}],14:[function(require,module,exports){
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

    function handleTrailingParens(wholeMatch, lookbehind, protocol, link) {
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

exports["default"] = Markdown;
module.exports = exports["default"];

},{}],15:[function(require,module,exports){
'use strict';

Object.defineProperty(exports, '__esModule', {
  value: true
});
var Markdown = {};
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
    var html = '<a href="#fn:' + id + '" id="fnref:' + id + '" title="See footnote" class="footnote">' + footnoteCounter + '</a>';
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
    text += '<li id="fn:' + id + '">' + formattedfootnote + ' <a href="#fnref:' + id + '" title="Return to article" class="reversefootnote">&#8617;</a></li>\n\n';
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

exports['default'] = Markdown.Extra;
module.exports = exports['default'];

},{}],16:[function(require,module,exports){
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

var _commonDependLoader = require('./../common/dependLoader');

var _commonDependLoader2 = _interopRequireDefault(_commonDependLoader);

var _MarkdownConverter = require('./Markdown.Converter');

var _MarkdownConverter2 = _interopRequireDefault(_MarkdownConverter);

var _MarkdownExtra = require('./Markdown.Extra');

var _MarkdownExtra2 = _interopRequireDefault(_MarkdownExtra);

var isMathjax = false;
var WizToc = '#wizToc';

var defalutCB = {
    markdown: function markdown() {
        Render.Utils.appendJsCode(Render.Document, 'prettyPrint();', 'text/javascript');
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

        _commonDependLoader2['default'].loadCss(Render.Document, Render.getDependcyFiles('css', 'markdown'));

        _commonDependLoader2['default'].loadJs(Render.Document, Render.getDependcyFiles('js', 'markdown'), function () {
            Render.markdownConvert({});
            if (isMathjax) {
                Render.mathJaxRender();
            }
        });
    },
    mathJax: function mathJax() {
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
    getDependcyFiles: function getDependcyFiles(type, id) {
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
                defaultCb();
                newCb();
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
            $body[0].innerHTML = text;

            Render.cb(Render.callback.markdown);
        } catch (e) {
            Render.cb(Render.callback.markdown);
            //console.log(e);
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
        var tocHtml = [];
        $('h1,h2,h3,h4,h5,h6', Render.Document.body).each(function (index, item) {
            var id = 'wiz_toc_' + index;
            var $item = $(item);
            $item.attr('id', id);
            tocHtml.push('<a class="wiz_toc ' + item.tagName.toLowerCase() + '" href="#' + id + '">' + $item.text() + '</a>');
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

        Render.Utils.appendJsCode(Render.Document, 'MathJax = null', 'text/javascript');
        Render.Utils.appendJsCode(Render.Document, config, 'text/x-mathjax-config');
        _commonDependLoader2['default'].loadJs(Render.Document, Render.getDependcyFiles('js', 'mathJax'), _render);

        function _render() {
            var runMath = 'MathJax.Hub.Queue(' + '["Typeset", MathJax.Hub, document.body]);';
            Render.Utils.appendJsCode(Render.Document, runMath, 'text/javascript');
            Render.cb(Render.callback.mathJax);
        }
    }
};

window.MarkdownRender = MarkdownRender;

exports['default'] = MarkdownRender;
module.exports = exports['default'];

},{"./../common/const":4,"./../common/dependLoader":5,"./../common/env":6,"./../common/utils":8,"./Markdown.Converter":14,"./Markdown.Extra":15}],17:[function(require,module,exports){
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
        var sel = _commonEnv2['default'].doc.getSelection(),
            range = sel.rangeCount > 0 ? sel.getRangeAt(0) : null,
            rectList = range ? range.getClientRects() : null,
            rect = rectList && rectList.length > 0 ? rectList[0] : null,
            cH = _commonEnv2['default'].doc.documentElement.clientHeight;
        if (rect && rect.top < 0) {
            _commonEnv2['default'].doc.body.scrollTop += rect.top;
        } else if (rect && rect.top + rect.height > cH) {
            _commonEnv2['default'].doc.body.scrollTop += rect.top + rect.height - cH;
        }
        if (rect && rect.left < 0) {
            _commonEnv2['default'].doc.body.scrollLeft += rect.left;
        } else if (rect && rect.left + rect.width > cH) {
            _commonEnv2['default'].doc.body.scrollLeft += rect.left + rect.width - cH;
        }
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
        var sel = _commonEnv2['default'].doc.getSelection(),
            range = sel.getRangeAt(0),
            rangeContainer = isBackward ? range.startContainer : range.endContainer,
            rangeOffset = isBackward ? range.startOffset : range.endOffset;

        if (!sel.isCollapsed && !isBackward) {
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
     * @returns {{}}
     */
    getRangeDomList: function getRangeDomList() {
        var sel = _commonEnv2['default'].doc.getSelection(),
            range = sel.getRangeAt(0),
            startDom = range.startContainer,
            startOffset = range.startOffset,
            endDom = range.endContainer,
            endOffset = range.endOffset;
        return _domUtilsDomBase2['default'].getDomListA2B({
            startDom: startDom,
            startOffset: startOffset,
            endDom: endDom,
            endOffset: endOffset
        });
    },
    /**
     * 获取 光标范围内 Dom 共同的父节点
     * @returns {*}
     */
    getRangeParentRoot: function getRangeParentRoot() {
        var sel = _commonEnv2['default'].doc.getSelection(),
            range,
            startDom,
            endDom;
        if (sel.rangeCount === 0) {
            return null;
        }
        range = sel.getRangeAt(0);
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

        var sel = _commonEnv2['default'].doc.getSelection();
        if (sel.rangeCount === 0) {
            return;
        }
        var range = sel.getRangeAt(0);

        result.isCollapsed = sel.isCollapsed;
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
        var sel = window.getSelection();
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

},{"./../common/const":4,"./../common/env":6,"./../common/utils":8,"./../domUtils/domBase":11}],18:[function(require,module,exports){
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
    var sel = _commonEnv2['default'].doc.getSelection();
    if (sel.rangeCount == 0) {
        if (rangeBackup) {
            return true;
        }

        _commonEnv2['default'].doc.body.focus();
        sel = _commonEnv2['default'].doc.getSelection();
        if (sel.rangeCount == 0) {
            return false;
        }
    }
    rangeBackup = sel.getRangeAt(0);
    return true;
    //rangeBackup.setEnd(rangeBackup.startContainer, rangeBackup.startOffset);
};

_rangeBase2['default'].restoreCaret = function () {
    if (!rangeBackup) {
        return false;
    }
    var sel = _commonEnv2['default'].doc.getSelection();
    if (sel.rangeCount == 0) {
        _commonEnv2['default'].doc.body.focus();
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
/**
 * 在 光标（isCollapse=false）选择范围内修改所有 dom内容，设置为指定样式
 * modify the style when selection's isCollapsed == false
 * @param style
 * @param attr
 */
_rangeBase2['default'].modifyRangeStyle = function (style, attr) {
    var rangeResult, rangeList, rangeLength;
    //get the RangeList
    rangeResult = _rangeBase2['default'].getRangeDomList();
    rangeList = rangeResult.list;
    rangeLength = rangeList.length;
    if (rangeLength === 0) {
        return;
    }

    //modify style
    _domUtilsDomExtend2['default'].modifyNodesStyle(rangeList, style, attr);

    //clear redundant span & TextNode
    var ps = [],
        i,
        j,
        t,
        tempAmend;
    for (i = 0, j = rangeLength; i < j; i++) {
        t = rangeList[i].parentNode;
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
    _domUtilsDomExtend2['default'].clearChild(t, [rangeResult.startDomBak, rangeResult.endDomBak]);

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
    var sel = _commonEnv2['default'].doc.getSelection();
    if (sel.isCollapsed) {
        _rangeBase2['default'].modifyCaretStyle(style, attr);
    } else {
        _rangeBase2['default'].modifyRangeStyle(style, attr);
    }
};

exports['default'] = _rangeBase2['default'];
module.exports = exports['default'];

},{"./../common/const":4,"./../common/env":6,"./../common/utils":8,"./../domUtils/domExtend":12,"./rangeBase":17}],19:[function(require,module,exports){
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

var _commonWizStyle = require('../common/wizStyle');

var _commonWizStyle2 = _interopRequireDefault(_commonWizStyle);

var _readerEvent = require('./readerEvent');

var _readerEvent2 = _interopRequireDefault(_readerEvent);

var reader = {
    init: function init() {
        _commonWizStyle2['default'].insertTmpReaderStyle();
        _readerEvent2['default'].init();

        //禁用 输入框（主要用于 九宫格 处理）
        setDomReadOnly('input', true);
        setDomReadOnly('textarea', true);
    },
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        _commonWizStyle2['default'].insertDefaultStyle(isReplace, customCss);
    }
};

function setDomReadOnly(tag, readOnly) {
    var domList = document.getElementsByTagName(tag),
        i,
        obj;
    for (i = 0; i < domList.length; i++) {
        obj = domList[i];
        obj.readOnly = readOnly;
    }
}

exports['default'] = reader;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/utils":8,"../common/wizStyle":9,"../domUtils/domExtend":12,"./readerEvent":20}],20:[function(require,module,exports){
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

        _commonEnv2['default'].client.sendCmdToWiznote(_commonConst2['default'].EVENT.wizReaderClickImg, {
            src: target.src,
            imgList: _commonEnv2['default'].client.type.isAndroid ? _imgUtilsImgUtils2['default'].getAll(true).join(',') : null
        });
        _commonUtils2['default'].stopEvent(e);
        return false;
    }
};

exports['default'] = ReaderEvent;
module.exports = exports['default'];

},{"../common/const":4,"../common/env":6,"../common/utils":8,"../domUtils/domExtend":12,"../imgUtils/imgUtils":13}],21:[function(require,module,exports){
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
     *   clientType,  //客户端类型
     * }
     */
    init: function init(options) {
        _commonEnv2['default'].setDoc(options.document || window.document);
        (0, _commonLang.initLang)(options.lang);
        _commonEnv2['default'].client.setType(options.clientType);
        _commonEnv2['default'].dependency.files.init(options.dependencyCss, options.dependencyJs);
        _readerBase2['default'].init();
        _markdownMarkdownRender2['default'].init();

        if (!options.noAmend) {
            _amendAmendUser2['default'].initUser(options.userInfo);
            _amendAmendUser2['default'].setUsersData(options.usersData);
            WizReader.amendInfo.on();
        }
    },
    insertDefaultStyle: function insertDefaultStyle(isReplace, customCss) {
        _readerBase2['default'].insertDefaultStyle(isReplace, customCss);
    },
    markdown: function markdown() {
        _markdownMarkdownRender2['default'].markdown({
            markdown: function markdown() {
                _commonEnv2['default'].client.sendCmdToWiznote(_commonConst2['default'].EVENT.wizMarkdownRender);
            }
        });
    },
    mathJax: function mathJax() {
        _markdownMarkdownRender2['default'].mathJax();
    },
    amendInfo: {
        on: function on() {
            _amendAmendInfo2['default'].init({
                readonly: true
            }, {
                onAccept: null,
                onRefuse: null
            });
        },
        off: function off() {
            _amendAmendInfo2['default'].remove();
        }
    },
    img: {
        getAll: function getAll(onlyLocal) {
            //为了保证客户端使用方便，转换为字符串
            return _imgUtilsImgUtils2['default'].getAll(onlyLocal).join(',');
        }
    }
};

window.WizReader = WizReader;

},{"./amend/amendInfo":1,"./amend/amendUser":2,"./common/const":4,"./common/env":6,"./common/lang":7,"./common/utils":8,"./domUtils/domExtend":12,"./imgUtils/imgUtils":13,"./markdown/markdownRender":16,"./reader/base":19}]},{},[21])
