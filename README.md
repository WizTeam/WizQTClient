WizQTClient
===========

为知笔记跨平台客户端

wiznote是一个自由软件，基于GPLv3协议，任何人都可以获得源代码，做出任何修改。

---

### 1. 准备Qt库以及QtCreator：
你有两种方式获得Qt库：源代码或是已编译好的链接库

到这里下载Qt和QtCreator：
http://qt-project.org/downloads

> 当前Qt5还有bug，如果使用Qt5编译会无法运行，已经提交了bug，linux用户需要下载基于v4.8.4的源代码自行编译 *

在ubuntu上，安装g++:
\# sudo apt-get install build-essential

编译Qt库需要以下依赖：
\#sudo apt-get install libfontconfig1-dev libfreetype6-dev libx11-dev libxcursor-dev libxext-dev libxfixes-dev libxft-dev libxi-dev libxrandr-dev libxrender-dev

我们推荐的Qt编译选项：
\# ./configure -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg -nomake demo -nomake example -no-qt3support
\# sudo make && make install

Qt会被安装到/usr/local/Trolltech这个文件夹中

这是采用的动态链接的编译方式，并且使用内置的plugins解析图片

### 2. 编译wiznote依赖：
wiznote的加密解密部分使用著名的cryptopp库，因为这个库还没有放入到wiznote源代码中，所以必须要手动编译一遍

下载地址：
http://cryptopp.com/

下载这个库最新的v5.6.1，解压到libcryptopp文件夹，以保证wiznote能够找到链接库，然后运行：
\# make -f GNUmakefile

### 3. 编译wiznote
下载源代码：
\# git clone git://github.com/WizTeam/WizQTClient.git

我们将会尽量保证每一次的代码提交都是可正常编译运行的，所以，你尽可以放心下载最新的代码来使用

编译之前，打开QtCreator设置好qt的路径以及版本，然后使用QtCreator打开src/wiznote.pro文件，这是QtCreator的工程文件

wiznote的运行需要依赖share/wznote/中的资源文件，所以我们需要拷贝这些文件到调试和发布目录
把share/wiznote文件夹中的所有文件拷贝到类似wiznote-build-Desktop这样一个前缀的文件夹中

*All things done, enjoy!*
