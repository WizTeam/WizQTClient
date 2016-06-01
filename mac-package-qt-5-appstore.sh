# 挂载的volumn的名称
volumn_name='wiznote-disk'
# 挂载点，一般不用修改，只配置volumn_name即可
volumn_path="/Volumes/$volumn_name"

# 以下参数只是用于自定义脚本的行为
package_home="./macos-package"
package_output_path="$HOME"

REV=`git rev-list HEAD | wc -l | awk '{print $1}'`
echo "build version : " $REV

# compile
mkdir ../WizQTClient-Release-QT5
rm -rf ../WizQTClient-Release-QT5/* && \
cd ../WizQTClient-Release-QT5 && \
cmake -DWIZNOTE_USE_QT5=YES -DCMAKE_BUILD_TYPE=Release -UPDATE_TRANSLATIONS=YES -DAPPSTORE_BUILD=YES -DCMAKE_PREFIX_PATH=/Users/weishijun/Qt5.5.1/5.5/clang_64/lib/cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk ../WizQTClient && \
make -j5

MYAPP="WizNote"
DEST="$MYAPP.app" # Our final App directory
BUILDDIR=$(pwd);
QTDIR="/Users/weishijun/Qt5.5.1/5.5/clang_64"
QTLIBS="QtCore QtNetwork QtSql QtGui QtOpenGL QtWidgets QtWebEngine QtWebEngineWidgets QtWebChannel QtWebSockets \
  QtPrintSupport QtXml QtPositioning QtSensors QtConcurrent QtMacExtras QtMultimediaWidgets QtMultimedia" # QtQml QtQuick QtSvg QtScript
PLUGINS="sqldrivers imageformats  platforms printsupport \
  position iconengines" # playlistformats sensors sensorgestures bearer audio
 
# make clean & create pathes 
mkdir -p $DEST/Contents/Frameworks $DEST/Contents/SharedSupport
 
# copy Qt libs, plug-ins
for L in $QTLIBS ; do
  cp -R -p $QTDIR/lib/$L.framework $MYAPP.app/Contents/Frameworks
  # mkdir $MYAPP.app/Contents/Frameworks/$L.framework/Versions/5/Resources
  # cp -R -p $MYAPP.app/Contents/Frameworks/$L.framework/Contents/Info.plist \
  #   $MYAPP.app/Contents/Frameworks/$L.framework/Versions/5/Resources
  # rm -R -f $MYAPP.app/Contents/Frameworks/$L.framework/Contents
  # remove all unnecessary header files:
  rm -f $MYAPP.app/Contents/Frameworks/$L.framework/Headers
  rm -R -f $MYAPP.app/Contents/Frameworks/$L.framework/Versions/5/Headers
  #rm -R -f $MYAPP.app/Contents/Frameworks/$L.framework/Versions/Current
  # rm $MYAPP.app/Contents/Frameworks/$L.framework/Versions/5/${L}_debug
  # rm $MYAPP.app/Contents/Frameworks/$L.framework/${L}
  rm $MYAPP.app/Contents/Frameworks/$L.framework/${L}.prl
  # rm $MYAPP.app/Contents/Frameworks/$L.framework/${L}_debug
  # rm $MYAPP.app/Contents/Frameworks/$L.framework/${L}_debug.prl
  # cd $MYAPP.app/Contents/Frameworks/$L.framework/Versions
  #ln -s 5/ Current
  # cd ..
  # cd $MYAPP.app/Contents/Frameworks/$L.framework
  # rm -f Resources
  # ln -s Versions/Current/$L $L
  # ln -s Versions/Current/Resources/ Resources
  # cd $BUILDDIR
  #rm $MYAPP.app/Contents/Frameworks/$L.framework/Versions/Current/.
done

##############################################################
#copy qt plugins
mkdir $MYAPP.app/Contents/PlugIns
for P in $PLUGINS ; do
  mkdir $MYAPP.app/Contents/PlugIns/$P
  cp -R -p $QTDIR/plugins/$P/*.dylib $MYAPP.app/Contents/PlugIns/$P/
  rm $MYAPP.app/Contents/PlugIns/$P/*_debug.dylib
done
#remove unused plugins
rm -R -f $MYAPP.app/Contents/PlugIns/platforms/libqminimal.dylib
rm -R -f $MYAPP.app/Contents/PlugIns/platforms/libqoffscreen.dylib
rm -R -f $MYAPP.app/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib

# copy own application libs if necessary to /Contents/PlugIns/myapp/
DISTPLUGINS=`cd $MYAPP.app/Contents/PlugIns; ls -1 */*.dylib` # extract all our *.dylib libs
 
for I in $QTLIBS ; do 
  install_name_tool -id "@executable_path/../Frameworks/$I.framework/Versions/5/$I"\
   "$MYAPP.app/Contents/Frameworks/$I.framework/Versions/5/$I"
  install_name_tool -change $I.framework/Versions/5/$I\
    @executable_path/../Frameworks/$I.framework/Versions/5/$I\
    $MYAPP.app/Contents/MacOS/$MYAPP # change references to Qt frameworks
  for L in $QTLIBS ; do # change all lib references in all Qt frameworks
    if [ $L = $I ] ; then continue; fi 
    install_name_tool -change $I.framework/Versions/5/$I\
      @executable_path/../Frameworks/$I.framework/Versions/5/$I\
      $MYAPP.app/Contents/Frameworks/$L.framework/Versions/5/$L
  done
done
 
for P in $DISTPLUGINS ; do # change ID for all *.dylib libs

  # # remove debug file
  # result=$(echo $P | grep "_debug")
  # if [ "$result" != "" ];
  # then
  #   rm $MYAPP.app/Contents/PlugIns/$P;
  #   continue;
  # fi

  install_name_tool -id "@executable_path/../PlugIns/$I" "$MYAPP.app/Contents/PlugIns/$P"
  for L in $QTLIBS ; do # change any reference to Qt in our *.dylib libs
    install_name_tool -change $L.framework/Versions/5/$L\
      @executable_path/../Frameworks/$L.framework/Versions/5/$L\
      $MYAPP.app/Contents/PlugIns/$P
  done
done
 
# we do the same for additional own libs in /Contents/PlugIns/myapp

DISTPLUGINS2=`cd $MYAPP.app/Contents/PlugIns; ls *.dylib`

for P in $DISTPLUGINS2 ; do # change ID for all *.dylib libs
  install_name_tool -id "@executable_path/../PlugIns/$I" "$MYAPP.app/Contents/PlugIns/$P"
  for L in $QTLIBS ; do # change any reference to Qt in our *.dylib libs
    install_name_tool -change $L.framework/Versions/5/$L\
      @executable_path/../Frameworks/$L.framework/Versions/5/$L\
      $MYAPP.app/Contents/PlugIns/$P
  done

  for M in $DISTPLUGINS2 ; do
    install_name_tool -change $BUILDDIR/WizNote.app/Contents/PlugIns/$M\
      @executable_path/../PlugIns/$M\
      $MYAPP.app/Contents/PlugIns/$P
  done

done

install_name_tool -change $QTDIR/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore WizNote.app/Contents/Frameworks/QtXml.framework/Versions/5/QtXml





##############
#如果需要添加Safari插件，则上面的编译部分和下面的打包部分需要分开执行。把Safari插件拷贝到WizNote.app/Contents/PlugIns/之后在进行签名打包
##############




cp -R -p ../WizQTClient/build/osx/WizNote-Entitlements.plist WizNote-Entitlements.plist

APPLCERT="3rd Party Mac Developer Application: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)"
INSTCERT="3rd Party Mac Developer Installer: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)"


#######对Safari剪辑器插件进行签名
#cp -R -p ../MacShareExtension.entitlements MacShareExtension.entitlements
#codesign --force --verify --deep --verbose=2 --sign "$APPLCERT" --entitlements MacShareExtension.entitlements\
#    $MYAPP.app/Contents/PlugIns/MacShareExtension.appex
#######

 
for I in $QTLIBS ; do # signing the Qt frameworks
  codesign --force --verify --deep --verbose --sign "$APPLCERT" \
    $MYAPP.app/Contents/Frameworks/$I.framework/Versions/5
done
for I in $DISTPLUGINS ; do # signing all *.dylib libs
  echo "code sign : "   $I;
  codesign --force --verify --deep --verbose --sign "$APPLCERT" \
    $MYAPP.app/Contents/PlugIns/$I
done

for I in $DISTPLUGINS2 ; do # signing all *.dylib libs
  echo "code sign : "   $I;
  codesign --force --verify --deep --verbose --sign "$APPLCERT" \
    $MYAPP.app/Contents/PlugIns/$I
done

plutil -replace CFBundleVersion -string $REV $MYAPP.app/Contents/Info.plist


codesign --verbose=2 --sign "$APPLCERT" --entitlements \
  WizNote-Entitlements.plist  "$MYAPP.app"

productbuild --component "$MYAPP.app" /Applications \
  --sign "$INSTCERT" "$MYAPP.pkg"
