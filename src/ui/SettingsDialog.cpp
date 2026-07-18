#include "ui/SettingsDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace metadrop {

SettingsDialog::SettingsDialog(const SettingsSnapshot& settings, QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Settings"));
    setModal(true);
    resize(560, 460);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(18, 18, 18, 18);
    root->setSpacing(14);

    auto* outputGroup = new QGroupBox(tr("Output"), this);
    auto* outputForm = new QFormLayout(outputGroup);
    outputMode_ = new QComboBox(outputGroup);
    outputMode_->addItem(tr("Create a cleaned copy next to the source"),
                         static_cast<int>(OutputMode::NextToSource));
    outputMode_->addItem(tr("Save cleaned copies in a directory"),
                         static_cast<int>(OutputMode::CustomDirectory));
    outputMode_->setCurrentIndex(
        outputMode_->findData(static_cast<int>(settings.outputMode)));
    outputForm->addRow(tr("Save mode:"), outputMode_);

    auto* directoryRow = new QWidget(outputGroup);
    auto* directoryLayout = new QHBoxLayout(directoryRow);
    directoryLayout->setContentsMargins(0, 0, 0, 0);
    outputDirectory_ = new QLineEdit(settings.customOutputDirectory, directoryRow);
    outputDirectory_->setPlaceholderText(tr("Choose an output directory"));
    browseOutput_ = new QPushButton(tr("Browse…"), directoryRow);
    directoryLayout->addWidget(outputDirectory_, 1);
    directoryLayout->addWidget(browseOutput_);
    outputForm->addRow(tr("Directory:"), directoryRow);
    root->addWidget(outputGroup);

    auto* privacyGroup = new QGroupBox(tr("Privacy and cleaning"), this);
    auto* privacyLayout = new QVBoxLayout(privacyGroup);
    cleanImmediately_ =
        new QCheckBox(tr("Clean automatically after inspection (always creates a copy)"), privacyGroup);
    cleanImmediately_->setChecked(settings.cleanImmediately);
    preserveFileTimes_ = new QCheckBox(tr("Preserve source file-system timestamps"), privacyGroup);
    preserveFileTimes_->setChecked(settings.preserveFileTimes);
    preserveFileTimes_->setToolTip(
        tr("Disabled by default because timestamps can help correlate files"));
    ownerOnlyPermissions_ =
        new QCheckBox(tr("Make cleaned copies readable only by my user"), privacyGroup);
    ownerOnlyPermissions_->setChecked(settings.ownerOnlyPermissions);
    removeColorProfile_ =
        new QCheckBox(tr("Remove image ICC color profiles (may change appearance)"), privacyGroup);
    removeColorProfile_->setChecked(settings.removeColorProfile);
    privacyLayout->addWidget(cleanImmediately_);
    privacyLayout->addWidget(preserveFileTimes_);
    privacyLayout->addWidget(ownerOnlyPermissions_);
    privacyLayout->addWidget(removeColorProfile_);
    root->addWidget(privacyGroup);

    auto* desktopGroup = new QGroupBox(tr("Desktop integration"), this);
    auto* desktopForm = new QFormLayout(desktopGroup);
    minimizeToTray_ = new QCheckBox(tr("Keep MetaDrop in the tray when the window closes"), desktopGroup);
    minimizeToTray_->setChecked(settings.minimizeToTray);
    showNotifications_ = new QCheckBox(tr("Show completion notifications"), desktopGroup);
    showNotifications_->setChecked(settings.showNotifications);
    language_ = new QComboBox(desktopGroup);
    language_->addItem(tr("System language"), QStringLiteral("system"));
    language_->addItem(QStringLiteral("English"), QStringLiteral("en"));
    language_->addItem(QStringLiteral("Русский"), QStringLiteral("ru"));
    language_->setCurrentIndex(language_->findData(settings.language));
    desktopForm->addRow(minimizeToTray_);
    desktopForm->addRow(showNotifications_);
    desktopForm->addRow(tr("Language:"), language_);
    root->addWidget(desktopGroup);

    auto* languageHint = new QLabel(tr("Language changes take effect after restarting MetaDrop."), this);
    languageHint->setWordWrap(true);
    languageHint->setForegroundRole(QPalette::PlaceholderText);
    root->addWidget(languageHint);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, this);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(browseOutput_, &QPushButton::clicked, this, &SettingsDialog::browseOutputDirectory);
    connect(outputMode_, &QComboBox::currentIndexChanged, this,
            &SettingsDialog::updateOutputControls);
    updateOutputControls();
}

SettingsSnapshot SettingsDialog::settings() const {
    SettingsSnapshot result;
    result.outputMode = static_cast<OutputMode>(outputMode_->currentData().toInt());
    result.customOutputDirectory = outputDirectory_->text().trimmed();
    result.cleanImmediately = cleanImmediately_->isChecked();
    result.preserveFileTimes = preserveFileTimes_->isChecked();
    result.ownerOnlyPermissions = ownerOnlyPermissions_->isChecked();
    result.removeColorProfile = removeColorProfile_->isChecked();
    result.minimizeToTray = minimizeToTray_->isChecked();
    result.showNotifications = showNotifications_->isChecked();
    result.language = language_->currentData().toString();
    return result;
}

void SettingsDialog::browseOutputDirectory() {
    const QString directory = QFileDialog::getExistingDirectory(
        this, tr("Choose output directory"), outputDirectory_->text());
    if (!directory.isEmpty()) {
        outputDirectory_->setText(directory);
    }
}

void SettingsDialog::updateOutputControls() {
    const bool custom = static_cast<OutputMode>(outputMode_->currentData().toInt()) ==
                        OutputMode::CustomDirectory;
    outputDirectory_->setEnabled(custom);
    browseOutput_->setEnabled(custom);
}

} // namespace metadrop
