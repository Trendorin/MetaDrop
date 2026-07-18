#pragma once

#include "core/MetadataTypes.h"

#include <QString>
#include <QStringList>

namespace metadrop {

class MetadataEngine {
public:
    virtual ~MetadataEngine() = default;

    [[nodiscard]] virtual QString id() const = 0;
    [[nodiscard]] virtual QString displayName() const = 0;
    [[nodiscard]] virtual QStringList supportedSuffixes() const = 0;
    [[nodiscard]] virtual bool matches(const QString& suffix, const QString& mimeType) const = 0;
    [[nodiscard]] virtual InspectionReport inspect(const QString& path) const = 0;

    virtual bool sanitize(const QString& sourcePath,
                          const QString& outputPath,
                          const SanitizeOptions& options,
                          QString* error,
                          QStringList* warnings) const = 0;
};

} // namespace metadrop
