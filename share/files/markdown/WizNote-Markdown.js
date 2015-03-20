;(function() {     
    var WizMD_pluginPath = "${CACHE_PATH}";   
    var WizMD_cssFilePath = "${CSS_FILE_PATH}"  
    
    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    function WizMDInsertElem(doc, part, elem_type, callbackfunc) {
        var WizMD_pluginPath = WizMD_pluginPath;
        var oPart = doc.getElementsByTagName(part).item(0);
        var oElem = doc.createElement(elem_type);
        callbackfunc(oElem);
        oPart.insertBefore(oElem, null);
        return oElem;
    }
    //--------------------------------------------

    function WizMDAppendScriptSrc(doc, part, script_type, str, isServer) {
        return WizMDInsertElem(doc, part, "script", function(oScript) {
            oScript.type = script_type;
            if (isServer) {
                oScript.src = str;
            } else {
                oScript.src = ("file:///" + WizMD_pluginPath + str).replace(/\\/g, '/');
            }
        });
    }

    function WizMDAppendCssSrc(doc, str) {
        WizMDInsertElem(doc, 'HEAD', "link", function(oCss) {
            oCss.rel = "stylesheet";
            oCss.href = ("file://" + str).replace(/\\/g, '/');
        });
    }

    function WizMDAppendScriptInnerHtml(doc, part, script_type, innerHtmlStr) {
        WizMDInsertElem(doc, part, "script", function(oScript) {
            oScript.type = script_type;
            oScript.innerHTML = innerHtmlStr;
        });
    }
    /*
    *解析markdown内容
    */

    function WizInitMarkdown(doc) {
        WizMDAppendScriptInnerHtml(doc, 'HEAD', "text/javascript", "MathJax=null");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "markdown\\marked.min.js");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "google-code-prettify\\prettify.js");
        var jqueryScript = WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "markdown\\jquery.min.js");
        WizMDAppendCssSrc(doc, WizMD_cssFilePath);
        jqueryScript.onload = function() {
            WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "wiznote-markdown-inject.js");
        };
    }
    WizInitMarkdown(editor.document);
})();

