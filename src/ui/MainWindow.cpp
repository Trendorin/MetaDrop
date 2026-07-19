#include "ui/MainWindow.h"

#include "core/FileGuard.h"
#include "core/OutputPlanner.h"
#include "ui/AboutDialog.h"
#include "ui/DropArea.h"
#include "ui/FileTableModel.h"
#include "ui/MetadataTreeModel.h"
#include "ui/SettingsDialog.h"

#include <QAbstractItemView>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFutureWatcher>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QPushButton>
#include <QShortcut>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QStatusBar>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTableView>
#include <QTreeView>
#include <QUrl>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <algorithm>

namespace metadrop {

MainWindow::MainWindow(AppSettings* settings, QWidget* parent)
    : QMainWindow(parent), settings_(settings), metadataService_(std::make_shared<MetadataService>()) {
    Q_ASSERT(settings_ != nullptr);
    buildActions();
    buildUi();
    buildTray();
    refreshActions();
}

MainWindow::~MainWindow() = default;

void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
}

void MainWindow::setTrayEnabled(const bool enabled) {
    trayEnabled_ = enabled && QSystemTrayIcon::isSystemTrayAvailable();
    if (tray_ != nullptr) {
        tray_->setVisible(trayEnabled_);
    }
}

void MainWindow::addFiles(const QStringList& paths) {
    bool addedAny = false;
    for (const QString& rawPath : paths) {
        QFileInfo info(rawPath);
        if (!info.exists() || !info.isFile()) {
            continue;
        }
        const QString path = info.absoluteFilePath();
        if (!files_->addPath(path)) {
            continue;
        }
        addedAny = true;
        scanPath(path);
    }

    if (addedAny && fileTable_->selectionModel()->selectedRows().isEmpty() && files_->rowCount() > 0) {
        fileTable_->selectRow(0);
    }
}

void MainWindow::receiveFilesFromSecondaryInstance(const QStringList& paths) {
    showAndActivate();
    addFiles(paths);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (trayEnabled_ && tray_ != nullptr && tray_->isVisible() &&
        settings_->snapshot().minimizeToTray) {
        hide();
        event->ignore();
        if (!closeNoticeShown_) {
            notify(tr("MetaDrop is still running"),
                   tr("Use the tray icon to reopen or quit MetaDrop."));
            closeNoticeShown_ = true;
        }
        return;
    }
    QMainWindow::closeEvent(event);
}

void MainWindow::browseFiles() {
    QStringList patterns;
    for (const QString& suffix : metadataService_->supportedSuffixes()) {
        patterns.append(QStringLiteral("*.%1").arg(suffix));
    }
    const QString suffixes = patterns.join(QLatin1Char(' '));
    const QString filter = tr("Supported files (%1);;All files (*)").arg(suffixes);
    const QStringList paths = QFileDialog::getOpenFileNames(this, tr("Choose files"), {}, filter);
    addFiles(paths);
}

void MainWindow::cleanSelected() {
    for (const QString& path : selectedPaths()) {
        cleanPath(path);
    }
}

void MainWindow::cleanAllReady() {
    for (const FileRecord& record : files_->records()) {
        if (record.state == JobState::Ready) {
            cleanPath(record.path);
        }
    }
}

void MainWindow::removeSelected() {
    QList<int> rows;
    for (const QModelIndex& index : fileTable_->selectionModel()->selectedRows()) {
        rows.append(index.row());
    }
    files_->removeRowsByIndex(rows);
    if (files_->rowCount() == 0) {
        metadata_->clear();
        detailsSummary_->setText(tr("Select a file to review its metadata."));
        detailsWarning_->clear();
    }
    refreshActions();
}

void MainWindow::showSettings() {
    SettingsDialog dialog(settings_->snapshot(), this);
    if (dialog.exec() == QDialog::Accepted) {
        const SettingsSnapshot value = dialog.settings();
        if (value.outputMode == OutputMode::CustomDirectory &&
            !value.customOutputDirectory.isEmpty() &&
            !QFileInfo(value.customOutputDirectory).isDir()) {
            QMessageBox::warning(this, tr("Invalid output directory"),
                                 tr("Choose an existing output directory."));
            return;
        }
        settings_->apply(value);
    }
}

