#include "core/engines/ImageEngine.h"

#include "core/RiskClassifier.h"

#include <QFile>
#include <QFileDevice>

#include <exiv2/exiv2.hpp>

#include <exception>
#include <string>

namespace metadrop {
namespace {

QString boundedUtf8(const std::string& value, const std::size_t rawSize) {
    constexpr qsizetype MaximumCharacters = 4096;
    QString text = QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size()));
    text.replace(QChar::Null, QChar::ReplacementCharacter);
    if (text.size() > MaximumCharacters) {
        text = text.left(MaximumCharacters) + QStringLiteral("… [truncated, %1 bytes]").arg(rawSize);
    }
    return text;
}

std::size_t iccProfileSize(Exiv2::Image& image) {
#if EXIV2_TEST_VERSION(0, 28, 0)
    return image.iccProfile().size();
#else
    const auto* profile = image.iccProfile();
    return profile != nullptr && profile->size_ > 0
               ? static_cast<std::size_t>(profile->size_)
               : 0U;
#endif
}

template <typename Container>
void appendContainer(const Container& container,
                     const QString& group,
                     QList<MetadataEntry>* entries) {
    for (const auto& datum : container) {
        QString value;
        try {
            value = boundedUtf8(datum.toString(), datum.size());
        } catch (const std::exception&) {
            value = QStringLiteral("Binary value (%1 bytes)").arg(datum.size());
        }

        const QString key = QString::fromStdString(datum.key());
        const QString label = QString::fromStdString(datum.tagLabel());
        const auto classified = RiskClassifier::classify(key + QLatin1Char(' ') + label, value);
        entries->append({group, label.isEmpty() ? key : label, value, classified.reason,
                         classified.level, true, false});
    }
}

bool copyPrivate(const QString& sourcePath, const QString& outputPath, QString* error) {
    QFile source(sourcePath);
    QFile output(outputPath);
    if (!source.open(QIODevice::ReadOnly) || !output.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error != nullptr) {
            *error = QStringLiteral("Could not copy the image into the private staging file");
        }
        return false;
    }
    QFile::setPermissions(outputPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    QByteArray buffer;
    buffer.resize(1024 * 1024);
    while (true) {
        const qint64 read = source.read(buffer.data(), buffer.size());
        if (read == 0) {
            break;
        }
        if (read < 0 || output.write(buffer.constData(), read) != read) {
            output.close();
            QFile::remove(outputPath);
            if (error != nullptr) {
                *error = QStringLiteral("Could not copy the complete image into staging");
            }
            return false;
        }
    }
    output.close();
    return true;
}

} // namespace

QString ImageEngine::id() const {
    return QStringLiteral("image-exiv2");
}

QString ImageEngine::displayName() const {
    return QStringLiteral("Image (Exiv2)");
}

QStringList ImageEngine::supportedSuffixes() const {
    return {QStringLiteral("jpg"),  QStringLiteral("jpeg"), QStringLiteral("jpe"),
            QStringLiteral("png"),  QStringLiteral("webp"), QStringLiteral("tif"),
            QStringLiteral("tiff"), QStringLiteral("dng"),  QStringLiteral("heic"),
            QStringLiteral("heif"), QStringLiteral("avif")};
}

bool ImageEngine::matches(const QString& suffix, const QString& mimeType) const {
    const QString normalized = suffix.toLower();
    return supportedSuffixes().contains(normalized) && mimeType.startsWith(QStringLiteral("image/"));
}

InspectionReport ImageEngine::inspect(const QString& path) const {
    InspectionReport report;
    report.sourcePath = path;
    report.engineId = id();
    report.formatName = QStringLiteral("Image");

    try {
        auto image = Exiv2::ImageFactory::open(path.toStdString());
        if (image.get() == nullptr) {
            report.error = QStringLiteral("Exiv2 did not recognize this image");
            return report;
        }

        image->readMetadata();
        appendContainer(image->exifData(), QStringLiteral("EXIF"), &report.entries);
        appendContainer(image->iptcData(), QStringLiteral("IPTC"), &report.entries);
        appendContainer(image->xmpData(), QStringLiteral("XMP"), &report.entries);

        const std::string comment = image->comment();
        if (!comment.empty()) {
            const QString value = boundedUtf8(comment, comment.size());
            const auto classified = RiskClassifier::classify(QStringLiteral("Image comment"), value);
            report.entries.append({QStringLiteral("Image"), QStringLiteral("Comment"), value,
                                   classified.reason, classified.level, true, false});
        }

        if (image->iccProfileDefined()) {
            report.entries.append(
                {QStringLiteral("Image"), QStringLiteral("ICC color profile"),
                 QStringLiteral("%1 bytes").arg(iccProfileSize(*image)),
                 QStringLiteral("A color profile affects rendering and is preserved by default"),
                 RiskLevel::Low, false, true});
        }

        if (image->pixelWidth() > 0 && image->pixelHeight() > 0) {
            report.entries.append({QStringLiteral("Image"), QStringLiteral("Dimensions"),
                                   QStringLiteral("%1 × %2")
                                       .arg(image->pixelWidth())
                                       .arg(image->pixelHeight()),
                                   {}, RiskLevel::None, false, true});
        }

        report.formatName = QString::fromStdString(image->mimeType());
        report.valid = true;
        report.canSanitize = true;
    } catch (const Exiv2::Error& exception) {
        report.error = QStringLiteral("Image metadata could not be read: %1")
                           .arg(QString::fromUtf8(exception.what()));
    } catch (const std::exception& exception) {
        report.error = QStringLiteral("Image inspection failed: %1")
                           .arg(QString::fromUtf8(exception.what()));
    }
    return report;
}

bool ImageEngine::sanitize(const QString& sourcePath,
                           const QString& outputPath,
                           const SanitizeOptions& options,
                           QString* error,
                           QStringList* warnings) const {
    Q_UNUSED(warnings)
    if (!copyPrivate(sourcePath, outputPath, error)) {
        return false;
    }

    try {
        auto image = Exiv2::ImageFactory::open(outputPath.toStdString());
        if (image.get() == nullptr) {
            if (error != nullptr) {
                *error = QStringLiteral("The copied image could not be reopened");
            }
            return false;
        }

        image->readMetadata();
        image->clearExifData();
        image->clearIptcData();
        image->clearXmpData();
        image->clearXmpPacket();
        image->clearComment();
        if (options.removeColorProfile) {
            image->clearIccProfile();
        }
        image->writeMetadata();
        return true;
    } catch (const Exiv2::Error& exception) {
        QFile::remove(outputPath);
        if (error != nullptr) {
            *error = QStringLiteral("Image metadata could not be removed: %1")
                         .arg(QString::fromUtf8(exception.what()));
        }
    } catch (const std::exception& exception) {
        QFile::remove(outputPath);
        if (error != nullptr) {
            *error = QStringLiteral("Image cleaning failed: %1")
                         .arg(QString::fromUtf8(exception.what()));
        }
    }
    return false;
}

} // namespace metadrop
