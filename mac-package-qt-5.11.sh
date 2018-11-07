# 挂载的volumn的名称
volumn_name='wiznote-disk'
# 挂载点，一般不用修改，只配置volumn_name即可
volumn_path="/Volumes/$volumn_name"

REV=`git rev-list HEAD | wc -l | awk '{print $1}'`
echo "build version : " $REV

# 以下参数只是用于自定义脚本的行为
package_home="./macos-package"
package_output_path="$HOME"

QTDIR="/Users/weishijun/Qt5.11.2/5.11.2/clang_64"

mkdir ../WizQTClient-Release-QT5
rm -rf ../WizQTClient-Release-QT5/* && \
cd ../WizQTClient-Release-QT5 && \
cmake -DCMAKE_BUILD_TYPE=Release -UPDATE_TRANSLATIONS=YES -DCMAKE_PREFIX_PATH=$QTDIR/lib/cmake -DCMAKE_OSX_SYSROOT=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.10.sdk ../WizQTClient && \
make -j5


MYAPP="WizNote"
DEST="$MYAPP.app" # Our final App directory
BUILDDIR=$(pwd);

echo "replace version"
plutil -replace CFBundleVersion -string $REV $MYAPP.app/Contents/Info.plist

mkdir WizNote.app/Contents/Frameworks
#cp -R -p ~/Library/Frameworks/CrashReporter.framework WizNote.app/Contents/Frameworks

echo "call macdeployqt"
$QTDIR/bin/macdeployqt $DEST


#install_name_tool -change @rpath/CrashReporter.framework/Versions/A/CrashReporter \
# @executable_path/../Frameworks/CrashReporter.framework/Versions/A/CrashReporter WizNote.app/Contents/MacOS/WizNote

APPLCERT="Developer ID Application: Beijing Wozhi Technology Co. Ltd (KCS8N3QJ92)"

codesign --verbose=2 --deep --sign "$APPLCERT"  WizNote.app

cd ../WizQTClient

setFile -a V ${package_home}/wiznote-disk-cover.jpg

current_date=`date "+%Y-%m-%d"`
rm -f "${package_output_path}/tmp.dmg"
rm -f "${package_output_path}/wiznote-macos-${current_date}.dmg"
if [ -e "$package_home" ]; then
        # 最好固定打包格式，可以只拷贝需要的文件，避免因为需要sudo权限才能访问的文件无法复制而导致失败
        #cp -R $volumn_path/wiznote.app $volumn_path/.wiznote-disk-cover.jpg $volumn_path/.DS_store $volumn_path/Applications $package_data_path
        rm -rf ./${package_home}/WizNote.app && \
        cp -R ../WizQTClient-Release-QT5/WizNote.app ${package_home} && \
        hdiutil makehybrid -hfs -hfs-volume-name $volumn_name -hfs-openfolder $package_home $package_home -o "${package_output_path}/tmp.dmg" && \
        hdiutil convert -format UDZO "${package_output_path}/tmp.dmg" -o  "${package_output_path}/wiznote-macos-${current_date}.dmg" && \
        rm -f "${package_output_path}/tmp.dmg"
        rm -rf ./${package_home}/WizNote.app
else
        echo "error:${package_home} not exist"
fi
