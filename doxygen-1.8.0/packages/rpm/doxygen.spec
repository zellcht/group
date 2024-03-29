%define version 1.8.0
%define revision 1
%define mmn 1
%define name doxygen

# optionally pass --with-doxywizard to rpmbuild

%define contentdir /var/www
%define suexec_caller doxygen
%define buildroot /var/tmp/%{name}-%{version}-%{revision}root

Summary: A documentation system for C/C++.
Name: doxygen
Version: %{version}
Release: %{revision}
URL: http://www.stack.nl/~dimitri/doxygen/index.html
Vendor: Dimitri van Heesch
License: GNU General Public License
Group: Development/Tools
Source: %{name}-%{version}.src.tar.gz
BuildRoot: %{buildroot}
BuildPrereq: libstdc++-devel >= 2.96, /usr/bin/perl, /usr/bin/latex, /usr/bin/dvips, /usr/bin/gs
Prereq: /sbin/chkconfig, /bin/mktemp, /bin/rm, /bin/mv, libstdc++ >= 2.96
Provides: doxygen = %{mmn}

%description
Doxygen can generate an online class browser (in HTML) and/or a
reference manual (in LaTeX) from a set of documented source files. The
documentation is extracted directly from the sources. Doxygen can
also be configured to extract the code structure from undocumented
source files.

%if %{?_with_doxywizard:1}%{!?_with_doxywizard:0}
%package doxywizard
Group: Development/Libraries
Summary: GUI Interface for doxygen.
Requires: doxygen = %{mmn}
Requires: qt >= 3.3
Provides: doxywizard = %{mmn}
# Obsoletes:

%description doxywizard
Doxygen can generate an online class browser (in HTML) and/or a
reference manual (in LaTeX) from a set of documented source files. The
documentation is extracted directly from the sources. Doxygen can
also be configured to extract the code structure from undocumented
source files.

This is the GUI interface for doxygen.  It requires qt and X11 to
install.
%endif

%prep
%setup -q -n %{name}-%{version}
./configure %{?_with_doxywizard} --prefix $RPM_BUILD_ROOT/usr

%build
make %{?_smp_mflags}
make %{?_smp_mflags} pdf

%install
rm -rf $RPM_BUILD_ROOT
make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%doc README LICENSE LANGUAGE.HOWTO examples ./latex/*.pdf
%doc /usr/man/man1/doxygen.1.gz
%doc /usr/man/man1/doxytag.1.gz

%{_bindir}/doxygen
%{_bindir}/doxytag

%if %{?_with_doxywizard:1}%{!?_with_doxywizard:0}
%files doxywizard
%defattr(-,root,root)
%{_bindir}/doxywizard
%doc /usr/man/man1/doxywizard.1.gz
%endif

%changelog
* Fri Apr 18 2008 Kenneth Porter <shiva+doxygenspec@sewingwitch.com> 1.5.5-1
- consolidate with and without doxywizard spec files with rpm macro
- add gs BuildPrereq

* Sun Nov 18 2007 Kevin McBride <kevin@planetsaphire.com> 1.5.4
- consolidated manual package in lieu of --excludedocs flag for rpm --install

* Mon Oct 10 2005 Kevin McBride <kevin@planetsaphire.com> 1.4.5
- fixed versioning bugs.

* Tue Oct 4 2005 Kevin McBride <kevin@planetsaphire.com> 1.4.5
- added obsoletes and proides sections.

* Sun Sep 20 2005 Kevin McBride <kevin@planetsaphire.com> 1.4.4
- modified rpm spec file for Fedora Core acceptance criteria.

* Sun Aug 7 2005 Kevin McBride <kevin@planetsaphire.com> 1.4.4
- created initial rpm spec file for doxygen-1.4.4