void MainWindow::showAbout() {
    AboutDialog(this).exec();
}

void MainWindow::showCurrentDetails() {
    const QModelIndex current = fileTable_->currentIndex();
    const FileRecord* record = files_->recordAt(current.row());
    if (record == nullptr) {
        metadata_->clear();
        detailsSummary_->setText(tr("Select a file to review its metadata."));
        detailsWarning_->clear();
        refreshActions();
        return;
    }

    if (!record->report.valid) {
        metadata_->clear();
        detailsSummary_->setText(record->error.isEmpty()
                                     ? tr("Inspection is in progress…")
                                     : localizedMetadataText(record->error));
        detailsWarning_->clear();
        refreshActions();
        return;
    }

    metadata_->setReport(record->report);
    metadataTree_->expandToDepth(0);
    detailsSummary_->setText(
        tr("%1 removable fields · highest risk: %2")
            .arg(record->report.removableCount())
            .arg(riskLevelName(record->report.highestRisk())));

    QStringList notes;
    notes.reserve(record->report.warnings.size() + 1);
    for (const QString& warning : record->report.warnings) {
        notes.append(localizedMetadataText(warning));
    }
    if (!record->outputPath.isEmpty()) {
        notes.prepend(tr("Verified copy: %1").arg(record->outputPath));
    }
    detailsWarning_->setText(notes.join(QStringLiteral("\n")));
    refreshActions();
}

void MainWindow::openCurrentLocation() {
    const FileRecord* record = files_->recordAt(fileTable_->currentIndex().row());
    if (record == nullptr) {
        return;
    }
    const QString path = record->outputPath.isEmpty() ? record->path : record->outputPath;
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
}

void MainWindow::addClipboardFiles() {
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        notify(tr("No files found"), tr("The clipboard does not contain local file URLs."));
        return;
    }
    QStringList paths;
    for (const QUrl& url : mimeData->urls()) {
        if (url.isLocalFile()) {
            paths.append(url.toLocalFile());
        }
    }
    showAndActivate();
    addFiles(paths);
}

