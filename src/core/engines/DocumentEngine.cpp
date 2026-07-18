#include "core/engines/DocumentEngine.h"

#include "core/OfficeXmlSanitizer.h"
#include "core/RiskClassifier.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <archive.h>
#include <archive_entry.h>

#include <array>
#include <ctime>
#include <memory>

namespace metadrop {
namespace {

constexpr qint64 MaximumMetadataXmlSize = 16 * 1024 * 1024;
constexpr qint64 MaximumExpandedArchiveSize = 16LL * 1024 * 1024 * 1024;
constexpr int MaximumEntryCount = 100000;
constexpr std::time_t NormalizedZipTime = 315532800; // 1980-01-01 UTC

struct ReadArchiveDeleter {
    void operator()(archive* value) const {
        if (value != nullptr) {
            archive_read_free(value);
        }
    }
};

struct WriteArchiveDeleter {
    void operator()(archive* value) const {
        if (value != nullptr) {
            archive_write_free(value);
        }
    }
};

struct EntryDeleter {
    void operator()(archive_entry* value) const {
        if (value != nullptr) {
            archive_entry_free(value);
        }
    }
};

using ReadArchive = std::unique_ptr<archive, ReadArchiveDeleter>;
using WriteArchive = std::unique_ptr<archive, WriteArchiveDeleter>;
using ArchiveEntry = std::unique_ptr<archive_entry, EntryDeleter>;

QString archiveError(archive* value, const QString& fallback) {
    const char* message = value == nullptr ? nullptr : archive_error_string(value);
    return message == nullptr ? fallback : QString::fromUtf8(message);
}

QString entryPath(archive_entry* entry) {
    const char* path = archive_entry_pathname_utf8(entry);
    if (path == nullptr) {
        path = archive_entry_pathname(entry);
    }
    return path == nullptr ? QString() : QString::fromUtf8(path);
}

bool isSafeEntryPath(const QString& path) {
    if (path.isEmpty() || QDir::isAbsolutePath(path)) {
        return false;
    }
    const QString normalized = QDir::cleanPath(path);
    return normalized != QStringLiteral("..") && !normalized.startsWith(QStringLiteral("../"));
}

bool isRegularOrDirectory(archive_entry* entry) {
    const auto type = archive_entry_filetype(entry);
    return type == AE_IFREG || type == AE_IFDIR || type == 0;
}

QByteArray readCurrentEntry(archive* reader, const qint64 declaredSize, QString* error) {
    if (declaredSize < 0 || declaredSize > MaximumMetadataXmlSize) {
        if (error != nullptr) {
            *error = QStringLiteral("A document metadata part is unexpectedly large");
        }
        return {};
    }

    QByteArray data;
    data.reserve(static_cast<qsizetype>(declaredSize));
    std::array<char, 64 * 1024> buffer{};
    while (true) {
        const la_ssize_t count = archive_read_data(reader, buffer.data(), buffer.size());
        if (count == 0) {
            break;
        }
        if (count < 0) {
            if (error != nullptr) {
                *error = archiveError(reader, QStringLiteral("Could not read a document metadata part"));
            }
            return {};
        }
        if (data.size() + count > MaximumMetadataXmlSize) {
            if (error != nullptr) {
                *error = QStringLiteral("A document metadata part exceeds the safety limit");
            }
            return {};
        }
        data.append(buffer.data(), static_cast<qsizetype>(count));
    }
    return data;
}

void normalizeEntry(archive_entry* entry) {
    archive_entry_set_uid(entry, 0);
    archive_entry_set_gid(entry, 0);
    archive_entry_set_uname(entry, nullptr);
    archive_entry_set_gname(entry, nullptr);
    archive_entry_set_mtime(entry, NormalizedZipTime, 0);
    archive_entry_unset_atime(entry);
    archive_entry_unset_ctime(entry);
    archive_entry_unset_birthtime(entry);

    if (archive_entry_filetype(entry) == AE_IFDIR) {
        archive_entry_set_perm(entry, 0755);
    } else {
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
    }
}

bool writeBytes(archive* writer, const QByteArray& data, QString* error) {
    qsizetype offset = 0;
    while (offset < data.size()) {
        const la_ssize_t written = archive_write_data(
            writer, data.constData() + offset, static_cast<std::size_t>(data.size() - offset));
        if (written <= 0) {
            if (error != nullptr) {
                *error = archiveError(writer, QStringLiteral("Could not write a document part"));
            }
            return false;
        }
        offset += static_cast<qsizetype>(written);
    }
    return true;
}

bool copyCurrentEntry(archive* reader, archive* writer, QString* error) {
    std::array<char, 128 * 1024> buffer{};
    while (true) {
        const la_ssize_t count = archive_read_data(reader, buffer.data(), buffer.size());
        if (count == 0) {
            return true;
        }
        if (count < 0) {
            if (error != nullptr) {
                *error = archiveError(reader, QStringLiteral("Could not read a document part"));
            }
            return false;
        }
        const la_ssize_t written =
            archive_write_data(writer, buffer.data(), static_cast<std::size_t>(count));
        if (written != count) {
            if (error != nullptr) {
                *error = archiveError(writer, QStringLiteral("Could not write a complete document part"));
            }
            return false;
        }
    }
}

} // namespace

QString DocumentEngine::id() const {
    return QStringLiteral("document-libarchive");
}

QString DocumentEngine::displayName() const {
    return QStringLiteral("Office document (libarchive)");
}

QStringList DocumentEngine::supportedSuffixes() const {
    return {QStringLiteral("docx"), QStringLiteral("xlsx"), QStringLiteral("pptx"),
            QStringLiteral("docm"), QStringLiteral("xlsm"), QStringLiteral("pptm"),
            QStringLiteral("odt"),  QStringLiteral("ods"),  QStringLiteral("odp"),
            QStringLiteral("ott"),  QStringLiteral("ots"),  QStringLiteral("otp")};
}

bool DocumentEngine::matches(const QString& suffix, const QString& mimeType) const {
    Q_UNUSED(mimeType)
    return supportedSuffixes().contains(suffix.toLower());
}

InspectionReport DocumentEngine::inspect(const QString& path) const {
    InspectionReport report;
    report.sourcePath = path;
    report.engineId = id();
    report.formatName = QFileInfo(path).suffix().toUpper();

    ReadArchive reader(archive_read_new());
    archive_read_support_filter_all(reader.get());
    archive_read_support_format_zip(reader.get());
    const QByteArray encodedPath = QFile::encodeName(path);
    if (archive_read_open_filename(reader.get(), encodedPath.constData(), 128 * 1024) != ARCHIVE_OK) {
        report.error = archiveError(reader.get(), QStringLiteral("The document container could not be opened"));
        return report;
    }

    int entryCount = 0;
    int timestampCount = 0;
    int ownershipCount = 0;
    qint64 expandedSize = 0;
    archive_entry* rawEntry = nullptr;
    int archiveStatus = ARCHIVE_OK;
    while ((archiveStatus = archive_read_next_header(reader.get(), &rawEntry)) == ARCHIVE_OK) {
        if (++entryCount > MaximumEntryCount) {
            report.error = QStringLiteral("The document contains too many archive entries");
            return report;
        }

        const QString archivePath = entryPath(rawEntry);
        if (!isSafeEntryPath(archivePath) || !isRegularOrDirectory(rawEntry)) {
            report.error = QStringLiteral("The document contains an unsafe archive entry");
            return report;
        }

        const la_int64_t size = archive_entry_size(rawEntry);
        if (size > 0) {
            expandedSize += size;
            if (expandedSize > MaximumExpandedArchiveSize) {
                report.error = QStringLiteral("The expanded document exceeds the safety limit");
                return report;
            }
        }

        if (archive_entry_mtime_is_set(rawEntry) && archive_entry_mtime(rawEntry) != NormalizedZipTime) {
            ++timestampCount;
        }
        if (archive_entry_uid(rawEntry) != 0 || archive_entry_gid(rawEntry) != 0) {
            ++ownershipCount;
        }

        if (OfficeXmlSanitizer::isMetadataEntry(archivePath)) {
            QString readError;
            const QByteArray xml = readCurrentEntry(reader.get(), size, &readError);
            if (!readError.isEmpty()) {
                report.error = readError;
                return report;
            }
            report.entries.append(OfficeXmlSanitizer::inspect(archivePath, xml, &report.warnings));
        } else if (OfficeXmlSanitizer::isThumbnailEntry(archivePath)) {
            const auto classified = RiskClassifier::classify(QStringLiteral("Document thumbnail"));
            report.entries.append({QStringLiteral("Document metadata"),
                                   QStringLiteral("Embedded thumbnail"), archivePath,
                                   classified.reason, classified.level, true, false});
            archive_read_data_skip(reader.get());
        } else {
            archive_read_data_skip(reader.get());
        }
    }

    if (archiveStatus != ARCHIVE_EOF) {
        report.error = archiveError(reader.get(), QStringLiteral("The document archive is truncated or invalid"));
        return report;
    }

    if (timestampCount > 0) {
        const auto classified = RiskClassifier::classify(QStringLiteral("Archive timestamps"));
        report.entries.append({QStringLiteral("Container metadata"),
                               QStringLiteral("Internal file timestamps"),
                               QStringLiteral("Present on %1 entries").arg(timestampCount),
                               classified.reason, classified.level, true, false});
    }
    if (ownershipCount > 0) {
        const auto classified = RiskClassifier::classify(QStringLiteral("Archive owner IDs"));
        report.entries.append({QStringLiteral("Container metadata"),
                               QStringLiteral("Internal owner/group IDs"),
                               QStringLiteral("Present on %1 entries").arg(ownershipCount),
                               classified.reason, classified.level, true, false});
    }

    report.warnings.append(QStringLiteral(
        "Comments, tracked changes, hidden sheets/slides, macros, and visible content are not metadata"));
    report.valid = true;
    report.canSanitize = true;
    archive_read_close(reader.get());
    return report;
}

bool DocumentEngine::sanitize(const QString& sourcePath,
                              const QString& outputPath,
                              const SanitizeOptions& options,
                              QString* error,
                              QStringList* warnings) const {
    Q_UNUSED(options)
    ReadArchive reader(archive_read_new());
    archive_read_support_filter_all(reader.get());
    archive_read_support_format_zip(reader.get());
    const QByteArray sourceName = QFile::encodeName(sourcePath);
    if (archive_read_open_filename(reader.get(), sourceName.constData(), 128 * 1024) != ARCHIVE_OK) {
        if (error != nullptr) {
            *error = archiveError(reader.get(), QStringLiteral("The document container could not be opened"));
        }
        return false;
    }

    WriteArchive writer(archive_write_new());
    archive_write_set_format_zip(writer.get());
    archive_write_add_filter_none(writer.get());
    const QByteArray outputName = QFile::encodeName(outputPath);
    if (archive_write_open_filename(writer.get(), outputName.constData()) != ARCHIVE_OK) {
        if (error != nullptr) {
            *error = archiveError(writer.get(), QStringLiteral("The cleaned document could not be created"));
        }
        return false;
    }

    int entryCount = 0;
    qint64 expandedSize = 0;
    bool removedThumbnail = false;
    archive_entry* rawEntry = nullptr;
    int archiveStatus = ARCHIVE_OK;
    while ((archiveStatus = archive_read_next_header(reader.get(), &rawEntry)) == ARCHIVE_OK) {
        if (++entryCount > MaximumEntryCount) {
            if (error != nullptr) {
                *error = QStringLiteral("The document contains too many archive entries");
            }
            QFile::remove(outputPath);
            return false;
        }

        const QString archivePath = entryPath(rawEntry);
        if (!isSafeEntryPath(archivePath) || !isRegularOrDirectory(rawEntry)) {
            if (error != nullptr) {
                *error = QStringLiteral("The document contains an unsafe archive entry");
            }
            QFile::remove(outputPath);
            return false;
        }

        const la_int64_t size = archive_entry_size(rawEntry);
        if (size > 0) {
            expandedSize += size;
            if (expandedSize > MaximumExpandedArchiveSize) {
                if (error != nullptr) {
                    *error = QStringLiteral("The expanded document exceeds the safety limit");
                }
                QFile::remove(outputPath);
                return false;
            }
        }

        if (OfficeXmlSanitizer::isThumbnailEntry(archivePath)) {
            archive_read_data_skip(reader.get());
            removedThumbnail = true;
            continue;
        }

        ArchiveEntry outputEntry(archive_entry_clone(rawEntry));
        normalizeEntry(outputEntry.get());

        QByteArray replacement;
        if (OfficeXmlSanitizer::isMetadataEntry(archivePath)) {
            replacement = OfficeXmlSanitizer::sanitizedXml(archivePath);
            archive_entry_set_size(outputEntry.get(), replacement.size());
        }

        if (archive_write_header(writer.get(), outputEntry.get()) != ARCHIVE_OK) {
            if (error != nullptr) {
                *error = archiveError(writer.get(), QStringLiteral("Could not write a document header"));
            }
            QFile::remove(outputPath);
            return false;
        }

        const bool wroteData = OfficeXmlSanitizer::isMetadataEntry(archivePath)
                                   ? writeBytes(writer.get(), replacement, error)
                                   : copyCurrentEntry(reader.get(), writer.get(), error);
        if (!wroteData || archive_write_finish_entry(writer.get()) != ARCHIVE_OK) {
            QFile::remove(outputPath);
            return false;
        }
    }

    if (archiveStatus != ARCHIVE_EOF) {
        if (error != nullptr) {
            *error = archiveError(reader.get(), QStringLiteral("The document archive is truncated or invalid"));
        }
        QFile::remove(outputPath);
        return false;
    }

    if (archive_read_close(reader.get()) != ARCHIVE_OK ||
        archive_write_close(writer.get()) != ARCHIVE_OK) {
        if (error != nullptr && error->isEmpty()) {
            *error = QStringLiteral("The cleaned document could not be finalized");
        }
        QFile::remove(outputPath);
        return false;
    }

    if (removedThumbnail && warnings != nullptr) {
        warnings->append(QStringLiteral("Removed the embedded document preview thumbnail"));
    }
    return true;
}

} // namespace metadrop
