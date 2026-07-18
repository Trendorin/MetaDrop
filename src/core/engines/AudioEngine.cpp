#include "core/engines/AudioEngine.h"

#include "core/RiskClassifier.h"

#include <QFile>
#include <QFileDevice>
#include <QFileInfo>

#include <taglib/aifffile.h>
#include <taglib/audioproperties.h>
#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mpegfile.h>
#include <taglib/opusfile.h>
#include <taglib/tfile.h>
#include <taglib/taglib.h>
#include <taglib/tpropertymap.h>
#include <taglib/tstring.h>
#include <taglib/vorbisfile.h>
#include <taglib/wavfile.h>
#include <taglib/xiphcomment.h>

namespace metadrop {
namespace {

QString fromTagString(const TagLib::String& value) {
    constexpr qsizetype MaximumCharacters = 4096;
    QString text = QString::fromUtf8(value.toCString(true));
    text.replace(QChar::Null, QChar::ReplacementCharacter);
    if (text.size() > MaximumCharacters) {
        text = text.left(MaximumCharacters) + QStringLiteral("… [truncated]");
    }
    return text;
}

QString joinedValues(const TagLib::StringList& values) {
    QStringList result;
    result.reserve(static_cast<qsizetype>(values.size()));
    for (const auto& value : values) {
        result.append(fromTagString(value));
    }
    return result.join(QStringLiteral(" · "));
}

void appendProperties(TagLib::File* file, QList<MetadataEntry>* entries) {
    if (file == nullptr) {
        return;
    }

    const TagLib::PropertyMap properties = file->properties();
    for (auto iterator = properties.begin(); iterator != properties.end(); ++iterator) {
        const QString key = fromTagString(iterator->first);
        const QString value = joinedValues(iterator->second);
        const auto classified = RiskClassifier::classify(key, value);
        entries->append({QStringLiteral("Audio tags"), key, value, classified.reason,
                         classified.level, true, false});
    }

    const TagLib::StringList unsupported = properties.unsupportedData();
    for (const auto& property : unsupported) {
        const QString key = fromTagString(property);
        const auto classified = RiskClassifier::classify(key);
        entries->append({QStringLiteral("Audio tags"), key,
                         QStringLiteral("Present (binary or format-specific value)"),
                         classified.reason, classified.level, true, false});
    }
}

void appendFormatSpecific(const QString& path,
                          const QString& suffix,
                          QList<MetadataEntry>* entries) {
    const QByteArray fileName = QFile::encodeName(path);
    qsizetype pictureCount = 0;

    if (suffix == QStringLiteral("flac")) {
        TagLib::FLAC::File file(fileName.constData());
        if (file.isValid()) {
            pictureCount = static_cast<qsizetype>(file.pictureList().size());
        }
    } else if (suffix == QStringLiteral("mp3")) {
        TagLib::MPEG::File file(fileName.constData());
        if (file.isValid() && file.ID3v2Tag(false) != nullptr) {
            pictureCount =
                static_cast<qsizetype>(file.ID3v2Tag(false)->frameList("APIC").size());
        }
    }

    if (pictureCount > 0) {
        const auto classified = RiskClassifier::classify(QStringLiteral("Embedded artwork"));
        entries->append({QStringLiteral("Audio tags"), QStringLiteral("Embedded artwork"),
                         QStringLiteral("%1 image(s)").arg(pictureCount), classified.reason,
                         classified.level, true, false});
    }
}

void appendAudioProperties(TagLib::AudioProperties* properties, QList<MetadataEntry>* entries) {
    if (properties == nullptr) {
        return;
    }
    const int durationSeconds =
#if TAGLIB_MAJOR_VERSION >= 2
        properties->lengthInSeconds();
#else
        properties->length();
#endif
    entries->append({QStringLiteral("Audio stream"), QStringLiteral("Duration"),
                     QStringLiteral("%1 s").arg(durationSeconds), {},
                     RiskLevel::None, false, true});
    entries->append({QStringLiteral("Audio stream"), QStringLiteral("Bitrate"),
                     QStringLiteral("%1 kb/s").arg(properties->bitrate()), {}, RiskLevel::None,
                     false, true});
    entries->append({QStringLiteral("Audio stream"), QStringLiteral("Sample rate"),
                     QStringLiteral("%1 Hz").arg(properties->sampleRate()), {}, RiskLevel::None,
                     false, true});
    entries->append({QStringLiteral("Audio stream"), QStringLiteral("Channels"),
                     QString::number(properties->channels()), {}, RiskLevel::None, false, true});
}

bool copyPrivate(const QString& sourcePath, const QString& outputPath, QString* error) {
    QFile source(sourcePath);
    QFile output(outputPath);
    if (!source.open(QIODevice::ReadOnly) || !output.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error != nullptr) {
            *error = QStringLiteral("Could not copy the media file into the private staging file");
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
                *error = QStringLiteral("Could not copy the complete media file into staging");
            }
            return false;
        }
    }
    output.close();
    return true;
}

