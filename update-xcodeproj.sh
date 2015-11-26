mkdir ../build-wiznote-xcode
cd ../build-wiznote-xcode
cmake -DWIZNOTE_USE_QT5=YES -DXCODEBUILD=YES -DCMAKE_BUILD_TYPE=DEBUG -DPLCrashReporter=YES -DCMAKE_PREFIX_PATH=~/Qt5.4.2/5.4/clang_64/lib/cmake -Wno-dev  -G Xcode ../WizQTClient