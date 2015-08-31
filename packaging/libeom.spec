%bcond_with x
%bcond_with wayland

Name: libeom
Summary: External Output Manager Library
Version: 1.0.2
Release: 1
Group: Graphics/X Window System
License: MIT
Source0: %{name}-%{version}.tar.gz
Source1001:	%name.manifest

%if %{with wayland}
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(ecore-wayland)
%else
%endif
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-unix-2.0)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(elementary)

%description
This package provides the runtime library to manage eo(External Output)

%package devel
Summary: External Output Manager Library development package
Group: Development/Libraries
Requires: %{name} = %{version}-%{release}
Requires: pkgconfig(capi-base-common)

%description devel
External Output Manager Library development package

%prep
%setup -q
cp %{SOURCE1001} .

%build
export CFLAGS="-g -O0 -Wall -Werror -Wno-error=deprecated-declarations"
export LDFLAGS="$LDFLAGS -Wl,--hash-style=both -Wl,--as-needed"
%if %{with wayland}
%reconfigure --disable-dlog --disable-static --with-eom-platform=WAYLAND
%else
%if 0%{?tizen_version_major} == 2
%reconfigure --disable-dlog --disable-static --with-eom-platform=X11 --with-tizen-version=TIZEN_2_X
%else
%reconfigure --disable-dlog --disable-static --with-eom-platform=X11 --with-tizen-version=TIZEN_3_X
%endif
%endif

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp -af COPYING %{buildroot}/usr/share/license/%{name}
%make_install

%remove_docs

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
/usr/share/license/%{name}
%{_libdir}/libeom.so.*

%files devel
%manifest %{name}.manifest
%dir %{_includedir}/eom/
%{_includedir}/eom/*.h
%{_libdir}/libeom.so
%{_libdir}/pkgconfig/eom.pc

