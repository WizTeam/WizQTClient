# compile
mkdir ../WizQTClient-Release-Linux
rm -rf ../WizQTClient-Release-Linux/*

cd ../WizQTClient-Release-Linux

cmake -DWIZNOTE_USE_QT5=NO -DCMAKE_BUILD_TYPE=Release ../WizQTClient -DQT_QMAKE_EXECUTABLE=/usr/local/Trolltech/Qt-4.8.6/bin/qmake  && \
make -j5

cd ..
rm -rf WizNote
mkdir WizNote
cd WizNote
mkdir bin
cd bin

cp ../../WizQTClient-Release-Linux/bin/WizNote ./
cp /usr/local/Trolltech/Qt-4.8.6/lib/libQtWebKit.so.4 ./
cp /usr/local/Trolltech/Qt-4.8.6/lib/libQtGui.so.4 ./
cp /usr/local/Trolltech/Qt-4.8.6/lib//libQtXml.so.4 ./
cp /usr/local/Trolltech/Qt-4.8.6/lib/libQtNetwork.so.4 ./
cp /usr/local/Trolltech/Qt-4.8.6/lib/libQtCore.so.4 ./
cp /usr/local/Trolltech/Qt-4.8.6/lib/libQtXmlPatterns.so.4 ./
cp ../../WizQTClient-Release-Linux/lib/wiznote/plugins/libextensionsystem.so ./
cp ../../WizQTClient-Release-Linux/lib/wiznote/plugins/libaggregation.so ./
cp ../../WizQTClient-Release-Linux/lib/wiznote/plugins/libCore.so ./
cp ../../WizQTClient-Release-Linux/lib/wiznote/plugins/libMarkDown.so ./
cp ../../WizQTClient-Release-Linux/lib/wiznote/plugins/libHelloWorld.so ./

cd ..
mkdir lib
cd lib
mkdir wiznote
cd wiznote
mkdir plugins
cd plugins

cp ../../../../WizQTClient-Release-Linux/lib/wiznote/plugins/MarkDown.pluginspec ./
cp ../../../../WizQTClient-Release-Linux/lib/wiznote/plugins/Core.pluginspec ./


cd ..
cd ..
cd ..

cp -R ../WizQTClient-Release-Linux/share ./
#cp ../WizQTClient/start-WizNote.sh ./
ln -s bin/WizNote WizNote
cd ..
rm ./WizNote.tar.gz
tar -zcvf ./WizNote.tar.gz WizNote
