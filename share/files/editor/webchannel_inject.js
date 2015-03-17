var serverUrl = '${SERVER_URL}';
var webchannelFile = '${WEBCHANNEL_FILE}';

function init(scriptText) {
	console.log("init function called , js file : " + webchannelFile);
	var channelScript = document.createElement('script');
	channelScript.src = scriptText;
	channelScript.onload = function() { 
		alert(1); 
		console.log("channelScript loaded");
		initializeJSObject(serverUrl);
	};
	document.head.appendChild(channelScript);
	//
    $(document.body).delegate('a', 'click', function(e) {    	
        var a = $(e.currentTarget),
        href = a.attr('href') || '';
        alert("body link clicked : " + href);
        console.log("eidtor link clicked : " + href);
        if (href && href.indexOf('wiz:') === 0) {
            //笔记内链
        
        } else if (href && (href.indexOf('mailto:') === 0 || href.indexOf('ftp:') === 0 || href.indexOf('http:') === 0 || href.indexOf('https:') === 0)) {
            //弹出浏览器窗口显示....

        }

    e.stopPropagation();
    e.preventDefault();
    });
}

function output(message)
{
    console.log(message);
}

function initializeJSObject(strUrl)
{
	output("Connecting to WebSocket server at " + strUrl + ".");
    var socket = new WebSocket(strUrl);
    socket.onclose = function()
    {
        console.error("web channel closed");
    };
    socket.onerror = function(error)
    {
        console.error("web channel error: " + error);
    };
    socket.onopen = function()
    {
        output("WebSocket connected, setting up QWebChannel.");
        new QWebChannel(socket, function(channel) {
            // make dialog object accessible globally
            // window.jsObject = channel.objects.jsObject;
            // jsObject.speakHello();
            window.WizEditor = channel.objects.WizEditor;
            initialUEditor(WizEditor);
            output("Connected to WebChannel, ready to send/receive messages!");
        });
    }
}

init('${SCRIPTTEXT}');