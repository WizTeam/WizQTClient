
(function() {

    var baseUrl = "ws://localhost:__port__";
    var socket = new WebSocket(baseUrl);
    //
    window.wizWebChannelSocket = socket;

    socket.onclose = function()
    {
        console.log("web channel closed");
    };
    socket.onerror = function(error)
    {
        console.error("web channel error: " + error);
    };
    socket.onopen = function()
    {
        console.log("web channel opened");
        //
        new QWebChannel(socket, function(channel) {
            // make dialog object accessible globally
            var objectNames = [__objectNames__];
            for (var i = 0; i < objectNames.length; i++) {
                var key = objectNames[i];
                window[key] = channel.objects[key];
            }
            //
            if (initForWebEngine)
            {
                initForWebEngine();
            }
        });
    }
})();



