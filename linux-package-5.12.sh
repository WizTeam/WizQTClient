# compile
mkdir ../WizQTClient-Release-Linux
rm -rf ../WizQTClient-Release-Linux/*

cd ../WizQTClient-Release-Linux

cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=~/Qt5.12.4/5.12.4/gcc_64  ../WizQTClient && \
make -j2

cd ..
rm -rf Package
mkdir Package
cd Package
mkdir WizNote
cd WizNote
mkdir bin
cd bin

cp ../../../WizQTClient-Release-Linux/src/WizNote ./
mkdir qtwebengine_dictionaries
cp -R ../../../WizQTClient/share/qtwebengine_dictionaries ./

cd ..
cp -R ../../WizQTClient-Release-Linux/share ./

cd ..

mkdir logo
cd logo
mkdir hicolor
cd hicolor
mkdir 16x16
mkdir 32x32
mkdir 64x64
mkdir 128x128
mkdir 256x256
mkdir 512x512

cp ../../../WizQTClient/build/common/logo/wiznote16.png 16x16/wiznote.png
cp ../../../WizQTClient/build/common/logo/wiznote32.png 32x32/wiznote.png
cp ../../../WizQTClient/build/common/logo/wiznote64.png 64x64/wiznote.png
cp ../../../WizQTClient/build/common/logo/wiznote128.png 128x128/wiznote.png
cp ../../../WizQTClient/build/common/logo/wiznote256.png 256x256/wiznote.png
cp ../../../WizQTClient/build/common/logo/wiznote512.png 512x512/wiznote.png

cd ..
cd ..

cp ../WizQTClient/build/common/wiznote2.desktop ./wiznote.desktop
cd ./WizNote
mkdir plugins
mkdir lib
cd ./plugins/
mkdir platforminputcontexts
cd ../..
#cp ../WizQTClient/libfcitxplatforminputcontextplugin.so ./WizNote/plugins/platforminputcontexts
cp /lib/x86_64-linux-gnu/libssl.so.1.0.0 ./WizNote/lib/
cp /lib/x86_64-linux-gnu/libcrypto.so.1.0.0 ./WizNote/lib/

cd ..


./WizQTClient/linuxdeployqt ./Package/wiznote.desktop -verbose=1 -appimage -qmake=../Qt5.12.4/5.12.4/gcc_64/bin/qmake


