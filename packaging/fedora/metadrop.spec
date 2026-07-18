Name:           metadrop
Version:        0.1.3
Release:        1%{?dist}
Summary:        Native Linux metadata inspector and sanitizer
License:        GPL-3.0-or-later
URL:            https://github.com/Trendorin/MetaDrop
Source0:        %{url}/archive/refs/tags/v%{version}/MetaDrop-%{version}.tar.gz

BuildRequires:  cmake >= 3.25
BuildRequires:  gcc-c++
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(exiv2)
BuildRequires:  pkgconfig(libarchive)
BuildRequires:  pkgconfig(libqpdf)
BuildRequires:  pkgconfig(taglib)
BuildRequires:  qt6-qtbase-devel >= 6.4
BuildRequires:  qt6-qtsvg-devel >= 6.4
BuildRequires:  qt6-qttools-devel >= 6.4

%description
MetaDrop reviews privacy-sensitive metadata and creates a verified sanitized
copy without overwriting the source file. The interface uses Qt 6 Widgets and
follows the active Linux desktop theme.

%prep
%autosetup -n MetaDrop-%{version}

%build
%cmake -G Ninja -DMETADROP_BUILD_TESTS=ON
%cmake_build

%check
%ctest

%install
%cmake_install

%files
%license %{_datadir}/doc/metadrop/LICENSE
%doc %{_datadir}/doc/metadrop/README.md
%doc %{_datadir}/doc/metadrop/README.ru.md
%doc %{_datadir}/doc/metadrop/README.uk.md
%doc %{_datadir}/doc/metadrop/README.de.md
%doc %{_datadir}/doc/metadrop/CHANGELOG.md
%doc %{_datadir}/doc/metadrop/CONTRIBUTING.md
%doc %{_datadir}/doc/metadrop/CONTRIBUTORS.md
%doc %{_datadir}/doc/metadrop/RELEASE_NOTES.md
%doc %{_datadir}/doc/metadrop/PRIVACY.md
%doc %{_datadir}/doc/metadrop/SECURITY.md
%doc %{_datadir}/doc/metadrop/SECURITY_MODEL.md
%doc %{_datadir}/doc/metadrop/SUPPORT.md
%doc %{_datadir}/doc/metadrop/THIRD_PARTY_NOTICES.md
%doc %{_datadir}/doc/metadrop/copyright
%{_bindir}/metadrop
%{_datadir}/applications/io.github.trendorin.MetaDrop.desktop
%{_datadir}/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png
%{_datadir}/metainfo/io.github.trendorin.MetaDrop.metainfo.xml

%changelog
* Sun Jul 19 2026 Trendorin - 0.1.3-1
- Align native release packages and documentation structure

* Sun Jul 19 2026 Trendorin - 0.1.2-1
- Refresh application identity and multilingual installation documentation

* Sat Jul 18 2026 Trendorin - 0.1.1-1
- Regenerate PDF identifiers and install package notices

* Sat Jul 18 2026 Trendorin - 0.1.0-1
- Initial package
