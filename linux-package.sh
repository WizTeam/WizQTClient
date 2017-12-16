# compile
mkdir ../WizQTClient-Release-Linux
rm -rf ../WizQTClient-Release-Linux/*

cd ../WizQTClient-Release-Linux

cmake -Wno-dev -DWIZNOTE_USE_QT5=NO -DCMAKE_BUILD_TYPE=Release ../WizQTClient && \
make -j5

cd ..
rm -rf WizNote
mkdir WizNote
cd WizNote
mkdir bin
cd bin

cp ../../WizQTClient-Release-Linux/src/WizNote ./
cp /usr/lib/x86_64-linux-gnu/libQtWebKit.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtGui.so.4 ./
cp /usr/lib/x86_64-linux-gnu//libQtXml.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtNetwork.so.4 ./
cp /usr/lib/x86_64-linux-gnu/libQtCore.so.4 ./

cd ..
mkdir lib
cd lib
mkdir wiznote
cd wiznote
mkdir plugins
cd plugins

cp ../../../../WizQTClient-Release-Linux/lib/wiznote/plugins/libextensionsystem.so ./
cp ../../../../WizQTClient-Release-Linux/lib/wiznote/plugins/libaggregation.so ./
cp ../../../../WizQTClient-Release-Linux/lib/wiznote/plugins/libCore.so ./

cd ..
cd ..
cd ..

cp -R ../WizQTClient-Release-Linux/share ./
#cp ../WizQTClient/start-WizNote.sh ./
ln -s bin/WizNote WizNote
cd ..
rm -f ./WizNote.tar.gz
tar -zcvf ./WizNote.tar.gz WizNote
