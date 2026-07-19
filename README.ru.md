<p align="center">
  <img src="data/icons/hicolor/512x512/apps/io.github.trendorin.MetaDrop.png" width="144" alt="Иконка приложения MetaDrop">
</p>

<h1 align="center">MetaDrop</h1>
<p align="center">Просмотр, очистка и проверка метаданных локально.</p>

<p align="center">
  <a href="README.md"><img alt="English" src="https://img.shields.io/badge/EN-English-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.ru.md"><img alt="Русский" src="https://img.shields.io/badge/RU-Русский-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.uk.md"><img alt="Українська" src="https://img.shields.io/badge/UK-Українська-d7dade?style=flat-square&labelColor=15181b"></a>
  <a href="README.de.md"><img alt="Deutsch" src="https://img.shields.io/badge/DE-Deutsch-d7dade?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="https://github.com/Trendorin/MetaDrop/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Trendorin/MetaDrop/ci.yml?branch=main&style=flat-square&label=build&labelColor=15181b&color=5d666d"></a>
  <a href="https://github.com/Trendorin/MetaDrop/releases/latest"><img alt="Релиз" src="https://img.shields.io/github/v/release/Trendorin/MetaDrop?style=flat-square&label=release&labelColor=15181b&color=5d666d"></a>
  <img alt="C++20" src="https://img.shields.io/badge/C%2B%2B-20-5d666d?style=flat-square&labelColor=15181b">
  <img alt="Qt 6 Widgets" src="https://img.shields.io/badge/Qt-6_Widgets-5d666d?style=flat-square&labelColor=15181b">
  <a href="LICENSE"><img alt="GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-5d666d?style=flat-square&labelColor=15181b"></a>
</p>

<p align="center">
  <a href="#install">Установка</a> ·
  <a href="#build-from-source">Исходники</a> ·
  <a href="#uninstall">Удаление</a> ·
  <a href="docs/SECURITY_MODEL.md">Безопасность</a> ·
  <a href="https://github.com/Trendorin/MetaDrop/releases">Релизы</a>
</p>

MetaDrop — нативное Linux-приложение, которое показывает конфиденциальные метаданные и создаёт отдельно проверенную очищенную копию. Проект написан на C++20 и Qt 6 Widgets, использует системную тему и обрабатывает файлы локально — без телеметрии, облака и сетевой загрузки.

Интерфейс использует язык системы или сразу переключается в настройках между английским, русским, украинским и немецким.

## Что делает MetaDrop

| Этап | Результат |
|---|---|
| Просмотр | Показывает метаданные до изменений и оценивает риски локации, личности, устройства, времени и идентификаторов. |
| Очистка | Записывает приватный временный файл; оригинал никогда не перезаписывается. |
| Проверка | Повторно открывает и сканирует результат до сохранения очищенной копии. |
| Пакетный режим | Принимает несколько файлов через drag-and-drop, файловый менеджер или автоматический режим. |

### Проверяемая поддержка форматов

| Семейство | Форматы | Удаляется и проверяется | Не считается метаданными |
|---|---|---|---|
| Изображения | JPEG, PNG, WebP, TIFF, DNG, HEIC/HEIF, AVIF | EXIF, IPTC, XMP, комментарии, миниатюры | Видимые пиксели; ICC-профиль без явного выбора |
| Аудио | MP3, FLAC, Ogg, Opus, M4A/M4B, MP4, WAV, AIFF | Теги TagLib и поддерживаемые контейнеры обложек | Аудио/видео и временные metadata-треки |
| PDF | PDF | Document Info, XMP каталога/страниц, PieceInfo, SpiderInfo, исходный trailer ID | Страницы, формы, аннотации и вложения |
| Office | OOXML и OpenDocument, включая шаблоны | Основные, расширенные и пользовательские свойства, ODF-метаданные, превью, даты/владельцы архива | Комментарии, правки, скрытые листы/слайды, макросы и видимое содержимое |

HEIF/AVIF требуют Exiv2 с поддержкой BMFF. Неподдерживаемый файл показывается как неподдерживаемый и никогда не помечается очищенным.

<a id="install"></a>
## Установка

Скачайте подходящий файл и `SHA256SUMS` из [последнего релиза](https://github.com/Trendorin/MetaDrop/releases/latest).

| Система | Файл | Команда |
|---|---|---|
| Fedora 44 | `metadrop-*.rpm` | `sudo dnf install ./metadrop-*.rpm` |
| Ubuntu 24.04 | `metadrop_*.deb` | `sudo apt install ./metadrop_*.deb` |
| Arch Linux | `metadrop-*-x86_64.pkg.tar.zst` | `sudo pacman -U ./metadrop-*-x86_64.pkg.tar.zst` |

Каждый бинарный пакет собирается и проверяется установкой в целевой системе: Ubuntu 24.04, Fedora 44 или актуальной Arch Linux. Для сборки из исходников в релизе остаётся `PKGBUILD`: `makepkg -si`.

Проверка скачанных файлов:

```bash
sha256sum --ignore-missing --check SHA256SUMS
```

<a id="build-from-source"></a>
## Сборка из исходников

Нужны CMake 3.25+, Ninja, компилятор C++20, Qt 6.4+, Exiv2, TagLib, qpdf и libarchive.

<details><summary>Зависимости Fedora</summary>

```bash
sudo dnf install gcc-c++ cmake ninja-build \
  qt6-qtbase-devel qt6-qtsvg-devel qt6-qttools-devel \
  exiv2-devel taglib-devel qpdf-devel libarchive-devel
```
</details>

<details><summary>Зависимости Ubuntu 24.04+</summary>

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
## Удаление

| Способ установки | Команда |
|---|---|
| Fedora 44 | `sudo dnf remove metadrop` |
| Ubuntu 24.04 | `sudo apt remove metadrop` |
| Arch Linux | `sudo pacman -Rns metadrop` |
| Сборка из исходников | `xargs -r -d '\n' rm -- < build/release/install_manifest.txt` |

Настройки можно удалить командой `rm -f "$HOME/.config/Trendorin/MetaDrop.conf"`. Очищенные копии являются пользовательскими документами и при удалении приложения не затрагиваются.

## Граница безопасности

- Оригинал не перезаписывается; результат сохраняется только после повторного открытия и сканирования.
- Символические ссылки и специальные файлы отклоняются.
- Временные файлы доступны только владельцу, распаковка архивов ограничена, итоговая установка атомарна.
- Удаление метаданных снижает риск случайной утечки, но не анонимизирует видимое содержимое, имя файла, записи файловой системы или аккаунт публикации.

Подробнее: [модель безопасности](docs/SECURITY_MODEL.md), [обработка данных](docs/PRIVACY.md), [сообщение об уязвимостях](SECURITY.md), [сторонние компоненты](THIRD_PARTY_NOTICES.md).

## Проект

[Изменения](CHANGELOG.md) · [Участие](CONTRIBUTING.md) · [Авторы](CONTRIBUTORS.md) · [Поддержка](SUPPORT.md)

Проект поддерживает [Trendorin](https://github.com/Trendorin). Лицензия: [GPL-3.0-or-later](LICENSE).
