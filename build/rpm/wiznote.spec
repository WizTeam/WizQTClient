Name:		WizNote
Version:	2.1.0
Release:	1%{?dist}
Summary:	WizNote QT Client

Group:		Application
License:	GPLv3
URL:		http://wiz.cn/
Source0:	WizQTClient.tbz2

BuildRequires:	cmake gcc
Requires:	Qt

%description

 Cross platform cloud based note-taking application
 WizNote is a cross platform cloud based note-taking application.
 .
 support following platforms:
 1. windows xp/vista/7/8.
 2. Mac OSX.
 3. Linux
 3. Android/IOs.
 4. Web
 .
 please refer to WizNote home for more detailed info: http://www.wiznote.com.

%prep
%setup -qc


%build
cmake WizQTClient
make %{?_smp_mflags}


%install
make install DESTDIR=%{buildroot}


%files
/usr/local/
%doc



%changelog

