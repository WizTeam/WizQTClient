;(function() {     var WizMD_pluginPath = "${CACHE_PATH}";     var
WizMD_inited = -1;
    
    var isMarkdown;
    var isMarkdownAndMathJax;

    function WizIsMarkdown(doc) {
        try {
            var title = doc.title;

            if (-1 != title.indexOf(".md")) {
                return true;
            }
            return false;
        }
        catch (err) {
            return false;
        }
    }
    function WizIsMarkdownAndMathJax(doc) {
        var text = doc.body.innerText.replace(/\n/g,'\\n').replace(/\r\n?/g, "\n").replace(/```(.*\n)+?```/gm,'');
        var SPLIT = /(\$\$?)[^$\n]+\1/;
        return SPLIT.test(text);
    }

    //---------------------------------------------------------------
    //eventsHtmlDocumentComplete.add(OnMarkdownHtmlDocumentComplete);


    function OnMarkdownHtmlDocumentComplete(doc) {
        isMarkdown = WizIsMarkdown(doc);
        if (isMarkdown) {
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
        WizMDAppendScriptInnerHtml(doc, 'HEAD', "text/javascript", "MathJax=null");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "markdown\\marked.min.js");
        WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "google-code-prettify\\prettify.js");
        var jqueryScript = WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "markdown\\jquery.min.js");
        WizMDAppendCssSrc(doc, "markdown\\github2.css");
        jqueryScript.onload = function() {
            var isMathJax = WizIsMarkdownAndMathJax(doc);
            if (isMathJax) {
                WizMDAppendScriptInnerHtml(doc, 'HEAD', "text/x-mathjax-config", "MathJax.Hub.Config({showProcessingMessages: false,tex2jax: { inlineMath: [['$','$'],['\\\\(','\\\\)']] },TeX: { equationNumbers: {autoNumber: 'AMS'} }});");
                //WizMDAppendScriptInnerHtml(doc, 'HEAD', "text/x-mathjax-config", "MathJax.Hub.Config({showProcessingMessages: false, tex2jax: { inlineMath: [['$','$'],['\\\\(','\\\\)']] },TeX: { equationNumbers: {autoNumber: 'AMS'} }});");
                
                var MathJaxScript = WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML", true);
                //var MathJaxScript = WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS-MML_SVG", true);
                MathJaxScript.onload = function() {
                    WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "wiznote-markdown-inject.js");
                };
            } else {
                WizMDAppendScriptSrc(doc, 'HEAD', "text/javascript", "wiznote-markdown-inject.js");
            }
        };
    }
    WizInitMarkdown(document);
})();

