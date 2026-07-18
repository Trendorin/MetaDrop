#pragma once

#include "core/AppSettings.h"

#include <QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;

namespace metadrop {

class SettingsDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(const SettingsSnapshot& settings, QWidget* parent = nullptr);
    [[nodiscard]] SettingsSnapshot settings() const;

private slots:
    void browseOutputDirectory();
    void updateOutputControls();

private:
    QComboBox* outputMode_ = nullptr;
    QLineEdit* outputDirectory_ = nullptr;
    QPushButton* browseOutput_ = nullptr;
    QCheckBox* cleanImmediately_ = nullptr;
    QCheckBox* preserveFileTimes_ = nullptr;
    QCheckBox* ownerOnlyPermissions_ = nullptr;
    QCheckBox* removeColorProfile_ = nullptr;
    QCheckBox* minimizeToTray_ = nullptr;
    QCheckBox* showNotifications_ = nullptr;
    QComboBox* language_ = nullptr;
};

} // namespace metadrop
