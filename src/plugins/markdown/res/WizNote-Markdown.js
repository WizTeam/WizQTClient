;(function() {
    var WizMD_pluginPath = "${CACHE_PATH}";
    var WizMD_inited = -1;

    function WizIsMarkdown(doc) {
        try {
            var title = doc.title;

            if (-1 != title.indexOf(".md ")) {
                return true;
            }
            if (title.lastIndexOf(".md") == -1) {
                return false;
            }
            if (title.lastIndexOf(".md") == title.length - 3) {
                return true;
            }
            return false;
        }
        catch (err) {
            return false;
        }
    }

    //---------------------------------------------------------------
    //eventsHtmlDocumentComplete.add(OnMarkdownHtmlDocumentComplete);


    function OnMarkdownHtmlDocumentComplete(doc) {
        if (WizIsMarkdown(doc)) {
            WizInitMarkdown(doc);
            WizMD_inited = 1;
        }
        else {
            WizMD_inited = 0;
        }
    }
    //-----------------------------------------------------------------
    //-----------------------------------------------------------------
    function WizMDInsertElem(doc, part, elem_type, callbackfunc) {
        var WizMD_pluginPath = WizMD_pluginPath;
        var oPart = doc.getElementsByTagName(part).item(0);
        var oElem = doc.createElement(elem_type);
        callbackfunc(oElem);
        //oHead.appendChild(oElem); 
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
        }
      );
    }

    function WizMDAppendCssSrc(doc, str) {
        WizMDInsertElem(doc, 'HEAD', "link", function(oCss) {
            oCss.rel = "stylesheet";
            oCss.href = ("file:///" + WizMD_pluginPath + str).replace(/\\/g, '/');
        }
      );
    }

    function WizMDAppendScriptInnerHtml(doc, part, script_type, innerHtmlStr) {
        WizMDInsertElem(doc, part, "script", function(oScript) {
            oScript.type = script_type;
            oScript.innerHTML = innerHtmlStr;
        }
      );
    }
    /*
    *解析markdown内容
    */

    function WizInitMarkdown(doc) {
        WizMDAppendScriptInnerHtml(doc, 'HEAD', "text/x-mathjax-config", "MathJax.Hub.Config({showProcessingMessages: false,tex2jax: { inlineMath: [['$','$'],['\\\\(','\\\\)']] },TeX: { equationNumbers: {autoNumber: 'AMS'} }});");
        var mathjaxScript = WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML", true);

        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "marked.min.js");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "highlight.pack.js");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "jquery.min.js");
        WizMDAppendCssSrc(doc, "GitHub2.css");
        mathjaxScript.onload = function() {
            WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "wiznote-markdown-inject.js");
        };
    }
    WizInitMarkdown(document);
})();

