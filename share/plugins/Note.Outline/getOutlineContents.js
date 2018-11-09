(function () {
    var arr = document.all;
    var jsonarray = [];
    //
    for (var i = 0; i < arr.length; i++) {
        var elem = arr[i];
        //
        var tagName = elem.tagName.toLowerCase();
        //
        if (tagName != "h1"
            && tagName != "h2"
            && tagName != "h3"
            && tagName != "h4"
            && tagName != "h5"
            && tagName != "h6")
            continue;
        //
        var nodes = elem.getElementsByTagName("DIV");
        if (0 < nodes.length)
            continue;
        //
        var elemtext = elem.textContent;
        if (elemtext.replace(/(^\s*)|(\s*$)h1<br>/g, "") == 0)
            continue;
        //
        var jsonelem = {
            "tagName": tagName,
            "offsetTop": elem.offsetTop,
            "text": elemtext
        };
        jsonarray.push(jsonelem);
    }
   var json = JSON.stringify(jsonarray);
   return json;
}());