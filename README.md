# WizNote for Mac/Linux


## cross-platform cloud based note-taking client
WizNote is an open-sourced project, published under GPLv3 for individual/personal users and custom commercial licence for company users.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

company users who want to adopt/modify/redistribute the source/binary form of this project need consult Wiz conpany and/or buy a commercial licence for your rights.


## Introduction

The project is based on Qt, aimed to provide an excellent PKM(personal knowledge management) desktop environment based on cloud usage. At present, we only have Wiz cloud backend(our company) on the table. but we strong encourage developers to contribute to this project to add more cloud backend for different cloud providers like evernote, youdao, etc...even offline usage.

PKM should be an very important thing cross through one person's life, it's unwise to stick yourself to a fixed service provider or jump around and leave your collected info/secrets behide. PKM should be the same as your mind, fly over the ocean but never splash the waves.

freedom, means knowledge, means PKM, means this WizNote client.

if you are windows or portable platform users, we have WizNote for windows, ios, android from our [Homepage](http://www.wiznote.com)


## Compile

### Requirement:

Build time denpendency: cmake gcc

Runtime denpendency: Qt4 or Qt5

### configure:

    $ cmake .

    Or if you want to build under Qt5

    $ cmake -DWIZNOTE_USE_QT5=ON .

### make && install:
    
    $ make
    $ make install

> Q&A: people says, why I can't login when compile && run ?

> When you use the default compilation flag, "Release" mode is implied set. resource files have not copied to building directory.

> Try alternatives:

    Install it with resources under fakeroot dir:

    $ mkdir fakeroot
    $ make DESTDIR=fakeroot/ install 
    $ ./fakeroot/bin/wiznote

    Or just use "Debug" mode for configure:
    
    $ cmake -DCMAKE_BUILD_TYPE=Debug .

We have tested on Linux(ubuntu 12.04+ / Arch with Qt5) and OSX above 10.8.2.


Different distributions or platforms
---

### Mac OSX

[Download](http://www.wiz.cn/wiznote-maclinux.html) from our homepage is the only way currently. We'll push to App Store soon!

### Ubuntu

install from [PPA](https://launchpad.net/~wiznote-team/+archive/ppa) is much more convenient:

    $ sudo add-apt-repository ppa:wiznote-team
    $ sudo apt-get update
    $ sudo apt-get install wiznote

Also, App Store is not so far!

### Arch-Linux

install from offical repo:

    $ pacman -S wiznote


### Fedora

Wow, we have not adopted to this distro yet. volunteers is strongly encouraged!


### others

Read the compile section and compile for your own flavor!


*All things done, enjoy!*
