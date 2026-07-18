#pragma once

#include "core/MetadataTypes.h"

#include <QObject>
#include <QString>

namespace metadrop {

enum class OutputMode { NextToSource = 0, CustomDirectory = 1 };

struct SettingsSnapshot {
    OutputMode outputMode = OutputMode::NextToSource;
    QString customOutputDirectory;
    bool cleanImmediately = false;
    bool preserveFileTimes = false;
    bool ownerOnlyPermissions = true;
    bool removeColorProfile = false;
    bool minimizeToTray = true;
    bool showNotifications = true;
    QString language = QStringLiteral("system");
};

class AppSettings final : public QObject {
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = nullptr);

    [[nodiscard]] SettingsSnapshot snapshot() const;
    void apply(const SettingsSnapshot& settings);

signals:
    void changed();

private:
    static constexpr auto Group = "General";
};

} // namespace metadrop
