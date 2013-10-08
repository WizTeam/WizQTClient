// setup ueditor
try {
    var strSave = "Save";

    var editorOption = {
    toolbars: [],
    initialStyle: 'body{font-size:13px}',
    fullscreen: true,
    autoHeightEnabled: false,
    scaleEnabled: false,
    contextMenu: [],
    elementPathEnabled: false,
    wordCount: false,
    imagePopup: false,
    maximumWords: 100000000,
    };

    editor = new baidu.editor.ui.Editor(editorOption);
    editor.render('editorArea');

    // hide builtin toolbar, from UE dev team.
    editor.addListener('ready', function () {
        editor.ui.getDom('toolbarbox').style.display = 'none';
        editor.ui.getDom('bottombar').style.display = 'none';
        editor.ui.getDom('scalelayer').style.display = 'none';
        editor.ui.getDom('elementpath').style.display = "none";
        editor.ui.getDom('wordcount').style.display = "none";
    });

} catch (err) {
    alert(err);
}
