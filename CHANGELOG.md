##2.1.16(2015-3-12)

Bugfixes:

- [Mac]Fix the delay problem of text input.
- [Linux]Fix the crash problem when drag document to folder.
- Fixed the initialization delay problem of insert code panel.

Features:

- Add the funtion of importing files(not supported documents with images for now).
- Added support table cell alignment.
- Modify the usage of editor color selection buttons.
- Add function to retrieve the deleted notes.
- Modify the limitation prompt of group.
- Optimized loading speed of notes list.
- Modify the Markdown references css file storage location, you can manually modify the style.
- You can edit note information now.


##2.1.15(2015-1-16)

Bugfixes:

- Fix the problem that modify the sync interval had no effect.
- Fix the width problem when pasting as plain text.

Features:

- Modify the way of checking the status of the group notes, no longer allowed to edit notes that editing by other users!
- Add the function of manually sorting folders. You can choose to manually sort the folder or sorted in a systematic manner in the preferences.
- Quit program if system tray icon is unvisible when users close the mainwindow.
- Change the icon for system tray icon.
- Add the function of sending notes by email.
- Added support for searching encrypted notes, need to enter the password in the preferences.
- Add the function of setting the editor background color in preferences.
- Add the function of editing the html source code of notes.
- All unread messages can be marked as read.
- Update note version when upload data to server.

##2.1.14(2014-11-9)

Bugfixes:

- Fixed shortcuts scope problem of editor, no longer affect the operation of other widgets.
- Normal token failure no longer ask for password.
- [Mac]When the program at full-screen model, insert code widget no longer show in full-screen model.

Features:

- [Mac]Adaptation for Retina screen.
- [Linux]Add option to use system window style.
- [Linux]Add function of screenshot.
- Notes browsing history.
- Increase the common notes list, you can set notes shortcuts now.
- Notes List sorted by access time.
- Set notes always on the top.
- Added support for multi-keyword search, use blank space to split keywords.
- Add creator information of notes to the document list.
- Too much registration will called for a verification code.

##2.1.13(2014-9-30)

Bugfixes:

- [Linux]Only start one instance of application, active the running application when users try to run the exec again.
- [Mac]Fix the dragging to add attachments problem on Yosemite system.
- When jumping from notes link, locate note in document list.

Features:

- Add the feature of notes encryption.
- Add the feature of receiving images from phone.
- Add the feature of creating notes through template.
- Add the feature of printing.
- Add the feature of exporting to Html documents.
- Optimize the mode of caching network images.
- Add support to Socks5 type proxy server.
- Add the feature of pasting as plain text.


##2.1.12(2014-9-8)

Bugfixes:

  - Repair format lost problem when paste multilevel list .
  - After syncing data, refresh the list of attachments.

Features:

  - Add the function of insert code.
  - Add the right-click menu sent to function for WizNote.
  - Biz group would display the number of unread notes.
  - Add the function of locating note position.
  - Drag and drop the image file to the editor, can add files as attachments to the notes.
  - Drag the notes to the editor, you can add notes link.
  - Add function of converting html to plain text.
  - Adjust the editor shortcuts.



##2.1.11(2014-7-28)

Bugfixes:

  - Fix problem that the markdown rendering fonts do not match with the custom font. 
  - Fix problem that the team list do not refresh after create new team. 

Features:

  - Add the function of view the notes in a separate window (double click on the notes in notes list view). 
  - Change right-click menu mode of notes list, right-click on no longer change selected notes. 
  - Add find and replace function to find and replace text in the single notes. 
  - Add the option to hide the tray icon. 
  - Add some tips of editor buttons.


##2.1.10(2014-7-4)

Bugfixes:

  - [Mac]Fix problem that CapsLock key do not work. 
  - [Mac]Fix format leakage problems when paste web html or code. 
  - [Mac]Enterprise group user avatar cache update in time. 
  - [Mac]Fix problem that toolbar shakes when resume mainwindow from minimum. 
  - Optimize the Markdown support for MathJax, accelerates the rendering speed. 
  - Fix problem that Markdonw plugin was not compatible with Checklist. 
  - Fix category folder state save problem. 

Features:

  - Document List could use ascending and descending sort order.


##2.1.9(2014-6-23)

Bugfixes:

  - Update note when the attachment was modified. 
  - Refresh note list after change note by note link.

Features:

  - Add icon in the system tray, click the exit button in the main window would no longer exit the program.
  - Optimize the online images saved locally.
  - Upgrade Markdown engine, add support for MathJax. 


##2.1.8(2014-6-6)

Bugfixes:

  - Solve problem that the pictures cannot copy and save in the notes. 
  - Repair problems such as unable to open the payment link.

Features:

  - Add network proxy function.
  - Add edit status of group notes, when someone else is editing the note will prompt tips message.
  - New login interface. 
  - [Linux]Change window style.
  - Highlight search keywords. 
  - Add the download progress indication of notes and attachments.
  - Add the copy notes link function.


##2.1.7(2014-5-16)

Bugfixes:

  - Fix bug about undo oprt

Features:

  - Change binary filename to WizNote


##2.1.6(2014-5-8)

Bugfixes:

  - Fix css update method when loading document.
  - Update Editor Engine to fix undo problem.
  - Update MarkDown Engine to fix extra line problem.
  - Prompt info if the note connected to message has been deleted.

Features:

  - Add icons in menubar.
