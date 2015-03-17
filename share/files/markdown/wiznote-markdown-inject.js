;(function() {
    var doc = document;
    var text; 
    var body = doc.getElementsByTagName('BODY').item(0);

    var math = [];
    var inline = "$"; 
    var SPLIT = /(\$\$?|\\(?:begin|end)\{[a-z]*\*?\}|\\[\\{}$]|[{}]|(?:\n\s*)+|@@\d+@@)/i;

    var isMathJax = WizIsMathJax();
    
    function WizIsMathJax() {
        var text = doc.body.innerText.replace(/\n/g,'\\n').replace(/\r\n?/g, "\n").replace(/```(.*\n)+?```/gm,'');
        var SPLIT = /(\$\$?)[^$\n]+\1/;
        return SPLIT.test(text);
    }

    function wizInsertElem(doc, part, elem_type, callback) {
        var oPart = doc.getElementsByTagName(part).item(0);
        var oElem = doc.createElement(elem_type);
        callback(oElem);
        oPart.insertBefore(oElem, null);
        return oElem;
    }
    function wizAppendScriptSrc(doc, part, script_type, str) {
        return wizInsertElem(doc, part, "script", function(oScript) {
            oScript.type = script_type;
            oScript.src = str;
        }
      );
    }
    function wizAppendScriptInnerHtml(doc, part, script_type, innerHtmlStr) {
        wizInsertElem(doc, part, "script", function(oScript) {
            oScript.type = script_type;
            oScript.innerHTML = innerHtmlStr;
        }
      );
    }

    function htmlUnEncode( input ) {
        return String(input)
            .replace(/\&amp;/g,'&')
            .replace(/\&gt;/g, '>')
            .replace(/\&lt;/g,'<')
            .replace(/\&quot;/g, '"')
            .replace(/\&&#39;/g, "'");
    }
    function init() {
        if (jQuery) {
            document.body.setAttribute("wiz_markdown_inited", "true");
            ParseContent(document);
        } else {
            setTimeout(init(), 100);
        }
    }


    function processMath(i, j) {
        var block = blocks.slice(i, j + 1).join("").replace(/&/g, "&amp;") // use
                                                                            // HTML
                                                                            // entity
                                                                            // for
                                                                            // &
        .replace(/</g, "&lt;")  // use HTML entity for <
        .replace(/>/g, "&gt;"); // use HTML entity for
        while (j > i) {
            blocks[j] = "";
            j--;
        }
        blocks[i] = "@@" + math.length + "@@";
        math.push(block);
        start = end = last = null;
    }

    function replaceMath(text) {
        text = text.replace(/@@(\d+)@@/g, function(match, n) {
            return math[n];
        });
        math = null;
        return text;
    }

    function parseMDContent(text) {
        var renderer = new marked.Renderer();
        renderer.code = function(code, lang) {
            var ret = '<pre class="prettyprint linenums language-' + lang + '">';
            ret+= '<code>' + code.replace(/</g, '&lt;').replace(/>/g, '&gt;') + '</code>';
            ret+= '</pre>';
            return ret;
        };
        var htmlStr = marked(text, {
            renderer: renderer,
            gfm: true,
            tables: true,
            breaks: true,
            pedantic: false,
            sanitize: false,
            smartLists: true,
            smartypants: false
        });
        return htmlStr;
    }

    function ParseMD2HTML(text) {
        var parsedHtml = parseMDContent(text);
        return parsedHtml;
    }
    function replaceCodeP2Div() {
        $('p').each(function(){
            $(this).replaceWith($('<div>' + this.innerHTML + '</div>'));
        });
    }
    function ParseContent(objHtmDoc) {
        try {
            $(objHtmDoc).find('img').each(function(index) {
                var span = $("<span></span>");
                span[0].innerText = htmlUnEncode($(this)[0].outerHTML);
                span.insertAfter($(this));
                $(this).remove();
            });
        } catch (e) {
            console.log(e);
        }
        try {
            $(objHtmDoc).find('a').each(function(index, link) {
                var href = $(link).attr('href');
                if (href.indexOf("wiz:") === 0) {
                    var text = $(link).text();
                    $(link).after('<span>[' + text + '](' + href + ')</span>');
                    $(link).remove();
                }
            });
        } catch (e) {
            console.log(e);
        }
        try {
            $(objHtmDoc).find('label.wiz-todo-label').each(function(index) {
                // 防止innerText后产生换行符
                var span = $("<span></span>");
                var parent = $(this).parent();
                span[0].innerText = htmlUnEncode(parent[0].outerHTML);
                span.insertAfter(parent);
                parent.remove();
            });
        } catch(e) {
            console.log(e);
        }
        // try {
        //     $(objHtmDoc).find('ol').each(function(index, link) {
        //         var div = $("<div></div>");
        //         div[0].innerText = htmlUnEncode($(this)[0].outerHTML);
        //         div.insertAfter($(this));
        //         $(this).remove();
        //     });
        // } catch (e) {
        //     console.log(e);
        // }
        replaceCodeP2Div();
        var text = removeMath(body.innerText);
        text = ParseMD2HTML(text);
        text = replaceMath(text);
        body.innerHTML = text;
        prettyPrint();
        if (isMathJax) {
            wizAppendScriptInnerHtml(doc, 'HEAD', "text/x-mathjax-config", "MathJax.Hub.Config({showProcessingMessages: false,tex2jax: { inlineMath: [['$','$'],['\\\\(','\\\\)']] },TeX: { equationNumbers: {autoNumber: 'AMS'} }});");
            var MathJaxScript = wizAppendScriptSrc(doc, 'HEAD', "text/javascript", "http://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_HTML");
            MathJaxScript.onload = function() {
                MathJax.Hub.Queue(
                    ["Typeset", MathJax.Hub, document.body]
                );
            };
        }
    }

    function removeMath(text) {
        start = end = last = null; // for tracking math delimiters
        math = []; // stores math strings for latter

        blocks = text.replace(/\r\n?/g, "\n").split(SPLIT);
        for ( var i = 1, m = blocks.length; i < m; i += 2) {
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
                if (block === inline || block === "$$") {
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
    init();
})();