void MainWindow::buildUi() {
    setWindowTitle(tr("MetaDrop — Metadata Cleaner"));
    setMinimumSize(780, 560);
    resize(1120, 760);

    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);
    root->setContentsMargins(18, 16, 18, 14);
    root->setSpacing(12);

    auto* header = new QHBoxLayout();
    header->setSpacing(10);
    auto* appIcon = new QLabel(central);
    appIcon->setPixmap(windowIcon().pixmap(36, 36));
    auto* titleColumn = new QVBoxLayout();
    titleColumn->setSpacing(1);
    auto* title = new QLabel(tr("MetaDrop"), central);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() + 2.0);
    title->setFont(titleFont);
    subtitle_ = new QLabel(central);
    subtitle_->setForegroundRole(QPalette::PlaceholderText);
    titleColumn->addWidget(title);
    titleColumn->addWidget(subtitle_);
    header->addWidget(appIcon);
    header->addLayout(titleColumn);
    header->addStretch();
    addButton_ = new QPushButton(central);
    addButton_->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
    settingsButton_ = new QPushButton(central);
    settingsButton_->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    header->addWidget(addButton_);
    header->addWidget(settingsButton_);
    root->addLayout(header);

    dropArea_ = new DropArea(central);
    root->addWidget(dropArea_);

    files_ = new FileTableModel(this);
    metadata_ = new MetadataTreeModel(this);
    metadataFilter_ = new QSortFilterProxyModel(this);
    metadataFilter_->setSourceModel(metadata_);
    metadataFilter_->setRecursiveFilteringEnabled(true);
    metadataFilter_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    metadataFilter_->setFilterKeyColumn(-1);

    auto* splitter = new QSplitter(Qt::Horizontal, central);
    splitter->setChildrenCollapsible(false);

    auto* filePanel = new QWidget(splitter);
    auto* fileLayout = new QVBoxLayout(filePanel);
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->setSpacing(6);
    fileHeading_ = new QLabel(filePanel);
    QFont sectionFont = fileHeading_->font();
    sectionFont.setBold(true);
    fileHeading_->setFont(sectionFont);
    fileLayout->addWidget(fileHeading_);
    fileTable_ = new QTableView(filePanel);
    fileTable_->setModel(files_);
    fileTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    fileTable_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    fileTable_->setAlternatingRowColors(true);
    fileTable_->setSortingEnabled(false);
    fileTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileTable_->verticalHeader()->setVisible(false);
    fileTable_->horizontalHeader()->setStretchLastSection(true);
    fileTable_->horizontalHeader()->setSectionResizeMode(FileTableModel::FileColumn,
                                                         QHeaderView::Stretch);
    fileTable_->horizontalHeader()->setSectionResizeMode(FileTableModel::FormatColumn,
                                                         QHeaderView::ResizeToContents);
    fileTable_->horizontalHeader()->setSectionResizeMode(FileTableModel::RiskColumn,
                                                         QHeaderView::ResizeToContents);
    fileTable_->horizontalHeader()->setSectionResizeMode(FileTableModel::MetadataColumn,
                                                         QHeaderView::ResizeToContents);
    fileTable_->horizontalHeader()->setSectionResizeMode(FileTableModel::StatusColumn,
                                                         QHeaderView::ResizeToContents);
    fileLayout->addWidget(fileTable_, 1);

    auto* fileButtons = new QHBoxLayout();
    removeButton_ = new QPushButton(filePanel);
    cleanSelectedButton_ = new QPushButton(filePanel);
    cleanAllButton_ = new QPushButton(filePanel);
    fileButtons->addWidget(removeButton_);
    fileButtons->addStretch();
    fileButtons->addWidget(cleanSelectedButton_);
    fileButtons->addWidget(cleanAllButton_);
    fileLayout->addLayout(fileButtons);

    auto* metadataPanel = new QWidget(splitter);
    auto* metadataLayout = new QVBoxLayout(metadataPanel);
    metadataLayout->setContentsMargins(0, 0, 0, 0);
    metadataLayout->setSpacing(6);
    detailsSummary_ = new QLabel(tr("Select a file to review its metadata."), metadataPanel);
    detailsSummary_->setFont(sectionFont);
    detailsSummary_->setWordWrap(true);
    metadataLayout->addWidget(detailsSummary_);
    metadataSearch_ = new QLineEdit(metadataPanel);
    metadataSearch_->setPlaceholderText(tr("Filter metadata…"));
    metadataSearch_->setClearButtonEnabled(true);
    metadataSearch_->setAccessibleName(tr("Metadata filter"));
    metadataLayout->addWidget(metadataSearch_);
    metadataTree_ = new QTreeView(metadataPanel);
    metadataTree_->setModel(metadataFilter_);
    metadataTree_->setAlternatingRowColors(true);
    metadataTree_->setUniformRowHeights(true);
    metadataTree_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    metadataTree_->header()->setStretchLastSection(false);
    metadataTree_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    metadataTree_->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    metadataTree_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    metadataLayout->addWidget(metadataTree_, 1);
    detailsWarning_ = new QLabel(metadataPanel);
    detailsWarning_->setWordWrap(true);
    detailsWarning_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    detailsWarning_->setForegroundRole(QPalette::PlaceholderText);
    metadataLayout->addWidget(detailsWarning_);

    splitter->addWidget(filePanel);
    splitter->addWidget(metadataPanel);
    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 6);
    splitter->setSizes({500, 600});
    root->addWidget(splitter, 1);

    safetyLabel_ = new QLabel(
        tr("Source files are never overwritten. A cleaned copy is saved only after verification."),
        central);
    safetyLabel_->setWordWrap(true);
    safetyLabel_->setForegroundRole(QPalette::PlaceholderText);
    root->addWidget(safetyLabel_);

    setCentralWidget(central);
    progress_ = new QProgressBar(this);
    progress_->setRange(0, 0);
    progress_->setMaximumWidth(140);
    progress_->setTextVisible(false);
    progress_->hide();
    statusBar()->addPermanentWidget(progress_);
    retranslateUi();

    connect(addButton_, &QPushButton::clicked, this, &MainWindow::browseFiles);
    connect(settingsButton_, &QPushButton::clicked, this, &MainWindow::showSettings);
    connect(removeButton_, &QPushButton::clicked, removeAction_, &QAction::trigger);
    connect(cleanSelectedButton_, &QPushButton::clicked, cleanSelectedAction_, &QAction::trigger);
    connect(cleanAllButton_, &QPushButton::clicked, cleanAllAction_, &QAction::trigger);
    connect(removeAction_, &QAction::changed, removeButton_,
            [this] { removeButton_->setEnabled(removeAction_->isEnabled()); });
    connect(cleanSelectedAction_, &QAction::changed, cleanSelectedButton_,
            [this] {
                cleanSelectedButton_->setEnabled(cleanSelectedAction_->isEnabled());
            });
    connect(cleanAllAction_, &QAction::changed, cleanAllButton_,
            [this] { cleanAllButton_->setEnabled(cleanAllAction_->isEnabled()); });
    connect(dropArea_, &DropArea::filesDropped, this, &MainWindow::addFiles);
    connect(dropArea_, &DropArea::browseRequested, this, &MainWindow::browseFiles);
    connect(metadataSearch_, &QLineEdit::textChanged, metadataFilter_,
            &QSortFilterProxyModel::setFilterFixedString);
    connect(fileTable_->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { showCurrentDetails(); });
    connect(fileTable_->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this] { showCurrentDetails(); });
    connect(fileTable_, &QTableView::doubleClicked, this, &MainWindow::openCurrentLocation);
}

