new QWebChannel(qt.webChannelTransport, function (channel) {
    // make dialog object accessible globally
    var objectNames = [__objectNames__];
    for (var i = 0; i < objectNames.length; i++) {
        var key = objectNames[i];
        window[key] = channel.objects[key];
    }
    //
    if (typeof initForWebEngine !== 'undefined') {
        try {
            initForWebEngine();
        } catch (err) {
            console.error(err);
        }
    }
});



