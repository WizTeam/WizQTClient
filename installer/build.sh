#!/bin/bash

function userHelp
{
    echo -e "\nUsage:\nbuild.sh [binarycreator location dir] [QtSdk dynamic libraries dir]\n"
}

if [ $# -lt 2 ]; then
    userHelp
    exit 1
fi

if [ $1 == "default" ]; then
    binarycreator="$HOME/bin"
fi

if [ $2 == "default" ]; then
    qtlib="$HOME/QtSDK/Desktop/Qt/4.8.1/gcc/lib"
fi

dataLoc=temp
mkdir $dataLoc
mkdir packages/wiznote/data

echo "Copy application resources from repo"

mkdir $dataLoc/share
cp -R ../share/wiznote/ $dataLoc/share/

echo "Copy compiled new main program"

mkdir $dataLoc/bin
cp ../bin/wiznote $dataLoc/bin/

echo "Copy dynamic libraries"

mkdir $dataLoc/lib
cp $qtlib/libQtWebKit.so.4 $dataLoc/lib/
cp $qtlib/libQtXml.so.4 $dataLoc/lib/
cp $qtlib/libQtNetwork.so.4 $dataLoc/lib/
cp $qtlib/libQtCore.so.4 $dataLoc/lib/
cp $qtlib/libQtGui.so.4 $dataLoc/lib/

echo "Archive All..."

archiveName="wiznote.7z"

cd $dataLoc
7z a -t7z $archiveName
cd ..

cp $dataLoc/$archiveName packages/wiznote/data/

rm -r $dataLoc

echo "Begin build process..."

$binarycreator/binarycreator -f -c config.xml -p packages wiznote.sh

echo "clean temp files"
rm -r packages/wiznote/data
