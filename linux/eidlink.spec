Summary: eIDlink native messaging component
Name: eidlink
Version: 0.0.8
Release: 1%{?dist}
License: MIT
Group: Applications/Communications

Source: eidlink-%{version}.tar.gz

BuildRequires: pcsc-lite-devel
BuildRequires: boost-devel
BuildRequires: gcc-c++
%if 0%{?suse_version}
Requires: pcsc-ccid
%else
Requires: ccid
%endif
Conflicts: openct

%description
The eIDlink is a program to help implementing digital signing services
and/or an identity service using the Belgian eID card. It provides
services to webbrowsers to read data from cards, and is intended to work
together with a WebExtension in the browser.

This package contains the native code. For the WebExtension, see your
webbrowser's extension store.

%prep
%setup -n eidlink

%build
%{__make} %{?_smp_mflags} -C linux CFLAGS='-g -O2' CXXFLAGS='-g -O2' eidlink

%install
%{__rm} -rf %{buildroot}
%{__make} -C linux install DESTDIR="%{buildroot}"

%clean
%{__rm} -rf %{buildroot}

%post
if [ "$1" -gt 0 ]; then
        mkdir -p /etc/chromium/native-messaging-hosts
        mkdir -p /etc/opt/chrome/native-messaging-hosts
        mkdir -p /usr/lib/mozilla/native-messaging-hosts
        mkdir -p /usr/lib64/mozilla/native-messaging-hosts
        /usr/bin/eidlink -setup /etc/chromium/native-messaging-hosts/ /usr/lib/mozilla/native-messaging-hosts/
        cp /etc/chromium/native-messaging-hosts/be.bosa.eidlink.json /etc/opt/chrome/native-messaging-hosts/
        cp /usr/lib/mozilla/native-messaging-hosts/be.bosa.eidlink.json /usr/lib64/mozilla/native-messaging-hosts/
fi
%files
%defattr(-, root, root, 0755)
%{_bindir}/eidlink
