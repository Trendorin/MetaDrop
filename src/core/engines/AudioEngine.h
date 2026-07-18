#pragma once

#include "core/MetadataEngine.h"

namespace metadrop {

class AudioEngine final : public MetadataEngine {
public:
    [[nodiscard]] QString id() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList supportedSuffixes() const override;
    [[nodiscard]] bool matches(const QString& suffix, const QString& mimeType) const override;
    [[nodiscard]] InspectionReport inspect(const QString& path) const override;
    bool sanitize(const QString& sourcePath,
                  const QString& outputPath,
                  const SanitizeOptions& options,
                  QString* error,
                  QStringList* warnings) const override;
};

} // namespace metadrop
