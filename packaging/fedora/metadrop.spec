Name:           metadrop
Version:        0.1.0
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
BuildRequires:  qt6-qtbase-devel >= 6.5
BuildRequires:  qt6-qtsvg-devel >= 6.5
BuildRequires:  qt6-qttools-devel >= 6.5

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
%license LICENSE
%doc README.md CHANGELOG.md SECURITY.md
%{_bindir}/metadrop
%{_datadir}/applications/io.github.trendorin.MetaDrop.desktop
%{_datadir}/icons/hicolor/scalable/apps/io.github.trendorin.MetaDrop.svg
%{_datadir}/metainfo/io.github.trendorin.MetaDrop.metainfo.xml

%changelog
* Sat Jul 18 2026 Trendorin - 0.1.0-1
- Initial package
