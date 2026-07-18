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
    const QString suffix = desired.completeSuffix();
    QString baseName = desired.fileName();
    if (!suffix.isEmpty()) {
        baseName.chop(suffix.size() + 1);
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
