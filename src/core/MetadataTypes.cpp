#include "core/MetadataTypes.h"

#include <algorithm>

namespace metadrop {

int InspectionReport::removableCount() const {
    return static_cast<int>(
        std::count_if(entries.cbegin(), entries.cend(), [](const MetadataEntry& entry) {
            return entry.removable;
        }));
}

int InspectionReport::sensitiveCount() const {
    return static_cast<int>(
        std::count_if(entries.cbegin(), entries.cend(), [](const MetadataEntry& entry) {
            return entry.removable && entry.risk >= RiskLevel::Medium;
        }));
}

RiskLevel InspectionReport::highestRisk() const {
    RiskLevel highest = RiskLevel::None;
    for (const auto& entry : entries) {
        if (entry.removable && entry.risk > highest) {
            highest = entry.risk;
        }
    }
    return highest;
}

QString riskLevelName(const RiskLevel level) {
    switch (level) {
    case RiskLevel::None:
        return QStringLiteral("None");
    case RiskLevel::Low:
        return QStringLiteral("Low");
    case RiskLevel::Medium:
        return QStringLiteral("Medium");
    case RiskLevel::High:
        return QStringLiteral("High");
    case RiskLevel::Critical:
        return QStringLiteral("Critical");
    }
    return QStringLiteral("Unknown");
}

QString jobStateName(const JobState state) {
    switch (state) {
    case JobState::Pending:
        return QStringLiteral("Pending");
    case JobState::Inspecting:
        return QStringLiteral("Inspecting");
    case JobState::Ready:
        return QStringLiteral("Ready");
    case JobState::Cleaning:
        return QStringLiteral("Cleaning");
    case JobState::Cleaned:
        return QStringLiteral("Cleaned");
    case JobState::Unsupported:
        return QStringLiteral("Unsupported");
    case JobState::Failed:
        return QStringLiteral("Failed");
    }
    return QStringLiteral("Unknown");
}

} // namespace metadrop
