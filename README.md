# WizNote for Mac/Linux


## cross-platform cloud based note-taking client
WizNote is an open-sourced project, published under GPLv3 for individual/personal users and custom commercial license for company users.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.


## Introduction

The project is based on Qt, aimed to provide an excellent PKM(personal knowledge management) desktop environment based on cloud usage. At present, we only have Wiz cloud backend(our company) on the table. but we strong encourage developers to contribute to this project to add more cloud backend for different cloud providers like evernote, youdao, etc...even offline usage.

PKM should be an very important thing cross through one person's life, it's unwise to stick yourself to a fixed service provider or jump around and leave your collected info/secrets behide. PKM should be the same as your mind, fly over the ocean but never splash the waves.

freedom, means knowledge, means PKM, means this WizNote client.

if you are windows or portable platform users, we have WizNote for windows, ios, android from our [Homepage](http://www.wiznote.com)


## Compile

visit: (http://wiznote-qt.com)

Different distributions or platforms
---

### Mac OSX

[Download](http://www.wiz.cn/wiznote-maclinux.html) from our homepage or install from [AppStore](https://itunes.apple.com/cn/app/wiznote/id863771545?l=zh&ls=1&mt=12).

### Ubuntu

install from [PPA](https://launchpad.net/~wiznote-team/+archive/ppa) is much more convenient:

    $ sudo add-apt-repository ppa:wiznote-team
    $ sudo apt-get update
    $ sudo apt-get install wiznote


### Fedora 22+

    $ dnf install dnf-plugins-core
    $ dnf copr enable mosquito/wiznote
    $ dnf install wiznote   # Stable version 
    Or
    $ dnf install wiznote-beta    # Development version

Thanks for mosquito's contribution, [more Fedora/CentOS soft.](https://copr.fedorainfracloud.org/coprs/mosquito/)


### Fedora 21/RHEL/CentOS 7

    $ yum install yum-plugin-copr 
    $ yum copr enable mosquito/wiznote
    $ yum install wiznote     # Stable version  
    Or
    $ yum install wiznote-beta    # Development version

If it comes with an error that cannot install yum-plugin-copr, you need to add [repo info](https://copr.fedorainfracloud.org/coprs/mosquito/wiznote/repo/epel-7/mosquito-wiznote-epel-7.repo) to your repo list in file `/etc/yum.repos.d/epel.repo`, or download the repo file to repos directory.
Here is the way of download the repo file:

	$ cd /etc/yum.repos.d/
	$ sudo wget https://copr.fedorainfracloud.org/coprs/mosquito/wiznote/repo/epel-7/mosquito-wiznote-epel-7.repo
	$ sudo yum install wiznote


### others

Read the compile section and compile for your own flavor!


*All things done, enjoy!*