void MainWindow::buildActions() {
    addAction_ = new QAction(this);
    addAction_->setShortcut(QKeySequence::Open);
    connect(addAction_, &QAction::triggered, this, &MainWindow::browseFiles);

    cleanSelectedAction_ = new QAction(this);
    cleanSelectedAction_->setShortcut(QKeySequence(QStringLiteral("Ctrl+Return")));
    connect(cleanSelectedAction_, &QAction::triggered, this, &MainWindow::cleanSelected);

    cleanAllAction_ = new QAction(this);
    connect(cleanAllAction_, &QAction::triggered, this, &MainWindow::cleanAllReady);

    removeAction_ = new QAction(this);
    removeAction_->setShortcut(QKeySequence::Delete);
    connect(removeAction_, &QAction::triggered, this, &MainWindow::removeSelected);

    openLocationAction_ = new QAction(this);
    connect(openLocationAction_, &QAction::triggered, this, &MainWindow::openCurrentLocation);

    settingsAction_ = new QAction(this);
    settingsAction_->setShortcut(QKeySequence::Preferences);
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::showSettings);

    quitAction_ = new QAction(this);
    quitAction_->setShortcut(QKeySequence::Quit);
    connect(quitAction_, &QAction::triggered, qApp, &QApplication::quit);

    aboutAction_ = new QAction(this);
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::showAbout);

    fileMenu_ = menuBar()->addMenu(QString());
    fileMenu_->addAction(addAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(cleanSelectedAction_);
    fileMenu_->addAction(cleanAllAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(openLocationAction_);
    fileMenu_->addSeparator();
    fileMenu_->addAction(quitAction_);

    editMenu_ = menuBar()->addMenu(QString());
    editMenu_->addAction(removeAction_);
    editMenu_->addSeparator();
    editMenu_->addAction(settingsAction_);

    helpMenu_ = menuBar()->addMenu(QString());
    helpMenu_->addAction(aboutAction_);
    retranslateUi();
}

void MainWindow::buildTray() {
    tray_ = new QSystemTrayIcon(windowIcon(), this);
    auto* menu = new QMenu(this);
    trayShowAction_ = menu->addAction(QString());
    trayAddAction_ = menu->addAction(QString());
    trayClipboardAction_ = menu->addAction(QString());
    menu->addSeparator();
    trayQuitAction_ = menu->addAction(QString());
    tray_->setContextMenu(menu);

    connect(trayShowAction_, &QAction::triggered, this, &MainWindow::showAndActivate);
    connect(trayAddAction_, &QAction::triggered, this, [this] {
        showAndActivate();
        browseFiles();
    });
    connect(trayClipboardAction_, &QAction::triggered, this, &MainWindow::addClipboardFiles);
    connect(trayQuitAction_, &QAction::triggered, qApp, &QApplication::quit);
    connect(tray_, &QSystemTrayIcon::activated, this,
            [this](const QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
                    showAndActivate();
                }
            });

    setTrayEnabled(true);
    retranslateUi();
}

