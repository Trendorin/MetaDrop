#include "core/FileGuard.h"

#include <QDir>
#include <QFileInfo>

namespace metadrop {

PathValidation FileGuard::validateInput(const QString& path) {
    if (path.isEmpty() || path.contains(QChar::Null)) {
        return {{}, QStringLiteral("The input path is invalid"), false};
    }

    const QFileInfo info(path);
    if (!info.exists()) {
        return {{}, QStringLiteral("The file does not exist"), false};
    }
    if (info.isSymLink()) {
        return {{}, QStringLiteral("Symbolic links are refused to avoid cleaning an unintended file"),
                false};
    }
    if (!info.isFile()) {
        return {{}, QStringLiteral("Only regular files are supported"), false};
    }
    if (!info.isReadable()) {
        return {{}, QStringLiteral("The file is not readable"), false};
    }

    const QString canonical = info.canonicalFilePath();
    if (canonical.isEmpty()) {
        return {{}, QStringLiteral("The canonical file path could not be resolved"), false};
    }
    return {canonical, {}, true};
}

QString FileGuard::validateDestination(const QString& sourcePath, const QString& destinationPath) {
    if (destinationPath.isEmpty() || destinationPath.contains(QChar::Null)) {
        return QStringLiteral("The output path is invalid");
    }

    const QFileInfo destination(destinationPath);
    if (destination.exists()) {
        return QStringLiteral("The output path already exists");
    }

    const QFileInfo source(sourcePath);
    if (QDir::cleanPath(source.absoluteFilePath()) ==
        QDir::cleanPath(destination.absoluteFilePath())) {
        return QStringLiteral("MetaDrop never overwrites the source file");
    }

    const QFileInfo parent(destination.dir().absolutePath());
    if (!parent.exists() || !parent.isDir()) {
        return QStringLiteral("The output directory does not exist");
    }
    if (!parent.isWritable()) {
        return QStringLiteral("The output directory is not writable");
    }
    return {};
}

} // namespace metadrop
