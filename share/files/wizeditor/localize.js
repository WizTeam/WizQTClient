var en_us_loc = {
}
var zh_cn_loc = {
    "Inserted contents": "插入了内容",
    "Deleted contents": "删除了内容",
    "Accept": "接受修订",
    "Reject": "拒绝修订",
    "Multiple changes are selected": "您选中了多处修订",
    "Someone": "有人",
    "Copy of deleted changes not allowed": "无法复制已删除的内容",
    "Cut of deleted changes not allowed": "无法剪切已删除的内容"
};
var zh_tw_loc = {
} ;
//
function WizGetBrowserLanguage() {
    var type = navigator.appName
    //
    return type == "Netscape" ? navigator.language
                              : navigator.userLanguage;
}
//
var ShareLocale = (function() {
    var lng = WizGetBrowserLanguage().toLowerCase();
    //
    var objLng;
    switch (lng) {
        case "zh-cn":
            objLng = zh_cn_loc;
            break;
        case "zh-tw":
            objLng = zh_tw_loc;
            break;
        default:
            objLng = en_us_loc;
    };
    //
    return objLng;
})();

function wizEditorTranslate (str) {
    if (!ShareLocale || !ShareLocale[str])
        return str;
    //
    return ShareLocale[str];
}