REV=`git rev-list HEAD | wc -l | awk '{print $1}'`
echo "build version : " $REV

QTDIR="/usr/local/Qt-5.7.0"

# compile
mkdir ../WizQTClient-Release-QT5
rm -rf ../WizQTClient-Release-QT5/* && \
cd ../WizQTClient-Release-QT5 && \
cmake -DCMAKE_BUILD_TYPE=Release -UPDATE_TRANSLATIONS=YES -DAPPSTORE_BUILD=YES -DCMAKE_PREFIX_PATH=$QTDIR/lib/cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk ../WizQTClient && \
make -j5



##############################################################
#defines


MYAPP="WizNote"
DEST="$MYAPP.app" # Our final App directory
BUILDDIR=$(pwd);

#qt frameworks
QTLIBS="QtCore QtNetwork QtSql QtGui QtOpenGL QtWidgets QtWebEngine QtWebEngineCore QtWebEngineWidgets QtWebChannel QtWebSockets QtDBus \
  QtPrintSupport QtXml QtPositioning QtSensors QtConcurrent QtMacExtras QtMultimediaWidgets QtMultimedia QtQml QtQuick" #QtSvg QtScript
#qt plugins
PLUGINS="sqldrivers imageformats  platforms printsupport \
  position iconengines bearer" # playlistformats sensors sensorgestures audio
 
 
##############################################################
#macdeployqt
#使用qt自带的发布工具，否则有问题


echo "replace version"
plutil -replace CFBundleVersion -string $REV $MYAPP.app/Contents/Info.plist

echo "call macdeployqt"
$QTDIR/bin/macdeployqt $DEST -appstore-compliant

echo "replace QtWebEngineProcess"
#plutil -insert CFBundleVersion -string $REV "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app/Contents/Info.plist"
#plutil -replace CFBundleIdentifier -string "cn.wiz.wiznoteformac.QtWebEngineProcess" "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app/Contents/Info.plist"

##############################################################
#sign
#签名仍然有问题，无法正常使用webengine，原因未知。

APPLCERT="3rd Party Mac Developer Application: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)"

echo "sign binary"
cp -R -p ../WizQTClient/build/osx/WizNote-Entitlements.plist WizNote-Entitlements.plist
cp -R -p ../WizQTClient/build/osx/WebEngineProcess.plist WebEngineProcess.plist


#codesign --deep --verbose --force --verify --sign "$APPLCERT" -i "cn.wiz.wiznoteformac.QtWebEngineProcess" "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"
#codesign --deep --verbose --force --verify --sign "$APPLCERT" "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"
#codesign --deep --verbose --force --verify --sign "$APPLCERT" "$MYAPP.app/Contents/Frameworks/"
#codesign --deep --verbose --force --verify --sign "$APPLCERT" "$MYAPP.app/Contents/PlugIns/"

echo "sign QtWebEngineProcess with entitlements"
#codesign --sign "$APPLCERT" -i "cn.wiz.wiznoteformac.QtWebEngineProcess" --entitlements WizNote-Entitlements.plist  "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"
#codesign --sign "$APPLCERT" --entitlements WizNote-Entitlements.plist  "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"

#codesign --force -i "org.qt-project.Qt.QtWebEngineProcess" --sign "$APPLCERT" --entitlements WebEngineProcess.plist  "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"

echo "sign frameworks"
for I in $QTLIBS ; do # signing the Qt frameworks
    echo "code sign framework: "  $I;
    codesign --verify --verbose --sign "$APPLCERT" $MYAPP.app/Contents/Frameworks/$I.framework
done

#codesign --force -i "org.qt-project.Qt.QtWebEngineProcess" --sign "$APPLCERT" --entitlements WebEngineProcess.plist  "$MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/5/Helpers/QtWebEngineProcess.app"

echo "sign plugins"
DISTPLUGINS=`cd $MYAPP.app/Contents/PlugIns; ls -1 */*.dylib` # extract all our *.dylib libs
for I in $DISTPLUGINS ; do # signing all *.dylib libs
    echo "code sign plugin: "  $I;
    codesign --verify --verbose --sign "$APPLCERT" $MYAPP.app/Contents/PlugIns/$I
done

#codesign --force -s "$APPLCERT" -i "org.qt-project.QtWebEngine" $MYAPP.app/Contents/Frameworks/QtWebEngine.framework/
codesign --force -s "$APPLCERT" -i "org.qt-project.Qt.QtWebEngineProcess" --entitlements WebEngineProcess.plist $MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/Versions/Current/Helpers/QtWebEngineProcess.app
codesign --force -s "$APPLCERT" -i "org.qt-project.Qt.QtWebEngineCore" $MYAPP.app/Contents/Frameworks/QtWebEngineCore.framework/
#codesign --force -s "$APPLCERT" -i "org.qt-project.QtWebEngineWidgets" $MYAPP.app/Contents/Frameworks/QtWebEngineWidgets.framework/

echo "sign WizNote.app with entitlements"
codesign --verbose=3 --sign "$APPLCERT" --entitlements WizNote-Entitlements.plist  "$MYAPP.app"

##############################################################
#make pkg

echo "build pkg"
INSTCERT="3rd Party Mac Developer Installer: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)"
productbuild --component "$MYAPP.app" /Applications --sign "$INSTCERT" "$MYAPP.pkg"
