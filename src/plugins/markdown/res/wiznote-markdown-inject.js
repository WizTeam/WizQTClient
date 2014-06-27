;(function() {
    var IsMathJax = WizIsMathJax();
    var inline = "$"; 

    var SPLIT = /(\$\$?|\\(?:begin|end)\{[a-z]*\*?\}|\\[\\{}$]|[{}]|(?:\n\s*)+|@@\d+@@)/i;

    var text;
    var math = [];
    var body = document.getElementsByTagName('BODY').item(0);

    function WizIsMathJax() {
        return typeof MathJax !== 'undefined';
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
            g_markdownInited = true;
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
                var div = $("<span></span>");
                var parent = $(this).parent();
                div[0].innerText = htmlUnEncode(parent[0].outerHTML);
                div.insertAfter(parent);
                parent.remove();
            });
        } catch(e) {
            console.log(e);
        }
        replaceCodeP2Div();
        var text = removeMath(body.innerText);
        text = ParseMD2HTML(text);
        text = replaceMath(text);
        body.innerHTML = text;
        prettyPrint();
        if (IsMathJax) {
            MathJax.Hub.Queue(
                ["Typeset", MathJax.Hub, document.body],
                ["resetEquationNumbers", MathJax.InputJax.TeX]
            );
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
    init(IsMathJax);
})();
