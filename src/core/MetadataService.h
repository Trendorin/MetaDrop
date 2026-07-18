#pragma once

#include "core/EngineRegistry.h"
#include "core/MetadataTypes.h"

#include <QString>
#include <QStringList>

namespace metadrop {

class MetadataService final {
public:
    MetadataService();

    [[nodiscard]] InspectionReport inspect(const QString& path) const;
    [[nodiscard]] SanitizeReport sanitize(const InspectionReport& report,
                                          const QString& destinationPath,
                                          const SanitizeOptions& options) const;
    [[nodiscard]] QStringList supportedSuffixes() const;

private:
    [[nodiscard]] InspectionReport inspectWithEngine(const QString& path,
                                                     const MetadataEngine& engine,
                                                     const QString& mimeType) const;
    static void appendTechnicalProperties(InspectionReport* report);

    EngineRegistry registry_;
};

} // namespace metadrop