void clearXiphComment(TagLib::Ogg::XiphComment* tag) {
    if (tag == nullptr) {
        return;
    }
    TagLib::StringList keys;
    const auto fields = tag->fieldListMap();
    for (auto it = fields.begin(); it != fields.end(); ++it) {
        keys.append(it->first);
    }
    for (const auto& key : keys) {
        tag->removeFields(key);
    }
}

bool sanitizeBySuffix(const QString& path, const QString& suffix) {
    const QByteArray fileName = QFile::encodeName(path);

    if (suffix == QStringLiteral("mp3")) {
        TagLib::MPEG::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        file.strip(TagLib::MPEG::File::AllTags, true);
        return file.save();
    }

    if (suffix == QStringLiteral("flac")) {
        TagLib::FLAC::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        file.removePictures();
        file.strip(TagLib::FLAC::File::AllTags);
        return file.save();
    }

    if (suffix == QStringLiteral("ogg") || suffix == QStringLiteral("oga")) {
        TagLib::Ogg::Vorbis::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        clearXiphComment(file.tag());
        return file.save();
    }

    if (suffix == QStringLiteral("opus")) {
        TagLib::Ogg::Opus::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        clearXiphComment(file.tag());
        return file.save();
    }

    if (suffix == QStringLiteral("m4a") || suffix == QStringLiteral("m4b") ||
        suffix == QStringLiteral("mp4")) {
        TagLib::MP4::File file(fileName.constData());
        if (!file.isValid() || file.tag() == nullptr) {
            return false;
        }
        TagLib::StringList keys;
        const auto items = file.tag()->itemMap();
        for (auto it = items.begin(); it != items.end(); ++it) {
            keys.append(it->first);
        }
        for (const auto& key : keys) {
            file.tag()->removeItem(key);
        }
        return file.save();
    }

    if (suffix == QStringLiteral("wav")) {
        TagLib::RIFF::WAV::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        file.strip(TagLib::RIFF::WAV::File::AllTags);
        return true;
    }

    if (suffix == QStringLiteral("aiff") || suffix == QStringLiteral("aif")) {
        TagLib::RIFF::AIFF::File file(fileName.constData());
        if (!file.isValid()) {
            return false;
        }
        auto* tag = file.tag();
        const auto frames = tag->frameList();
        for (auto* frame : frames) {
            tag->removeFrame(frame, true);
        }
        return file.save();
    }

    return false;
}

} // namespace

QString AudioEngine::id() const {
    return QStringLiteral("audio-taglib");
}

QString AudioEngine::displayName() const {
    return QStringLiteral("Audio (TagLib)");
}

QStringList AudioEngine::supportedSuffixes() const {
    return {QStringLiteral("mp3"),  QStringLiteral("flac"), QStringLiteral("ogg"),
            QStringLiteral("oga"),  QStringLiteral("opus"), QStringLiteral("m4a"),
            QStringLiteral("m4b"),  QStringLiteral("mp4"),  QStringLiteral("wav"),
            QStringLiteral("aiff"), QStringLiteral("aif")};
}

bool AudioEngine::matches(const QString& suffix, const QString& mimeType) const {
    const QString normalized = suffix.toLower();
    return supportedSuffixes().contains(normalized) &&
           (mimeType.startsWith(QStringLiteral("audio/")) ||
            (normalized == QStringLiteral("mp4") && mimeType.startsWith(QStringLiteral("video/"))));
}

InspectionReport AudioEngine::inspect(const QString& path) const {
    InspectionReport report;
    report.sourcePath = path;
    report.engineId = id();
    report.formatName = QFileInfo(path).suffix().toUpper();

    const QByteArray fileName = QFile::encodeName(path);
    TagLib::FileRef file(fileName.constData(), true, TagLib::AudioProperties::Accurate);
    if (file.isNull() || file.file() == nullptr || !file.file()->isValid()) {
        report.error = QStringLiteral("TagLib did not recognize this media file");
        return report;
    }

    appendProperties(file.file(), &report.entries);
    appendAudioProperties(file.audioProperties(), &report.entries);

    const QString suffix = QFileInfo(path).suffix().toLower();
    appendFormatSpecific(path, suffix, &report.entries);
    if (suffix == QStringLiteral("mp4")) {
        report.warnings.append(
            QStringLiteral("Timed metadata tracks and visible content are outside the TagLib cleaning scope"));
    }

    report.valid = true;
    report.canSanitize = true;
    return report;
}

bool AudioEngine::sanitize(const QString& sourcePath,
                           const QString& outputPath,
                           const SanitizeOptions& options,
                           QString* error,
                           QStringList* warnings) const {
    Q_UNUSED(options)
    Q_UNUSED(warnings)
    if (!copyPrivate(sourcePath, outputPath, error)) {
        return false;
    }

    const QString suffix = QFileInfo(sourcePath).suffix().toLower();
    if (!sanitizeBySuffix(outputPath, suffix)) {
        QFile::remove(outputPath);
        if (error != nullptr) {
            *error = QStringLiteral("TagLib could not remove all tags from this media file");
        }
        return false;
    }
    return true;
}

} // namespace metadrop
