#pragma once

#include "core/AppSettings.h"

#include <QString>

namespace metadrop {

class OutputPlanner final {
public:
    [[nodiscard]] static QString destinationFor(const QString& sourcePath,
                                                const SettingsSnapshot& settings);
    [[nodiscard]] static QString uniquePath(const QString& desiredPath);
};

} // namespace metadrop
