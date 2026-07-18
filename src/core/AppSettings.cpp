#include "core/AppSettings.h"

#include <QSettings>

namespace metadrop {

AppSettings::AppSettings(QObject* parent) : QObject(parent) {}

SettingsSnapshot AppSettings::snapshot() const {
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(Group));

    SettingsSnapshot result;
    result.outputMode = static_cast<OutputMode>(
        settings.value(QStringLiteral("outputMode"), static_cast<int>(OutputMode::NextToSource))
            .toInt());
    result.customOutputDirectory = settings.value(QStringLiteral("customOutputDirectory")).toString();
    result.cleanImmediately = settings.value(QStringLiteral("cleanImmediately"), false).toBool();
    result.preserveFileTimes = settings.value(QStringLiteral("preserveFileTimes"), false).toBool();
    result.ownerOnlyPermissions =
        settings.value(QStringLiteral("ownerOnlyPermissions"), true).toBool();
    result.removeColorProfile = settings.value(QStringLiteral("removeColorProfile"), false).toBool();
    result.minimizeToTray = settings.value(QStringLiteral("minimizeToTray"), true).toBool();
    result.showNotifications = settings.value(QStringLiteral("showNotifications"), true).toBool();
    result.language = settings.value(QStringLiteral("language"), QStringLiteral("system")).toString();
    return result;
}

void AppSettings::apply(const SettingsSnapshot& value) {
    QSettings settings;
    settings.beginGroup(QString::fromLatin1(Group));
    settings.setValue(QStringLiteral("outputMode"), static_cast<int>(value.outputMode));
    settings.setValue(QStringLiteral("customOutputDirectory"), value.customOutputDirectory);
    settings.setValue(QStringLiteral("cleanImmediately"), value.cleanImmediately);
    settings.setValue(QStringLiteral("preserveFileTimes"), value.preserveFileTimes);
    settings.setValue(QStringLiteral("ownerOnlyPermissions"), value.ownerOnlyPermissions);
    settings.setValue(QStringLiteral("removeColorProfile"), value.removeColorProfile);
    settings.setValue(QStringLiteral("minimizeToTray"), value.minimizeToTray);
    settings.setValue(QStringLiteral("showNotifications"), value.showNotifications);
    settings.setValue(QStringLiteral("language"), value.language);
    settings.endGroup();
    settings.sync();
    emit changed();
}

} // namespace metadrop
