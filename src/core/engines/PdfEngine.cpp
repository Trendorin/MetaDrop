#include "core/engines/PdfEngine.h"

#include "core/RiskClassifier.h"

#include <QFile>

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFWriter.hh>

#include <algorithm>
#include <exception>
#include <string>

namespace metadrop {
namespace {

QString pdfValue(const QPDFObjectHandle& object) {
    try {
        const std::string raw = object.isString() ? object.getUTF8Value() : object.unparse();
        constexpr qsizetype MaximumCharacters = 4096;
        QString value = QString::fromUtf8(raw.data(), static_cast<qsizetype>(raw.size()));
        if (value.size() > MaximumCharacters) {
            value = value.left(MaximumCharacters) + QStringLiteral("… [truncated]");
        }
        return value;
    } catch (const std::exception&) {
        return QStringLiteral("Present (value could not be decoded safely)");
    }
}

void inspectDictionary(const QPDFObjectHandle& dictionary,
                       const QString& group,
                       QList<MetadataEntry>* entries) {
    if (!dictionary.isDictionary()) {
        return;
    }
    for (const auto& rawKey : dictionary.getKeys()) {
        const QString key = QString::fromStdString(rawKey).remove(QLatin1Char('/'));
        const QString value = pdfValue(dictionary.getKey(rawKey));
        const auto classified = RiskClassifier::classify(key, value);
        entries->append(
            {group, key, value, classified.reason, classified.level, true, false});
    }
}

void appendPresence(const QString& key,
                    const QString& description,
                    QList<MetadataEntry>* entries) {
    const auto classified = RiskClassifier::classify(key);
    entries->append({QStringLiteral("PDF metadata"), key, description, classified.reason,
                     classified.level, true, false});
}

void removePageMetadata(QPDF& pdf) {
    for (auto& page : pdf.getAllPages()) {
        page.removeKey("/Metadata");
        page.removeKey("/PieceInfo");
    }
}

} // namespace

QString PdfEngine::id() const {
    return QStringLiteral("pdf-qpdf");
}

QString PdfEngine::displayName() const {
    return QStringLiteral("PDF (qpdf)");
}

QStringList PdfEngine::supportedSuffixes() const {
    return {QStringLiteral("pdf")};
}

bool PdfEngine::matches(const QString& suffix, const QString& mimeType) const {
    return suffix.compare(QStringLiteral("pdf"), Qt::CaseInsensitive) == 0 &&
           mimeType == QStringLiteral("application/pdf");
}

InspectionReport PdfEngine::inspect(const QString& path) const {
    InspectionReport report;
    report.sourcePath = path;
    report.engineId = id();
    report.formatName = QStringLiteral("PDF");

    try {
        QPDF pdf;
        pdf.processFile(QFile::encodeName(path).constData());

        const QPDFObjectHandle trailer = pdf.getTrailer();
        if (trailer.hasKey("/Info")) {
            inspectDictionary(trailer.getKey("/Info"), QStringLiteral("PDF document info"),
                              &report.entries);
        }

        const QPDFObjectHandle root = pdf.getRoot();
        if (root.hasKey("/Metadata")) {
            appendPresence(QStringLiteral("XMP metadata stream"),
                           QStringLiteral("Embedded XML metadata stream"), &report.entries);
        }
        if (root.hasKey("/PieceInfo")) {
            appendPresence(QStringLiteral("PieceInfo"),
                           QStringLiteral("Application-specific editing metadata"),
                           &report.entries);
        }
        if (root.hasKey("/SpiderInfo")) {
            appendPresence(QStringLiteral("SpiderInfo"),
                           QStringLiteral("Web-capture metadata"), &report.entries);
        }

        int pageMetadataStreams = 0;
        for (const auto& page : pdf.getAllPages()) {
            if (page.hasKey("/Metadata") || page.hasKey("/PieceInfo")) {
                ++pageMetadataStreams;
            }
        }
        if (pageMetadataStreams > 0) {
            appendPresence(QStringLiteral("Page metadata"),
                           QStringLiteral("Found on %1 page(s)").arg(pageMetadataStreams),
                           &report.entries);
        }

        report.warnings.append(QStringLiteral(
            "Annotations, form values, attachments, and visible page content are not treated as metadata"));
        report.valid = true;
        report.canSanitize = true;
    } catch (const std::exception& exception) {
        report.error =
            QStringLiteral("PDF inspection failed: %1").arg(QString::fromUtf8(exception.what()));
    }
    return report;
}

bool PdfEngine::sanitize(const QString& sourcePath,
                         const QString& outputPath,
                         const SanitizeOptions& options,
                         QString* error,
                         QStringList* warnings) const {
    Q_UNUSED(options)
    Q_UNUSED(warnings)
    try {
        QPDF pdf;
        pdf.processFile(QFile::encodeName(sourcePath).constData());

        QPDFObjectHandle trailer = pdf.getTrailer();
        trailer.removeKey("/Info");

        QPDFObjectHandle root = pdf.getRoot();
        root.removeKey("/Metadata");
        root.removeKey("/PieceInfo");
        root.removeKey("/SpiderInfo");
        removePageMetadata(pdf);

        QPDFWriter writer(pdf, QFile::encodeName(outputPath).constData());
        writer.setObjectStreamMode(qpdf_o_generate);
        writer.setCompressStreams(true);
        writer.write();
        return true;
    } catch (const std::exception& exception) {
        QFile::remove(outputPath);
        if (error != nullptr) {
            *error =
                QStringLiteral("PDF metadata could not be removed: %1")
                    .arg(QString::fromUtf8(exception.what()));
        }
        return false;
    }
}

} // namespace metadrop
