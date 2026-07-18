#pragma once

#include "core/AppSettings.h"
#include "core/MetadataService.h"

#include <QMainWindow>
#include <QSet>
#include <QStringList>

#include <memory>

class QAction;
class QLabel;
class QLineEdit;
class QProgressBar;
class QPushButton;
class QSortFilterProxyModel;
class QSystemTrayIcon;
class QTableView;
class QTreeView;

namespace metadrop {

class DropArea;
class FileTableModel;
class MetadataTreeModel;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(AppSettings* settings, QWidget* parent = nullptr);
    ~MainWindow() override;

    void setTrayEnabled(bool enabled);

public slots:
    void addFiles(const QStringList& paths);
    void receiveFilesFromSecondaryInstance(const QStringList& paths);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void browseFiles();
    void cleanSelected();
    void cleanAllReady();
    void removeSelected();
    void showSettings();
    void showAbout();
    void showCurrentDetails();
    void openCurrentLocation();
    void addClipboardFiles();

private:
    void buildUi();
    void buildActions();
    void buildTray();
    void scanPath(const QString& path);
    void cleanPath(const QString& path);
    void updateBusyState(int delta);
    void refreshActions();
    void showAndActivate();
    void notify(const QString& title, const QString& message);
    [[nodiscard]] QStringList selectedPaths() const;
    [[nodiscard]] SanitizeOptions sanitizeOptions() const;

    AppSettings* settings_ = nullptr;
    std::shared_ptr<MetadataService> metadataService_;
    FileTableModel* files_ = nullptr;
    MetadataTreeModel* metadata_ = nullptr;
    QSortFilterProxyModel* metadataFilter_ = nullptr;
    DropArea* dropArea_ = nullptr;
    QTableView* fileTable_ = nullptr;
    QTreeView* metadataTree_ = nullptr;
    QLineEdit* metadataSearch_ = nullptr;
    QLabel* detailsSummary_ = nullptr;
    QLabel* detailsWarning_ = nullptr;
    QLabel* safetyLabel_ = nullptr;
    QProgressBar* progress_ = nullptr;
    QSystemTrayIcon* tray_ = nullptr;
    QAction* cleanSelectedAction_ = nullptr;
    QAction* cleanAllAction_ = nullptr;
    QAction* removeAction_ = nullptr;
    QAction* openLocationAction_ = nullptr;
    QSet<QString> activePaths_;
    int activeJobs_ = 0;
    bool trayEnabled_ = true;
    bool closeNoticeShown_ = false;
};

} // namespace metadrop