void MainWindow::scanPath(const QString& path) {
    if (activePaths_.contains(path)) {
        return;
    }
    activePaths_.insert(path);
    files_->setState(path, JobState::Inspecting);
    updateBusyState(1);

    auto* watcher = new QFutureWatcher<InspectionReport>(this);
    connect(watcher, &QFutureWatcher<InspectionReport>::finished, this, [this, watcher, path] {
        const InspectionReport report = watcher->result();
        activePaths_.remove(path);
        files_->setInspection(path, report);
        updateBusyState(-1);
        watcher->deleteLater();

        if (files_->recordAt(fileTable_->currentIndex().row()) != nullptr &&
            files_->recordAt(fileTable_->currentIndex().row())->path == path) {
            showCurrentDetails();
        }
        if (report.valid && report.canSanitize && settings_->snapshot().cleanImmediately) {
            cleanPath(path);
        }
        refreshActions();
    });
    watcher->setFuture(QtConcurrent::run([service = metadataService_, path] {
        return service->inspect(path);
    }));
}

void MainWindow::cleanPath(const QString& path) {
    if (activePaths_.contains(path)) {
        return;
    }
    const int row = files_->rowForPath(path);
    const FileRecord* record = files_->recordAt(row);
    if (record == nullptr || record->state != JobState::Ready || !record->report.canSanitize) {
        return;
    }

    const SettingsSnapshot settings = settings_->snapshot();
    if (settings.outputMode == OutputMode::CustomDirectory &&
        !QFileInfo(settings.customOutputDirectory).isDir()) {
        QMessageBox::warning(this, tr("Output directory unavailable"),
                             tr("Choose an existing output directory in Settings."));
        return;
    }

    const QString destination = OutputPlanner::destinationFor(path, settings);
    if (destination.isEmpty()) {
        QMessageBox::warning(this, tr("Could not choose an output name"),
                             tr("Too many cleaned copies already exist for this file."));
        return;
    }

    const InspectionReport inspection = record->report;
    activePaths_.insert(path);
    files_->setState(path, JobState::Cleaning);
    updateBusyState(1);
    refreshActions();

    auto* watcher = new QFutureWatcher<SanitizeReport>(this);
    connect(watcher, &QFutureWatcher<SanitizeReport>::finished, this,
            [this, watcher, path] {
                const SanitizeReport report = watcher->result();
                activePaths_.remove(path);
                files_->setCleaned(path, report);
                updateBusyState(-1);
                watcher->deleteLater();

                if (report.success) {
                    statusBar()->showMessage(
                        tr("Verified cleaned copy saved to %1").arg(report.outputPath), 8000);
                    notify(tr("Metadata removed"),
                           tr("Saved a verified copy of %1")
                               .arg(QFileInfo(report.sourcePath).fileName()));
                } else {
                    const QString error = localizedMetadataText(report.error);
                    statusBar()->showMessage(tr("Cleaning failed: %1").arg(error), 10000);
                    notify(tr("Cleaning failed"), error);
                }
                showCurrentDetails();
                refreshActions();
            });
    watcher->setFuture(QtConcurrent::run(
        [service = metadataService_, inspection, destination, options = sanitizeOptions()] {
            return service->sanitize(inspection, destination, options);
        }));
}

