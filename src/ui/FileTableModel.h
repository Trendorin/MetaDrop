#pragma once

#include "core/MetadataTypes.h"

#include <QAbstractTableModel>
#include <QList>
#include <QString>

namespace metadrop {

struct FileRecord {
    QString path;
    QString outputPath;
    QString error;
    InspectionReport report;
    JobState state = JobState::Pending;
};

class FileTableModel final : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column { FileColumn, FormatColumn, RiskColumn, MetadataColumn, StatusColumn, ColumnCount };
    enum Role { PathRole = Qt::UserRole + 1, StateRole, ReportRole, OutputPathRole };

    explicit FileTableModel(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section,
                                      Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    [[nodiscard]] bool addPath(const QString& path);
    void setState(const QString& path, JobState state, const QString& error = {});
    void setInspection(const QString& path, const InspectionReport& report);
    void setCleaned(const QString& path, const SanitizeReport& report);
    void removeRowsByIndex(QList<int> rows);
    void clear();

    [[nodiscard]] int rowForPath(const QString& path) const;
    [[nodiscard]] const FileRecord* recordAt(int row) const;
    [[nodiscard]] FileRecord* recordAt(int row);
    [[nodiscard]] QList<FileRecord> records() const;

private:
    QList<FileRecord> records_;
};

} // namespace metadrop
