<p align="center">
  <img src="docs/assets/hero.svg" alt="MetaDrop — native metadata privacy for Linux" width="100%">
</p>

<p align="center">
  <a href="README.md"><strong>English</strong></a> · <a href="README.ru.md">Русский</a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/MetaDrop/actions/workflows/ci.yml"><img alt="Build" src="https://img.shields.io/github/actions/workflow/status/Trendorin/MetaDrop/ci.yml?branch=main&style=flat-square&label=build"></a>
  <a href="https://github.com/Trendorin/MetaDrop/releases/latest"><img alt="Release" src="https://img.shields.io/github/v/release/Trendorin/MetaDrop?style=flat-square&display_name=tag"></a>
  <a href="LICENSE"><img alt="License" src="https://img.shields.io/badge/license-GPL--3.0--or--later-30363d?style=flat-square"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-30363d?style=flat-square">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6%20Widgets-30363d?style=flat-square">
</p>

MetaDrop is a native Linux application for inspecting privacy-sensitive metadata and creating a verified sanitized copy. It uses C++20 and Qt 6 Widgets, follows the active desktop theme, and never sends a file or path over the network.

<p align="center">
  <a href="https://github.com/Trendorin/MetaDrop/releases/latest"><strong>Download the latest release</strong></a>
  &nbsp;·&nbsp;
  <a href="SECURITY.md">Security</a>
  &nbsp;·&nbsp;
  <a href="PRIVACY.md">Privacy</a>
</p>

## One deliberate workflow

<table>
  <tr>
    <td width="33%"><strong>1. Inspect</strong><br>Drop one file or a batch. MetaDrop lists the embedded fields before changing anything.</td>
    <td width="33%"><strong>2. Understand</strong><br>Location, identity, device, timestamp, and persistent-ID fields receive a clear risk level.</td>
    <td width="33%"><strong>3. Sanitize and verify</strong><br>A private copy is cleaned, reopened, rescanned, and saved only when verification passes.</td>
  </tr>
</table>

- Native Qt Widgets interface for KDE Plasma, GNOME, Xfce, Cinnamon, and other Linux desktops.
- System tray, drag-and-drop, batch processing, metadata filtering, and automatic-copy mode.
- No Electron, web view, account, telemetry, analytics, cloud service, or background network client.
- Source files are never overwritten in the current release.

## Verified format scope

MetaDrop is intentionally explicit about what each backend can prove. An unsupported file is reported as unsupported; it is never labeled clean.

| Family | Formats | Removed and verified | Important boundary |
|---|---|---|---|
| Images | JPEG, PNG, WebP, TIFF, DNG, HEIC/HEIF, AVIF | EXIF, IPTC, XMP, image comments, embedded thumbnails | HEIF/AVIF require an Exiv2 build with BMFF support. ICC color profiles are preserved unless explicitly enabled. |
| Audio | MP3, FLAC, Ogg Vorbis, Opus, M4A/M4B, MP4, WAV, AIFF | Tags exposed by TagLib, including common textual tags and supported artwork/tag containers | Timed metadata tracks and visible video content are outside the MP4 scope. |
| PDF | PDF | Document Info, catalog/page XMP, PieceInfo, SpiderInfo | Annotations, form values, attachments, and visible page content are document content, not removed metadata. |
| Documents | DOCX/XLSX/PPTX, macro-enabled OOXML, ODT/ODS/ODP and templates | Core, extended and custom properties, ODF metadata, preview thumbnails, archive ownership and internal timestamps | Comments, tracked changes, hidden sheets/slides, macros, and visible content are not removed. |

Technical data required to render a file—dimensions, codec properties, page structure—is shown separately and retained.

## Install

### AppImage or Debian package

Download the `.AppImage` or `.deb` and its `SHA256SUMS` file from [Releases](https://github.com/Trendorin/MetaDrop/releases/latest). Verify before running:

```bash
sha256sum --check SHA256SUMS
chmod +x MetaDrop-*.AppImage
./MetaDrop-*.AppImage
```

### Build on Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  exiv2-devel taglib-devel qpdf-devel libarchive-devel

git clone https://github.com/Trendorin/MetaDrop.git
cd MetaDrop
cmake --preset release
cmake --build --preset release
ctest --preset release
cmake --install build/release --prefix "$HOME/.local"
```

### Build on Ubuntu 24.04+

```bash
sudo apt install build-essential cmake ninja-build \
  qt6-base-dev qt6-svg-dev qt6-tools-dev \
  libexiv2-dev libtag1-dev libqpdf-dev libarchive-dev

cmake --preset release
cmake --build --preset release
ctest --preset release
```

## Use

1. Drop files into the window or open them with MetaDrop from your file manager.
2. Select a file to review every field exposed by its verified backend.
3. Choose **Clean selected** or **Clean all ready**.
4. Use the saved path shown in the details panel. The `.cleaned` copy has already been reopened and verified.

Automatic cleaning can be enabled in Settings. It still inspects and verifies every file and still creates a separate copy.

## Security and privacy

Metadata cleaning reduces accidental disclosure; it does not make content anonymous. A face, address, document text, filename, file-system timestamp, hidden revision, or account used to upload the file can still identify someone.

The implementation refuses symbolic links and non-regular inputs, writes staging files with owner-only access, applies archive size and entry-count limits, and installs output atomically after verification. Read the complete [security model](SECURITY.md) and [data-handling policy](PRIVACY.md) before relying on MetaDrop for high-risk material.

## Contributing

Bug reports, format fixtures, accessibility improvements, and backend hardening are welcome. Start with [CONTRIBUTING.md](CONTRIBUTING.md); contributors are credited without maintaining a fake or manually inflated list in the README.

## License

MetaDrop is licensed under [GPL-3.0-or-later](LICENSE). See [CONTRIBUTORS.md](CONTRIBUTORS.md) for authorship and third-party acknowledgements.