void MainWindow::updateBusyState(const int delta) {
    activeJobs_ = std::max(0, activeJobs_ + delta);
    progress_->setVisible(activeJobs_ > 0);
    if (activeJobs_ > 0) {
        statusBar()->showMessage(tr("Processing %1 file(s)…").arg(activeJobs_));
    } else {
        statusBar()->showMessage(tr("Ready"), 3000);
    }
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("MetaDrop — Metadata Cleaner"));
    if (subtitle_ != nullptr) {
        subtitle_->setText(tr("Inspect first. Remove locally. Verify before saving."));
    }
    if (addButton_ != nullptr) {
        addButton_->setText(tr("Add files…"));
        settingsButton_->setText(tr("Settings"));
        fileHeading_->setText(tr("Files"));
        removeButton_->setText(tr("Remove"));
        cleanSelectedButton_->setText(tr("Clean selected"));
        cleanAllButton_->setText(tr("Clean all ready"));
        metadataSearch_->setPlaceholderText(tr("Filter metadata…"));
        metadataSearch_->setAccessibleName(tr("Metadata filter"));
        safetyLabel_->setText(
            tr("Source files are never overwritten. A cleaned copy is saved only after verification."));
    }
    if (addAction_ != nullptr) {
        addAction_->setText(tr("Add files…"));
        cleanSelectedAction_->setText(tr("Clean selected"));
        cleanAllAction_->setText(tr("Clean all ready files"));
        removeAction_->setText(tr("Remove from list"));
        openLocationAction_->setText(tr("Open containing folder"));
        settingsAction_->setText(tr("Settings…"));
        quitAction_->setText(tr("Quit"));
        aboutAction_->setText(tr("About MetaDrop"));
        fileMenu_->setTitle(tr("&File"));
        editMenu_->setTitle(tr("&Edit"));
        helpMenu_->setTitle(tr("&Help"));
    }
    if (tray_ != nullptr) {
        tray_->setToolTip(tr("MetaDrop — local metadata cleaner"));
        trayShowAction_->setText(tr("Open MetaDrop"));
        trayAddAction_->setText(tr("Add files…"));
        trayClipboardAction_->setText(tr("Inspect files from clipboard"));
        trayQuitAction_->setText(tr("Quit"));
    }
    if (files_ != nullptr) {
        files_->retranslate();
        metadata_->retranslate();
        showCurrentDetails();
    }
    if (progress_ != nullptr) {
        updateBusyState(0);
    }
}

void MainWindow::refreshActions() {
    bool selectedReady = false;
    for (const QString& path : selectedPaths()) {
        const FileRecord* record = files_->recordAt(files_->rowForPath(path));
        selectedReady = selectedReady || (record != nullptr && record->state == JobState::Ready);
    }
    const QList<FileRecord> records = files_->records();
    const bool anyReady = std::any_of(records.cbegin(), records.cend(), [](const FileRecord& record) {
        return record.state == JobState::Ready;
    });
    const bool hasSelection = !selectedPaths().isEmpty();
    cleanSelectedAction_->setEnabled(selectedReady);
    cleanAllAction_->setEnabled(anyReady);
    removeAction_->setEnabled(hasSelection);
    openLocationAction_->setEnabled(hasSelection);
}

void MainWindow::showAndActivate() {
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::notify(const QString& title, const QString& message) {
    if (trayEnabled_ && tray_ != nullptr && tray_->isVisible() &&
        settings_->snapshot().showNotifications) {
        tray_->showMessage(title, message, QSystemTrayIcon::Information, 5000);
    }
}

QStringList MainWindow::selectedPaths() const {
    QStringList paths;
    if (fileTable_ == nullptr || fileTable_->selectionModel() == nullptr) {
        return paths;
    }
    for (const QModelIndex& index : fileTable_->selectionModel()->selectedRows()) {
        const FileRecord* record = files_->recordAt(index.row());
        if (record != nullptr) {
            paths.append(record->path);
        }
    }
    return paths;
}

SanitizeOptions MainWindow::sanitizeOptions() const {
    const SettingsSnapshot settings = settings_->snapshot();
    return {settings.preserveFileTimes, settings.ownerOnlyPermissions,
            settings.removeColorProfile};
}

} // namespace metadrop
