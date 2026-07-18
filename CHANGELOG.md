# Changelog

All notable changes are documented here. MetaDrop follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Planned

- Reproducible Flatpak packaging after the sandbox portal workflow is validated.
- Additional fixture-driven format coverage without weakening verification.

## [0.1.4] - 2026-07-19

### Fixed

- Build the RPM inside Fedora 44 so it links against Fedora's `libexiv2.so.28` and `libqpdf.so.30` instead of Ubuntu library versions.
- Install the generated RPM through DNF and check all dynamic-library links before a release can be published.

### Changed

- Split DEB and RPM packaging into distribution-native release jobs.
- Document the binary RPM as a Fedora 44 package instead of claiming unverified RHEL compatibility.

## [0.1.3] - 2026-07-19

### Changed

- Replaced the wide README banner with the centered application icon across EN, RU, UK and DE documentation.
- Aligned the documentation layout with Vacuo and removed the unused code-of-conduct file.
- Standardized release output as DEB, RPM, TGZ, Arch `PKGBUILD`, source, SPDX SBOM and `SHA256SUMS`.
- Removed AppImage from new releases so both projects publish the same supported artifact set.
- Added a dedicated security model, support page and CodeQL workflow.

## [0.1.2] - 2026-07-19

### Changed

- Replaced the application icon and compacted the repository header around a shared monochrome identity.
- Rebuilt the README around the verified format boundary, package installation, source builds and uninstallation.
- Added complete Russian, Ukrainian and German README translations alongside the English default.
- Added RPM and Arch `PKGBUILD` artifacts to the automated release alongside AppImage, DEB and source packages.
- Added build-provenance attestations and concise release notes without changing sanitization behavior.

## [0.1.1] - 2026-07-18

### Security

- Replace the original PDF trailer `/ID` during a full rewrite and verify that the cleaned file received a different identifier.

### Changed

- Add an integration test covering PDF document information and identifier replacement.
- Install project licensing, privacy, security, and third-party notices with binary packages.
- Declare the Qt SVG runtime dependency explicitly for Debian packages.
- Make release version resolution and artifact checksums deterministic.

## [0.1.0] - 2026-07-18

### Added

- Native C++20 and Qt 6 Widgets application with system theme integration.
- Drag-and-drop, batch inspection, metadata filtering, system tray, and automatic-copy mode.
- Image inspection and sanitization through Exiv2.
- Audio tag inspection and sanitization through TagLib.
- Full-rewrite PDF document metadata removal through qpdf.
- OOXML and OpenDocument property cleanup through libarchive.
- Privacy risk classification for location, identity, device, time, software, and persistent IDs.
- Private staging files, source-file protection, atomic output installation, and mandatory rescan verification.
- English and Russian interface resources.
- CI, sanitizer tests, AppImage and Debian release automation.

[Unreleased]: https://github.com/Trendorin/MetaDrop/compare/v0.1.4...HEAD
[0.1.4]: https://github.com/Trendorin/MetaDrop/compare/v0.1.3...v0.1.4
[0.1.3]: https://github.com/Trendorin/MetaDrop/compare/v0.1.2...v0.1.3
[0.1.2]: https://github.com/Trendorin/MetaDrop/compare/v0.1.1...v0.1.2
[0.1.1]: https://github.com/Trendorin/MetaDrop/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/Trendorin/MetaDrop/releases/tag/v0.1.0
