(function(){
    var added = false;
    var arrLinks = document.anchors;
    var arrayBookmarkAddtime = [];
    var arrayBookmarkName = [];
    var jsonelem = [];
    //
    for (var i = 0; i < arrLinks.length; i++) {
        var elem = arrLinks[i];
        var name = elem.name;
        if (name == null || name == "")
            continue;
        //
        var text = elem.innerText;
        if (text.replace(/(^\s*)|(\s*$)/g, "").length < 1)
            continue;
        //
        var addtime = elem.getAttribute("wiznote_add_time");
        //
        var isadded = false;
        for (var j = arrayBookmarkAddtime.length - 1; j >= 0; j--) {
            if (addtime == arrayBookmarkAddtime[j]) {
                isadded = true;
                break;
            }
        }
        if (isadded == true)
            continue;
        //
        arrayBookmarkAddtime.push(addtime);
        //
        arrayBookmarkName.push(name);
    }
    return arrayBookmarkName;
}());