<p align="center">
  <img src="data/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png" width="144" alt="Іконка застосунку MetaDrop">
</p>

<h1 align="center">MetaDrop</h1>
<p align="center">Перегляд, очищення та перевірка метаданих локально.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/MetaDrop/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/MetaDrop/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/MetaDrop/releases/latest"><img alt="Реліз" src="https://img.shields.io/github/v/release/Trendorin/MetaDrop?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#install">Встановлення</a> · <a href="#build-from-source">Вихідний код</a> ·
  <a href="#uninstall">Видалення</a> · <a href="docs/SECURITY_MODEL.md">Безпека</a> ·
  <a href="https://github.com/Trendorin/MetaDrop/releases">Релізи</a>
</p>

MetaDrop — нативний Linux-застосунок, що показує конфіденційні метадані та створює окремо перевірену очищену копію. Проєкт написаний на C++20 і Qt 6 Widgets, використовує системну тему й обробляє файли локально — без телеметрії, хмари та мережевого завантаження.

## Що робить MetaDrop

| Етап | Результат |
|---|---|
| Перегляд | Показує метадані до змін і оцінює ризики геолокації, особи, пристрою, часу та ідентифікаторів. |
| Очищення | Записує приватний тимчасовий файл; оригінал ніколи не перезаписується. |
| Перевірка | Повторно відкриває та сканує результат до збереження очищеної копії. |
| Пакетний режим | Приймає кілька файлів через drag-and-drop, файловий менеджер або автоматичний режим. |

### Перевірювана підтримка форматів

| Сімейство | Формати | Видаляється та перевіряється | Не вважається метаданими |
|---|---|---|---|
| Зображення | JPEG, PNG, WebP, TIFF, DNG, HEIC/HEIF, AVIF | EXIF, IPTC, XMP, коментарі, мініатюри | Видимі пікселі; ICC-профіль без явного вибору |
| Аудіо | MP3, FLAC, Ogg, Opus, M4A/M4B, MP4, WAV, AIFF | Теги TagLib і підтримувані контейнери обкладинок | Аудіо/відео та часові metadata-треки |
| PDF | PDF | Document Info, XMP каталогу/сторінок, PieceInfo, SpiderInfo, початковий trailer ID | Сторінки, форми, анотації та вкладення |
| Office | OOXML і OpenDocument, включно з шаблонами | Основні, розширені й користувацькі властивості, ODF-метадані, прев'ю, дати/власники архіву | Коментарі, виправлення, приховані аркуші/слайди, макроси та видимий вміст |

HEIF/AVIF потребують Exiv2 із підтримкою BMFF. Непідтримуваний файл ніколи не позначається очищеним.

<a id="install"></a>
## Встановлення

Завантажте потрібний файл і `SHA256SUMS` з [останнього релізу](https://github.com/Trendorin/MetaDrop/releases/latest).

| Система | Файл | Команда |
|---|---|---|
| Fedora 44 | `metadrop-*.rpm` | `sudo dnf install ./metadrop-*.rpm` |
| Debian / Ubuntu | `metadrop_*.deb` | `sudo apt install ./metadrop_*.deb` |
| Arch-based | `PKGBUILD` | `makepkg -si` |

RPM збирається та перевіряється встановленням у Fedora 44. Він не заявлений як пакет для RHEL.

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="build-from-source"></a>
## Збірка з вихідного коду

Потрібні CMake 3.25+, Ninja, компілятор C++20, Qt 6.4+, Exiv2, TagLib, qpdf і libarchive.

<details><summary>Залежності Fedora</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  exiv2-devel taglib-devel qpdf-devel libarchive-devel
```
</details>

<details><summary>Залежності Ubuntu 24.04+</summary>

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
## Видалення

| Спосіб встановлення | Команда |
|---|---|
| Fedora 44 | `sudo dnf remove metadrop` |
| Debian / Ubuntu | `sudo apt remove metadrop` |
| Arch-based | `sudo pacman -Rns metadrop` |
| Збірка з вихідного коду | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Налаштування можна видалити командою `rm -f "$HOME/.config/Trendorin/MetaDrop.conf"`. Очищені копії є документами користувача й не видаляються разом із застосунком.

## Межа безпеки

- Оригінал не перезаписується; результат зберігається лише після повторного відкриття та сканування.
- Символічні посилання й спеціальні файли відхиляються.
- Тимчасові файли доступні лише власнику, розпакування архівів обмежене, фінальне переміщення атомарне.
- Видалення метаданих зменшує ризик випадкового витоку, але не анонімізує видимий вміст, назву файла, записи файлової системи чи обліковий запис публікації.

Докладніше: [модель безпеки](docs/SECURITY_MODEL.md), [обробка даних](docs/PRIVACY.md), [повідомлення про вразливості](SECURITY.md), [сторонні компоненти](THIRD_PARTY_NOTICES.md).

## Проєкт

[Зміни](CHANGELOG.md) · [Участь](CONTRIBUTING.md) · [Автори](CONTRIBUTORS.md) · [Підтримка](SUPPORT.md)

Проєкт підтримує [Trendorin](https://github.com/Trendorin). Ліцензія: [GPL-3.0-or-later](LICENSE).
