#include "core/EngineRegistry.h"

#include "core/engines/AudioEngine.h"
#include "core/engines/DocumentEngine.h"
#include "core/engines/ImageEngine.h"
#include "core/engines/PdfEngine.h"

#include <algorithm>

namespace metadrop {

EngineRegistry::EngineRegistry() {
    engines_.emplace_back(std::make_unique<ImageEngine>());
    engines_.emplace_back(std::make_unique<AudioEngine>());
    engines_.emplace_back(std::make_unique<PdfEngine>());
    engines_.emplace_back(std::make_unique<DocumentEngine>());
}

const MetadataEngine* EngineRegistry::find(const QString& suffix, const QString& mimeType) const {
    const auto match = std::find_if(engines_.cbegin(), engines_.cend(), [&](const auto& engine) {
        return engine->matches(suffix, mimeType);
    });
    return match == engines_.cend() ? nullptr : match->get();
}

const MetadataEngine* EngineRegistry::byId(const QString& id) const {
    const auto match = std::find_if(engines_.cbegin(), engines_.cend(), [&](const auto& engine) {
        return engine->id() == id;
    });
    return match == engines_.cend() ? nullptr : match->get();
}

QStringList EngineRegistry::supportedSuffixes() const {
    QStringList suffixes;
    for (const auto& engine : engines_) {
        suffixes.append(engine->supportedSuffixes());
    }
    suffixes.removeDuplicates();
    suffixes.sort(Qt::CaseInsensitive);
    return suffixes;
}

} // namespace metadrop
