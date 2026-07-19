#include "core/MetadataTypes.h"

#include <QCoreApplication>

#include <algorithm>

namespace metadrop {
namespace {

[[maybe_unused]] constexpr const char* kMetadataTextSources[] = {
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "File"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "MIME type"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Size"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Image"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Comment"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "ICC color profile"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Dimensions"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Audio tags"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Audio stream"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Embedded artwork"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Duration"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Bitrate"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Sample rate"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Channels"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Document metadata"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Container metadata"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Embedded thumbnail"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Internal file timestamps"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Internal owner/group IDs"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "PDF metadata"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "PDF document info"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "PDF trailer"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "File identifier"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "XMP metadata stream"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Page metadata"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "May reveal a precise or approximate location"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "May identify the person, account, or device that created the file"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "A persistent identifier can link this file to other files or devices"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Timing or edit-history data can help correlate activity"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Software or hardware details can strengthen a fingerprint"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Descriptive data may expose unintended context"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Embedded metadata not required to render the file"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "This format has no verified cleaning engine"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The selected file does not have a verified cleaning engine"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The cleaning engine failed"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The cleaned copy could not be reopened for verification"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Verification found metadata that should have been removed"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The original file timestamps could not be restored"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The input path is invalid"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The file does not exist"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Symbolic links are refused to avoid cleaning an unintended file"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "Only regular files are supported"),
    QT_TRANSLATE_NOOP("metadrop::MetadataText", "The file is not readable"),
};

} // namespace

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
        return QCoreApplication::translate("metadrop::MetadataTypes", "None");
    case RiskLevel::Low:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Low");
    case RiskLevel::Medium:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Medium");
    case RiskLevel::High:
        return QCoreApplication::translate("metadrop::MetadataTypes", "High");
    case RiskLevel::Critical:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Critical");
    }
    return QCoreApplication::translate("metadrop::MetadataTypes", "Unknown");
}

QString jobStateName(const JobState state) {
    switch (state) {
    case JobState::Pending:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Pending");
    case JobState::Inspecting:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Inspecting");
    case JobState::Ready:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Ready");
    case JobState::Cleaning:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Cleaning");
    case JobState::Cleaned:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Cleaned");
    case JobState::Unsupported:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Unsupported");
    case JobState::Failed:
        return QCoreApplication::translate("metadrop::MetadataTypes", "Failed");
    }
    return QCoreApplication::translate("metadrop::MetadataTypes", "Unknown");
}

QString localizedMetadataText(const QString& source) {
    const QByteArray utf8 = source.toUtf8();
    return QCoreApplication::translate("metadrop::MetadataText", utf8.constData());
}

} // namespace metadrop
