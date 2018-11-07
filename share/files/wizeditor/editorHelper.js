
function WizStartEditorAmend() {
    try {
        if (!WizEditor) {
            alert("WizEditor is null!");
            return;
        }
        //
        WizEditor.amend.on();
    }
    catch (e) {
        console.log(e.toString());
    }
}

function WizStopEditorAmend() {
    try {
        if (!WizEditor) {
            alert("WizEditor is null!");
            return;
        }
        //
        WizEditor.amend.off();
    }
    catch (e) {
        console.log(e.toString());
    }
}

////////////////////////////////////////////////////////////////////////////////

/*
  function for C++ execute
*/
function WizEditorInit(basePath, browserLang, userGUID, userAlias, ignoreTable, noteType, enableNightMode) {
    try {
        if (!WizEditor) {
            alert("WizEditor is null!");
            return;
        }
        var user = {
            user_guid: userGUID,
            user_name: userAlias
        };
        //
        var editorOptions = {
            document: document,
            lang: browserLang,
            noteType: noteType,
            userInfo: user,
            clientType: "mac",
            ignoreTable : ignoreTable,
            dependencyUrl: basePath + 'dependency',
            nightMode: {
                enable: enableNightMode,
                color: '#a6a6a6',
                bgColor: '#272727',
            }
        }
        //
        WizEditor.init(editorOptions);
        WizEditor.link.on();
        WizEditor.addListener(WizEditor.ListenerType.SelectionChange, WizOnSelectionChange)
        return true;
    }
    catch (e) {
        console.log(e.toString());
        return false;
    }
}

function WizOnSelectionChange(style)
{
    try {
        WizQtEditor.OnSelectionChange(JSON.stringify(style));
    } catch (e) {

    }
}


function WizAddCssForCode(cssFile) {
    console.log("WizAddCssForCode called , css file " + cssFile);
    var doc = document;
    if (!doc)
        return;
    //
    var oldLink = doc.getElementById('wiz_code_highlight_link');
    if (oldLink) {
        console.log("old css link find, try to remove");
        oldLink.parentNode.removeChild(oldLink);
    }
    //
    var link = doc.createElement('link');
    if (!link)
        return;
    try {
        link.type = 'text/css';
        link.rel = 'stylesheet';
        link.id = 'wiz_code_highlight_link';
        link.href = cssFile;
        //
        if (!doc.head)
        {
            doc.insertBefore(doc.createElement('head'), doc.body);
        }
        doc.head.appendChild(link);
    }
    catch(e) {

    }
}
