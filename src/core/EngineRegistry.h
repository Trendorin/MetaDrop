#pragma once

#include "core/MetadataEngine.h"

#include <memory>
#include <vector>

namespace metadrop {

class EngineRegistry final {
public:
    EngineRegistry();

    [[nodiscard]] const MetadataEngine* find(const QString& suffix,
                                             const QString& mimeType) const;
    [[nodiscard]] const MetadataEngine* byId(const QString& id) const;
    [[nodiscard]] QStringList supportedSuffixes() const;

private:
    std::vector<std::unique_ptr<MetadataEngine>> engines_;
};

} // namespace metadrop
