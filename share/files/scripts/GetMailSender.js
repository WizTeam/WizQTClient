(function(){
    var items = document.getElementsByTagName('input');
    //
    for (var i = items.length - 1; i >= 0; i--) {
        if (items[i].type == "hidden" && RegExp("\\bwiz_mail_reply_to\\b").test(items[i].value)){
            var str = items[i].value;
            return str.replace("wiz_mail_reply_to/", "");
        }
    }
    //
    for (var i = items.length - 1; i >= 0; i--) {
        if (items[i].type == "hidden" && RegExp("\\bwiz_mail_from\\b").test(items[i].value)){
            var str = items[i].value;
            return str.replace("wiz_mail_from/", "");
        }
    }
})();


