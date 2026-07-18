#include "core/OutputPlanner.h"

#include <QDir>
#include <QFileInfo>

namespace metadrop {

QString OutputPlanner::destinationFor(const QString& sourcePath, const SettingsSnapshot& settings) {
    const QFileInfo source(sourcePath);
    QDir outputDirectory = source.dir();
    if (settings.outputMode == OutputMode::CustomDirectory &&
        !settings.customOutputDirectory.trimmed().isEmpty()) {
        outputDirectory = QDir(settings.customOutputDirectory);
    }

    const QString suffix = source.completeSuffix();
    QString baseName = source.fileName();
    if (!suffix.isEmpty()) {
        baseName.chop(suffix.size() + 1);
    }

    const QString cleanedName = suffix.isEmpty()
                                    ? baseName + QStringLiteral(".cleaned")
                                    : baseName + QStringLiteral(".cleaned.") + suffix;
    return uniquePath(outputDirectory.filePath(cleanedName));
}

QString OutputPlanner::uniquePath(const QString& desiredPath) {
    if (!QFileInfo::exists(desiredPath)) {
        return desiredPath;
    }

    const QFileInfo desired(desiredPath);
    const QString fileName = desired.fileName();
    const QString cleanedMarker = QStringLiteral(".cleaned");
    const qsizetype markerPosition = fileName.lastIndexOf(cleanedMarker);

    QString baseName;
    QString suffix;
    const qsizetype markerEnd = markerPosition + cleanedMarker.size();
    if (markerPosition >= 0 &&
        (markerEnd == fileName.size() || fileName.at(markerEnd) == QLatin1Char('.'))) {
        baseName = fileName.left(markerEnd);
        if (markerEnd < fileName.size()) {
            suffix = fileName.mid(markerEnd + 1);
        }
    } else {
        suffix = desired.completeSuffix();
        baseName = fileName;
        if (!suffix.isEmpty()) {
            baseName.chop(suffix.size() + 1);
        }
    }

    for (int index = 2; index < 10000; ++index) {
        const QString candidateName =
            suffix.isEmpty()
                ? QStringLiteral("%1 (%2)").arg(baseName).arg(index)
                : QStringLiteral("%1 (%2).%3").arg(baseName).arg(index).arg(suffix);
        const QString candidate = desired.dir().filePath(candidateName);
        if (!QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return {};
}

} // namespace metadrop
