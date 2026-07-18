#pragma once

#include <QString>

namespace metadrop {

struct PathValidation {
    QString canonicalPath;
    QString error;
    bool ok = false;
};

class FileGuard final {
public:
    [[nodiscard]] static PathValidation validateInput(const QString& path);
    [[nodiscard]] static QString validateDestination(const QString& sourcePath,
                                                     const QString& destinationPath);
};

} // namespace metadrop
