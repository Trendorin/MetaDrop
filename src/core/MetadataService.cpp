#include "core/MetadataService.h"

#include "core/FileGuard.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTemporaryFile>

namespace metadrop {
namespace {

QString stagingTemplateFor(const QString& destinationPath) {
    const QFileInfo destination(destinationPath);
    const QString suffix = destination.completeSuffix();
    const QString tail = suffix.isEmpty() ? QString() : QLatin1Char('.') + suffix;
    return destination.dir().filePath(QStringLiteral(".metadrop-XXXXXX") + tail);
}

void removeQuietly(const QString& path) {
    if (!path.isEmpty()) {
        QFile::remove(path);
    }
}

} // namespace

MetadataService::MetadataService() = default;

InspectionReport MetadataService::inspect(const QString& path) const {
    const PathValidation validation = FileGuard::validateInput(path);
    if (!validation.ok) {
        InspectionReport report;
        report.sourcePath = path;
        report.error = validation.error;
        return report;
    }

    QMimeDatabase database;
    const QString mimeType =
        database.mimeTypeForFile(validation.canonicalPath, QMimeDatabase::MatchContent).name();
    const QString suffix = QFileInfo(validation.canonicalPath).suffix().toLower();
    const MetadataEngine* engine = registry_.find(suffix, mimeType);
    if (engine == nullptr) {
        InspectionReport report;
        report.sourcePath = validation.canonicalPath;
        report.mimeType = mimeType;
        report.fileSize = QFileInfo(validation.canonicalPath).size();
        report.error = QStringLiteral("This format has no verified cleaning engine");
        return report;
    }
    return inspectWithEngine(validation.canonicalPath, *engine, mimeType);
}

InspectionReport MetadataService::inspectWithEngine(const QString& path,
                                                    const MetadataEngine& engine,
                                                    const QString& mimeType) const {
    InspectionReport report = engine.inspect(path);
    report.sourcePath = path;
    report.engineId = engine.id();
    report.mimeType = mimeType;
    const QFileInfo info(path);
    report.fileSize = info.size();
    report.modifiedAt = info.lastModified();
    appendTechnicalProperties(&report);
    return report;
}

SanitizeReport MetadataService::sanitize(const InspectionReport& report,
                                         const QString& destinationPath,
                                         const SanitizeOptions& options) const {
    SanitizeReport result;
    result.sourcePath = report.sourcePath;
    result.outputPath = destinationPath;
    result.before = report;

    const PathValidation sourceValidation = FileGuard::validateInput(report.sourcePath);
    if (!sourceValidation.ok) {
        result.error = sourceValidation.error;
        return result;
    }

    const QString destinationError =
        FileGuard::validateDestination(sourceValidation.canonicalPath, destinationPath);
    if (!destinationError.isEmpty()) {
        result.error = destinationError;
        return result;
    }

    const MetadataEngine* engine = registry_.byId(report.engineId);
    if (engine == nullptr || !report.canSanitize) {
        result.error = QStringLiteral("The selected file does not have a verified cleaning engine");
        return result;
    }

    QTemporaryFile staging(stagingTemplateFor(destinationPath));
    staging.setAutoRemove(false);
    if (!staging.open()) {
        result.error = QStringLiteral("Could not create a private staging file in the output directory");
        return result;
    }
    const QString stagingPath = staging.fileName();
    staging.close();
    QFile::setPermissions(stagingPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);

    QString engineError;
    if (!engine->sanitize(sourceValidation.canonicalPath, stagingPath, options, &engineError,
                          &result.warnings)) {
        removeQuietly(stagingPath);
        result.error = engineError.isEmpty() ? QStringLiteral("The cleaning engine failed") : engineError;
        return result;
    }

    QMimeDatabase database;
    const QString mimeType =
        database.mimeTypeForFile(stagingPath, QMimeDatabase::MatchContent).name();
    InspectionReport verification = inspectWithEngine(stagingPath, *engine, mimeType);
    if (!verification.valid) {
        removeQuietly(stagingPath);
        result.error = QStringLiteral("The cleaned copy could not be reopened for verification");
        result.warnings.append(verification.warnings);
        return result;
    }
    if (verification.removableCount() != 0) {
        removeQuietly(stagingPath);
        result.error = QStringLiteral("Verification found metadata that should have been removed");
        result.after = verification;
        return result;
    }

    if (options.ownerOnlyPermissions) {
        QFile::setPermissions(stagingPath, QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    } else {
        const QFileDevice::Permissions sourcePermissions = QFileInfo(sourceValidation.canonicalPath).permissions();
        const QFileDevice::Permissions safePermissions =
            sourcePermissions & (QFileDevice::ReadOwner | QFileDevice::WriteOwner |
                                 QFileDevice::ReadGroup | QFileDevice::WriteGroup |
                                 QFileDevice::ReadOther | QFileDevice::WriteOther);
        QFile::setPermissions(stagingPath, safePermissions);
    }

    if (!QFile::rename(stagingPath, destinationPath)) {
        removeQuietly(stagingPath);
        result.error = QStringLiteral("Could not atomically move the verified copy into place");
        return result;
    }

    if (options.preserveFileTimes) {
        QFile output(destinationPath);
        if (output.open(QIODevice::ReadWrite)) {
            const QFileInfo sourceInfo(sourceValidation.canonicalPath);
            output.setFileTime(sourceInfo.fileTime(QFileDevice::FileModificationTime),
                               QFileDevice::FileModificationTime);
            output.setFileTime(sourceInfo.fileTime(QFileDevice::FileAccessTime),
                               QFileDevice::FileAccessTime);
            output.close();
        } else {
            result.warnings.append(QStringLiteral("The original file timestamps could not be restored"));
        }
    }

    verification.sourcePath = destinationPath;
    result.after = verification;
    result.removedCount = report.removableCount();
    result.verified = true;
    result.success = true;
    return result;
}

QStringList MetadataService::supportedSuffixes() const {
    return registry_.supportedSuffixes();
}

void MetadataService::appendTechnicalProperties(InspectionReport* report) {
    if (report == nullptr || report->sourcePath.isEmpty()) {
        return;
    }

    const QFileInfo info(report->sourcePath);
    report->entries.prepend({QStringLiteral("File"), QStringLiteral("MIME type"),
                             report->mimeType, {}, RiskLevel::None, false, true});
    report->entries.prepend({QStringLiteral("File"), QStringLiteral("Size"),
                             QString::number(info.size()), {}, RiskLevel::None, false, true});
}

} // namespace metadrop
