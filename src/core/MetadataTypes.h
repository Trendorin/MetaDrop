#pragma once

#include <QDateTime>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

namespace metadrop {
Q_NAMESPACE

enum class RiskLevel { None = 0, Low = 1, Medium = 2, High = 3, Critical = 4 };
Q_ENUM_NS(RiskLevel)

enum class JobState { Pending, Inspecting, Ready, Cleaning, Cleaned, Unsupported, Failed };
Q_ENUM_NS(JobState)

struct MetadataEntry {
    QString group;
    QString key;
    QString value;
    QString privacyReason;
    RiskLevel risk = RiskLevel::None;
    bool removable = false;
    bool technical = false;
};

struct InspectionReport {
    QString sourcePath;
    QString mimeType;
    QString formatName;
    QString engineId;
    QList<MetadataEntry> entries;
    QStringList warnings;
    QString error;
    qint64 fileSize = 0;
    QDateTime modifiedAt;
    bool valid = false;
    bool canSanitize = false;

    [[nodiscard]] int removableCount() const;
    [[nodiscard]] int sensitiveCount() const;
    [[nodiscard]] RiskLevel highestRisk() const;
};

struct SanitizeOptions {
    bool preserveFileTimes = false;
    bool ownerOnlyPermissions = true;
    bool removeColorProfile = false;
};

struct SanitizeReport {
    QString sourcePath;
    QString outputPath;
    InspectionReport before;
    InspectionReport after;
    QStringList warnings;
    QString error;
    int removedCount = 0;
    bool success = false;
    bool verified = false;
};

[[nodiscard]] QString riskLevelName(RiskLevel level);
[[nodiscard]] QString jobStateName(JobState state);

} // namespace metadrop

Q_DECLARE_METATYPE(metadrop::InspectionReport)
Q_DECLARE_METATYPE(metadrop::SanitizeReport)
