Name: plot
Version: 0.4.0
Release: 1%{?dist}
Summary: Generate a simple ascii plot
License: MIT
URL: https://github.com/annacrombie/plot
Prefix: /usr
Source0: %{expand:%%(pwd)}
BuildRoot: %(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}%{dist}-XXXXXX)
BuildRequires: gcc, meson, ninja-build, rpm-build

%description
Generate a simple ascii plot.

%prep
rm -rf ./build
meson --prefix "${RPM_BUILD_ROOT}/usr" ./build "%{SOURCEURL0}"

%build
ninja-build -C build

%install
rm -rf "${RPM_BUILD_ROOT}"
ninja-build -C build install

%clean
[ "${RPM_BUILD_ROOT}" != '/' ] && rm -rf "${RPM_BUILD_ROOT}"

%files
%attr(-,root,root) /usr/bin/plot

%changelog
