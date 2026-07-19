<p align="center">
  <img src="data/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png" width="144" alt="MetaDrop application icon">
</p>

<h1 align="center">MetaDrop</h1>
<p align="center">Inspect, sanitize and verify file metadata locally.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/MetaDrop/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/MetaDrop/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/MetaDrop/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/MetaDrop?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#install">Install</a> ·
  <a href="#build-from-source">Source</a> ·
  <a href="#uninstall">Uninstall</a> ·
  <a href="docs/SECURITY_MODEL.md">Security</a> ·
  <a href="https://github.com/Trendorin/MetaDrop/releases">Releases</a>
</p>

MetaDrop is a native Linux application that reveals privacy-sensitive file metadata and creates a separately verified sanitized copy. It uses C++20 and Qt 6 Widgets, follows the active desktop theme, and processes files locally without telemetry, cloud services or network uploads.

## What it does

| Step | Result |
|---|---|
| Inspect | Shows metadata before changing the file and classifies location, identity, device, time and identifier risks. |
| Sanitize | Writes to a private staging file; the original is never overwritten. |
| Verify | Reopens and rescans the result before moving the cleaned copy into place. |
| Batch | Handles multiple files through drag-and-drop, the file manager or automatic-copy mode. |

### Verified format scope

| Family | Formats | Removed and verified | Not treated as metadata |
|---|---|---|---|
| Images | JPEG, PNG, WebP, TIFF, DNG, HEIC/HEIF, AVIF | EXIF, IPTC, XMP, comments, thumbnails | Visible pixels; ICC profile unless explicitly selected |
| Audio | MP3, FLAC, Ogg, Opus, M4A/M4B, MP4, WAV, AIFF | Tags exposed by TagLib and supported artwork containers | Audio/video content and timed metadata tracks |
| PDF | PDF | Document Info, catalog/page XMP, PieceInfo, SpiderInfo, source trailer ID | Page content, forms, annotations and attachments |
| Office | OOXML and OpenDocument files/templates | Core, extended and custom properties, ODF metadata, previews, archive dates/ownership | Comments, tracked changes, hidden sheets/slides, macros and visible content |

HEIF/AVIF support depends on Exiv2 being built with BMFF support. Unsupported files are reported as unsupported and are never marked clean.

<a id="install"></a>
## Install

Download the matching asset and `SHA256SUMS` from the [latest release](https://github.com/Trendorin/MetaDrop/releases/latest).

| System | Asset | Command |
|---|---|---|
| Fedora 44 | `metadrop-*.rpm` | `sudo dnf install ./metadrop-*.rpm` |
| Ubuntu 24.04 | `metadrop_*.deb` | `sudo apt install ./metadrop_*.deb` |
| Arch Linux | `metadrop-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./metadrop-*-x86_64.pkg.tar.zst` |

Each binary package is built and install-tested in its native target: Ubuntu 24.04, Fedora 44 or current Arch Linux. `PKGBUILD` is included for a source rebuild with `makepkg -si`.

Verify downloaded files before installation:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="build-from-source"></a>
## Build from source

Requirements: CMake 3.25+, Ninja, a C++20 compiler, Qt 6.4+, Exiv2, TagLib, qpdf and libarchive.

<details>
<summary>Fedora build dependencies</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  exiv2-devel taglib-devel qpdf-devel libarchive-devel
```
</details>

<details>
<summary>Ubuntu 24.04+ build dependencies</summary>

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-svg-dev qt6-tools-dev \
  libexiv2-dev libtag1-dev libqpdf-dev libarchive-dev
```
</details>

```bash
git clone https://github.com/Trendorin/MetaDrop.git
cd MetaDrop
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

<a id="uninstall"></a>
## Uninstall

| Installation | Command |
|---|---|
| Fedora 44 | `sudo dnf remove metadrop` |
| Ubuntu 24.04 | `sudo apt remove metadrop` |
| Arch Linux | `sudo pacman -Rns metadrop` |
| Source build | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Optional: remove local preferences with `rm -f "$HOME/.config/Trendorin/MetaDrop.conf"`. Cleaned output files are user documents and are never removed by uninstalling MetaDrop.

## Security boundary

- Sources are never overwritten; output is installed only after a successful reopen and rescan.
- Symbolic links and non-regular input files are rejected.
- Staging files are owner-only, archive expansion is bounded, and final placement is atomic.
- Metadata removal reduces accidental disclosure but does not anonymize visible content, filenames, filesystem records or the account used to share a file.

Read the [security model](docs/SECURITY_MODEL.md), [data-handling policy](docs/PRIVACY.md), [vulnerability policy](SECURITY.md), and [third-party notices](THIRD_PARTY_NOTICES.md).

## Project

[Changelog](CHANGELOG.md) · [Contributing](CONTRIBUTING.md) · [Contributors](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Maintained by [Trendorin](https://github.com/Trendorin). Licensed under [GPL-3.0-or-later](LICENSE).
