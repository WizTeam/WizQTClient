# compile
readonly currentDir=$(pwd)
readonly releaseDir="$currentDir/../WizNote"
readonly releaseTmpDir="$currentDir/../WizQTClient-Release-Linux"

rm -rf "$releaseTmpDir"
mkdir "$releaseTmpDir"

# find QtPath
QtPath=$(locate libQt5Widgets.so | grep ".*/Qt[0-9].*/[0-9].*/gcc.[0-9]*" -o | sed -n "1p")

cd "$releaseTmpDir"

# specify QtPath
cmake -DWIZNOTE_USE_QT5=NO -DCMAKE_BUILD_TYPE=Release $currentDir \
    -DCMAKE_PREFIX_PATH=$QtPath && make -j5

rm -rf "$releaseDir"
mkdir "$releaseDir"
cd "$releaseDir"
mkdir bin
cd bin

cp $releaseTmpDir/src/WizNote ./
cp /usr/lib/x86_64-linux-gnu/libQtWebKit.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtGui.so.4 ./
cp /usr/lib/x86_64-linux-gnu//libQtXml.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtNetwork.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtCore.so.4 ./

cd "$releaseDir"
mkdir lib
cd lib
mkdir wiznote
cd wiznote
mkdir plugins
cd plugins

#No these three file
#cp $releaseTmpDir/lib/wiznote/plugins/libextensionsystem.so ./
#cp $releaseTmpDir/lib/wiznote/plugins/libaggregation.so ./
#cp $releaseTmpDir/lib/wiznote/plugins/libCore.so ./

cd "$releaseDir"

cp -R $releaseTmpDir/share ./
#cp ../WizQTClient/start-WizNote.sh ./
ln -s bin/WizNote WizNote
cd ..
rm -f ./WizNote.tar.gz
rm -rf "$releaseTmpDir"
tar -zcvf ./WizNote.tar.gz WizNote
