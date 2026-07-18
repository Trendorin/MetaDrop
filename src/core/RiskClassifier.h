#pragma once

#include "core/MetadataTypes.h"

#include <QString>

namespace metadrop {

struct ClassifiedRisk {
    RiskLevel level = RiskLevel::Low;
    QString reason;
};

class RiskClassifier final {
public:
    [[nodiscard]] static ClassifiedRisk classify(const QString& key, const QString& value = {});
};

} // namespace metadrop
