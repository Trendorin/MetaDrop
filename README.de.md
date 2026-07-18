<p align="center">
  <img src="data/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png" width="144" alt="MetaDrop-Anwendungssymbol">
</p>

<h1 align="center">MetaDrop</h1>
<p align="center">Dateimetadaten lokal prüfen, bereinigen und verifizieren.</p>

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
  <a href="#install">Installation</a> · <a href="#build-from-source">Quellcode</a> ·
  <a href="#uninstall">Deinstallation</a> · <a href="docs/SECURITY_MODEL.md">Sicherheit</a> ·
  <a href="https://github.com/Trendorin/MetaDrop/releases">Releases</a>
</p>

MetaDrop ist eine native Linux-Anwendung, die sensible Dateimetadaten sichtbar macht und eine separat geprüfte, bereinigte Kopie erstellt. Sie verwendet C++20 und Qt 6 Widgets, übernimmt das aktive Systemdesign und verarbeitet Dateien lokal — ohne Telemetrie, Cloud-Dienst oder Netzwerk-Upload.

## Funktionsweise

| Schritt | Ergebnis |
|---|---|
| Prüfen | Zeigt Metadaten vor jeder Änderung und bewertet Risiken durch Ort, Identität, Gerät, Zeit und Kennungen. |
| Bereinigen | Schreibt in eine private temporäre Datei; das Original wird nie überschrieben. |
| Verifizieren | Öffnet und scannt das Ergebnis erneut, bevor die bereinigte Kopie gespeichert wird. |
| Stapel | Verarbeitet mehrere Dateien per Drag-and-drop, Dateimanager oder automatischem Kopiermodus. |

### Verifizierter Formatumfang

| Familie | Formate | Entfernt und geprüft | Nicht als Metadaten behandelt |
|---|---|---|---|
| Bilder | JPEG, PNG, WebP, TIFF, DNG, HEIC/HEIF, AVIF | EXIF, IPTC, XMP, Kommentare, Vorschaubilder | Sichtbare Pixel; ICC-Profil ohne ausdrückliche Auswahl |
| Audio | MP3, FLAC, Ogg, Opus, M4A/M4B, MP4, WAV, AIFF | Von TagLib erkannte Tags und unterstützte Cover-Container | Audio-/Videoinhalt und zeitgesteuerte Metadatenspuren |
| PDF | PDF | Document Info, Katalog-/Seiten-XMP, PieceInfo, SpiderInfo, ursprüngliche Trailer-ID | Seiteninhalt, Formulare, Anmerkungen und Anhänge |
| Office | OOXML- und OpenDocument-Dateien/Vorlagen | Kern-, erweiterte und benutzerdefinierte Eigenschaften, ODF-Metadaten, Vorschauen, Archivdaten/-besitzer | Kommentare, Änderungen, ausgeblendete Blätter/Folien, Makros und sichtbarer Inhalt |

HEIF/AVIF benötigt Exiv2 mit BMFF-Unterstützung. Nicht unterstützte Dateien werden nie als bereinigt markiert.

<a id="install"></a>
## Installation

Lade die passende Datei und `SHA256SUMS` aus dem [aktuellen Release](https://github.com/Trendorin/MetaDrop/releases/latest) herunter.

| System | Datei | Befehl |
|---|---|---|
| Fedora 44 | `metadrop-*.rpm` | `sudo dnf install ./metadrop-*.rpm` |
| Debian / Ubuntu | `metadrop_*.deb` | `sudo apt install ./metadrop_*.deb` |
| Arch-basiert | `PKGBUILD` | `makepkg -si` |

Das RPM wird unter Fedora 44 gebaut und per Installation geprüft. Es wird nicht als RHEL-Paket angeboten.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="build-from-source"></a>
## Aus dem Quellcode bauen

Benötigt werden CMake 3.25+, Ninja, ein C++20-Compiler, Qt 6.4+, Exiv2, TagLib, qpdf und libarchive.

<details><summary>Fedora-Abhängigkeiten</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  exiv2-devel taglib-devel qpdf-devel libarchive-devel
```
</details>

<details><summary>Ubuntu-24.04+-Abhängigkeiten</summary>

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
## Deinstallation

| Installationsart | Befehl |
|---|---|
| Fedora 44 | `sudo dnf remove metadrop` |
| Debian / Ubuntu | `sudo apt remove metadrop` |
| Arch-basiert | `sudo pacman -Rns metadrop` |
| Quellcode-Build | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Lokale Einstellungen lassen sich mit `rm -f "$HOME/.config/Trendorin/MetaDrop.conf"` entfernen. Bereinigte Ausgabedateien sind Benutzerdokumente und werden bei der Deinstallation nicht gelöscht.

## Sicherheitsgrenze

- Originale werden nie überschrieben; Ergebnisse werden erst nach erneutem Öffnen und Scannen gespeichert.
- Symbolische Links und nicht reguläre Eingabedateien werden abgewiesen.
- Temporäre Dateien sind nur für den Besitzer zugänglich, die Archiventpackung ist begrenzt und die endgültige Ablage erfolgt atomar.
- Das Entfernen von Metadaten reduziert unbeabsichtigte Offenlegung, anonymisiert aber keine sichtbaren Inhalte, Dateinamen, Dateisystemeinträge oder Freigabekonten.

Siehe [Sicherheitsmodell](docs/SECURITY_MODEL.md), [Datenrichtlinie](docs/PRIVACY.md), [Schwachstellenrichtlinie](SECURITY.md) und [Hinweise zu Drittkomponenten](THIRD_PARTY_NOTICES.md).

## Projekt

[Änderungen](CHANGELOG.md) · [Mitwirken](CONTRIBUTING.md) · [Mitwirkende](CONTRIBUTORS.md) · [Support](SUPPORT.md)

Betreut von [Trendorin](https://github.com/Trendorin). Lizenz: [GPL-3.0-or-later](LICENSE).
