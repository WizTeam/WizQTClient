
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

function WizEditorInitDeleteCommentAction() {
    try {
        var copiedComment = document.getElementById("wiz-comments-copy");
        if (copiedComment) {
            var deleteComment = document.getElementById("deleteComments");
            if (deleteComment) {
                deleteComment.addEventListener("click", function () {
                    copiedComment.parentNode.removeChild(copiedComment);
                });
            }
        }
    }
    catch (e) {
        console.log(e.toString());
    }
}


////////////////////////////////////////////////////////////////////////////////

function WizChecklistSave() {
    if (WizTodoReadChecked) {
        WizTodoReadChecked.onDocumentClose();
    }
}

function WizOnBeforeReaderBrowserClose() {
    WizChecklistSave();
}

function WizOnBeforeChangeDocument() {
    WizChecklistSave();
}

function WizOnBeforeReaderModeToEditor() {
    WizChecklistSave();
}

function WizOnBeforeEditorModeToReader() {
    try {
        if (WizTodoReadChecked) {
            WizTodoReadChecked.clear();
            WizTodoReadChecked.init("mac");
        }
    }
    catch (e) {
    }
}

function WizTodoReaderInit() {
    if (WizTodoReadChecked) {
        WizTodoReadChecked.clear();
        WizTodoReadChecked.init('mac');
    }
}

/*
  function for C++ execute
*/
function WizEditorInit(basePath, browserLang, userGUID, userAlias, ignoreTable, noteType) {
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
            dependencyCss: {
                fonts: basePath + 'dependency/fonts.css',
                github2: basePath + 'dependency/github2.css',
                wizToc: basePath + 'dependency/wizToc.css',
            },
            dependencyJs: {
                jquery: basePath + 'dependency/jquery-1.11.3.js',
                prettify: basePath + 'dependency/prettify.js',
                raphael: basePath + 'dependency/raphael.js',
                underscore: basePath + 'dependency/underscore.js',
                flowchart: basePath + 'dependency/flowchart.js',
                sequence: basePath + 'dependency/sequence-diagram.js'
            }
        }
        //
        WizEditor.init(editorOptions);
        //
        WizEditorInitDeleteCommentAction();
        //
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
