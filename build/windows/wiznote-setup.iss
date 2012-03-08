[Setup]
AppId=WizNote
OutputDir=e:\
DisableStartupPrompt=yes
AppName={cm:MyAppName}
AppVerName={cm:MyAppVerName}
DefaultGroupName={cm:MyGroupName}
DefaultDirName={pf}\WizNote
OutputBaseFilename=wiznote-1.0-beta-setup-x86
ShowLanguageDialog=yes
MinVersion=0, 5.0
SolidCompression=yes
AppCopyright=Copyright (C) 2001-2012 WoZhi Co., Ltd
VersionInfoProductName=WizNote 1.0
VersionInfoCompany=wiz.cn
VersionInfoDescription=WizNote 1.0
VersionInfoVersion=1.0
WizardImageFile=setup\WizModernImage.bmp
WizardSmallImageFile=setup\WizModernSmallImage.bmp
WizardImageStretch=yes
SetupIconFile=setup\setup.ico
DisableProgramGroupPage=yes
;BackColor=clWhite
;BackColor2=clWhite

[Languages]
Name: en; MessagesFile: "compiler:Default.isl";
Name: cn; MessagesFile: "isl\ChineseSimp.isl";
Name: tw; MessagesFile: "isl\ChineseTrad.isl";

[CustomMessages]
en.MyAppName=WizNote
en.MyAppVerName=WizNote 1.0
en.WizNote=WizNote
en.MyGroupName=WizNote
en.LuanchWizNote=Launch WizNote

cn.MyAppName=为知笔记
cn.MyAppVerName=为知笔记 1.0
cn.WizNote=为知笔记
cn.MyGroupName=为知笔记
cn.LuanchWizNote=运行 为知笔记

tw.MyAppName=爲知筆記
tw.MyAppVerName=爲知筆記 1.0
tw.WizNote=爲知筆記
tw.MyGroupName=爲知筆記
tw.LuanchWizNote=啟動 爲知筆記


[Icons]
Name: "{group}\{cm:WizNote}";                              FileName: "{app}\WizNote.exe";
Name: "{userdesktop}\{cm:WizNote}";                        FileName: "{app}\WizNote.exe";

[Files]
Source: "..\..\..\build\wiznote\release\WizNote.exe";      DestDir: "{app}";                       Flags: ignoreversion
Source: "..\..\share\wiznote\*.*";                         DestDir: "{app}";                       Flags: ignoreversion recursesubdirs
Source: "files\*.*";                                       DestDir: "{app}";                       Flags: ignoreversion recursesubdirs

[Run]
Filename: "{app}\WizNote.exe";                             Description: "{cm:LuanchWizNote}";      Flags: postinstall nowait skipifsilent

