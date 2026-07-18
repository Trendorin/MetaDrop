#include "core/OfficeXmlSanitizer.h"

#include "core/RiskClassifier.h"

#include <QBuffer>
#include <QXmlStreamReader>

namespace metadrop {
namespace {

QString normalizedPath(const QString& path) {
    QString result = path;
    result.replace(QLatin1Char('\\'), QLatin1Char('/'));
    return result.toLower();
}

QString bounded(const QString& value) {
    constexpr qsizetype MaximumCharacters = 4096;
    if (value.size() <= MaximumCharacters) {
        return value;
    }
    return value.left(MaximumCharacters) + QStringLiteral("… [truncated]");
}

bool shouldReportElement(const QString& archivePath, const QString& localName) {
    const QString path = normalizedPath(archivePath);
    const QString name = localName.toLower();

    if (path == QStringLiteral("docprops/core.xml")) {
        return name != QStringLiteral("coreproperties");
    }
    if (path == QStringLiteral("docprops/app.xml")) {
        return name != QStringLiteral("properties") && name != QStringLiteral("headingpairs") &&
               name != QStringLiteral("titlesofparts");
    }
    if (path == QStringLiteral("docprops/custom.xml")) {
        return name != QStringLiteral("properties") && name != QStringLiteral("property");
    }
    if (path == QStringLiteral("meta.xml")) {
        return name != QStringLiteral("document-meta") && name != QStringLiteral("meta");
    }
    return false;
}

} // namespace

bool OfficeXmlSanitizer::isMetadataEntry(const QString& archivePath) {
    const QString path = normalizedPath(archivePath);
    return path == QStringLiteral("docprops/core.xml") ||
           path == QStringLiteral("docprops/app.xml") ||
           path == QStringLiteral("docprops/custom.xml") || path == QStringLiteral("meta.xml");
}

bool OfficeXmlSanitizer::isThumbnailEntry(const QString& archivePath) {
    return normalizedPath(archivePath).startsWith(QStringLiteral("docprops/thumbnail."));
}

QByteArray OfficeXmlSanitizer::sanitizedXml(const QString& archivePath) {
    const QString path = normalizedPath(archivePath);
    if (path == QStringLiteral("docprops/core.xml")) {
        return QByteArrayLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<cp:coreProperties "
            "xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
            "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
            "xmlns:dcterms=\"http://purl.org/dc/terms/\" "
            "xmlns:dcmitype=\"http://purl.org/dc/dcmitype/\" "
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"/>");
    }
    if (path == QStringLiteral("docprops/app.xml")) {
        return QByteArrayLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<Properties "
            "xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/extended-properties\" "
            "xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\"/>");
    }
    if (path == QStringLiteral("docprops/custom.xml")) {
        return QByteArrayLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            "<Properties "
            "xmlns=\"http://schemas.openxmlformats.org/officeDocument/2006/custom-properties\" "
            "xmlns:vt=\"http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes\"/>");
    }
    if (path == QStringLiteral("meta.xml")) {
        return QByteArrayLiteral(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<office:document-meta "
            "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" "
            "xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\" "
            "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" office:version=\"1.3\">"
            "<office:meta/></office:document-meta>");
    }
    return {};
}

QList<MetadataEntry> OfficeXmlSanitizer::inspect(const QString& archivePath,
                                                 const QByteArray& xml,
                                                 QStringList* warnings) {
    QList<MetadataEntry> entries;
    QXmlStreamReader reader(xml);
    QString customPropertyName;

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isStartElement()) {
            continue;
        }

        const QString localName = reader.name().toString();
        if (localName.compare(QStringLiteral("property"), Qt::CaseInsensitive) == 0) {
            customPropertyName = reader.attributes().value(QStringLiteral("name")).toString();
            continue;
        }

        if (localName.compare(QStringLiteral("document-statistic"), Qt::CaseInsensitive) == 0) {
            QStringList statistics;
            for (const auto& attribute : reader.attributes()) {
                statistics.append(attribute.name().toString() + QLatin1Char('=') +
                                  attribute.value().toString());
            }
            if (!statistics.isEmpty()) {
                const auto classified = RiskClassifier::classify(localName, statistics.join(u", "));
                entries.append({QStringLiteral("Document metadata"), localName,
                                bounded(statistics.join(u", ")), classified.reason,
                                classified.level, true, false});
            }
            continue;
        }

        if (!shouldReportElement(archivePath, localName)) {
            continue;
        }

        QString value = reader.readElementText(QXmlStreamReader::SkipChildElements).trimmed();
        if (value.isEmpty()) {
            continue;
        }

        const QString key = customPropertyName.isEmpty()
                                ? localName
                                : QStringLiteral("%1 (%2)").arg(customPropertyName, localName);
        const auto classified = RiskClassifier::classify(key, value);
        entries.append({QStringLiteral("Document metadata"), key, bounded(value), classified.reason,
                        classified.level, true, false});
    }

    if (reader.hasError() && warnings != nullptr) {
        warnings->append(QStringLiteral("Could not parse %1 completely: %2")
                             .arg(archivePath, reader.errorString()));
    }
    return entries;
}

} // namespace metadrop
