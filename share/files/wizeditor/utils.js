
var wizStyle = "wiz_style";

function WizInsertJavascript(htmlDocument, path, id, callback) {
    if (!htmlDocument)
        return;
    var node = htmlDocument.getElementById(id);
    if (node)
        return;
    //
    path += "?t=" + Date.now();
    //
    var node = htmlDocument.createElement('script');
    node.type = 'text/javascript';
    node.src = path;
    node.id = id;
    node.setAttribute(wizStyle, 'unsave');
    node.setAttribute('charset', 'utf-8');
    htmlDocument.head.appendChild(node);
    //
    if (callback) {
        node.onload = callback;
    }
}

function WizInsertCss(htmlDocument, path, id, callback) {
    if (!htmlDocument)
        return;
    var node = htmlDocument.getElementById(id);
    if (node)
        return;
    //
    path += "?t=" + Date.now();
    //
    var node = htmlDocument.createElement('link');
    node.rel = 'stylesheet';
    node.type = 'text/css';
    node.href = path;
    node.id = id;
    node.setAttribute(wizStyle, 'unsave');
    node.setAttribute('charset', 'utf-8');
    htmlDocument.head.appendChild(node);
    //
    if (callback) {
        node.onload = callback;
    }
}

function WizIsMarkdown(doc) {
    var title = doc.title;

    if (!title)
        return false;
    if (-1 != title.indexOf(".md "))
        return true;
    if (-1 != title.indexOf(".md@"))
        return true;
    if (title.match(/\.md$/i))
        return true;
    return false;
}

function WizIsMathJax(doc) {
    try {
        var title = doc.title;

        if (!title)
            return false;
        if (-1 != title.indexOf(".mj "))
            return true;
        if (-1 != title.indexOf(".mj@"))
            return true;
        if (title.match(/\.mj$/i))
            return true;
        return false;
    }
    catch (err) {
        return false;
    }
}


function WizEditorGetBrowserLanguage() {
    var type = navigator.appName
    //
    return type == "Netscape" ? navigator.language
                              : navigator.userLanguage;
}