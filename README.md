WizQTClient
===========

为知笔记跨平台客户端

Wiznote是一个开源项目，个人用户使用遵循GPLv3协议，任何人都可以获得源代码，做出任何修改。
公司用户如果想要采用，修改，分发wiznote的代码，需向我知科技有限公司取得商业授权许可，在未取得商业授权之前私自采用，分发wiznote中的部分或全部代码，都将追究其法律责任。

---

### 1. 开发环境及编译说明

Qt开发采用一般使用Qt自己的IDE：QtCreator

QtCreator具有fake Vim模式，能够使用Vim的快捷键绑定的基础上，也方便于代码调试。

qmake是Qt开发默认的makefile生成程序，鉴于qmake的各种弱点，wiznote已经不在使用qmake而转向cmake, QtCreator能够很好地支持cmake编译

wiznote的编译过程已在ubuntu12.04以及OSX 10.8.2测试过

windows暂不支持

### 1. 准备Qt库以及QtCreator：
你有两种方式获得Qt库：从源代码编译或是使用系统内建的QT库

到这里下载Qt和QtCreator：

<http://qt-project.org/downloads>

> 当前Qt5还有bug，如果使用Qt5编译会无法运行，已经提交了bug，linux用户需要下载基于v4.8.4的源代码自行编译

在ubuntu上，安装g++:

\# sudo apt-get install build-essential

编译Qt库需要以下依赖：

\#sudo apt-get install libfontconfig1-dev libfreetype6-dev libx11-dev libxcursor-dev libxext-dev libxfixes-dev libxft-dev libxi-dev libxrandr-dev libxrender-dev

我们推荐的Qt编译选项：

\# ./configure -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg -nomake demo -nomake example -no-qt3support

\# sudo make && make install

Qt会被安装到/usr/local/Trolltech这个文件夹中

如果你打算直接使用系统内建的Qt库，直接安装即可：

\# sudo apt-get install libqt4-dev

### 3. 编译wiznote
下载源代码：

\# git clone git://github.com/WizTeam/WizQTClient.git

编译之前，打开QtCreator设置好qt的路径以及版本，然后使用QtCreator打开CMakeLists.txt文件，这是QtCreator的工程文件

设置好shadow build的编译路径，并运行cmake之后，即可开始编译过程

Mac OSX下为了向下兼容，需要添加环境变量:

\# export MACOSX_DEPLOYMENT_TARGET=10.6

你可以把这个变量写入~/.profile里边，以便编译时生效

参考文档：
<http://www.cmake.org/pipermail/cmake/2009-November/033131.html>
<http://developer.apple.com/library/mac/#technotes/tn2064/_index.html>

默认编译方式为Release,可设置为Debug默认：

\# CMAKE_BUILD_TYPE=Debug cmake ../wiznote *

Linux下，编译结果位于${CMAKE_BINARY_DIR}/build/目录下

Mac下，编译结果位于${CMAKE_BINARY_DIR}/bundle/目录下

如果编译模式是Release, Mac下会生成dmg包，Linux下会拷贝库文件到build目录

*All things done, enjoy!*
