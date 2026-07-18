#include "ui/FileTableModel.h"

#include <QFileInfo>
#include <QFont>

#include <algorithm>
#include <functional>

namespace metadrop {

FileTableModel::FileTableModel(QObject* parent) : QAbstractTableModel(parent) {}

int FileTableModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : records_.size();
}

int FileTableModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : ColumnCount;
}

QVariant FileTableModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= records_.size()) {
        return {};
    }
    const FileRecord& record = records_.at(index.row());

    if (role == PathRole) {
        return record.path;
    }
    if (role == StateRole) {
        return static_cast<int>(record.state);
    }
    if (role == ReportRole) {
        return QVariant::fromValue(record.report);
    }
    if (role == OutputPathRole) {
        return record.outputPath;
    }
    if (role == Qt::ToolTipRole) {
        if (!record.error.isEmpty()) {
            return record.error;
        }
        if (!record.outputPath.isEmpty()) {
            return tr("Verified copy: %1").arg(record.outputPath);
        }
        return record.path;
    }
    if (role == Qt::FontRole && index.column() == FileColumn) {
        QFont font;
        font.setBold(true);
        return font;
    }
    if (role != Qt::DisplayRole) {
        return {};
    }

    switch (index.column()) {
    case FileColumn:
        return QFileInfo(record.path).fileName();
    case FormatColumn:
        return record.report.formatName.isEmpty() ? QStringLiteral("—") : record.report.formatName;
    case RiskColumn:
        return record.report.valid ? riskLevelName(record.report.highestRisk()) : QStringLiteral("—");
    case MetadataColumn:
        return record.report.valid ? QString::number(record.report.removableCount())
                                   : QStringLiteral("—");
    case StatusColumn:
        return jobStateName(record.state);
    default:
        return {};
    }
}

QVariant FileTableModel::headerData(const int section,
                                    const Qt::Orientation orientation,
                                    const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractTableModel::headerData(section, orientation, role);
    }
    switch (section) {
    case FileColumn:
        return tr("File");
    case FormatColumn:
        return tr("Format");
    case RiskColumn:
        return tr("Risk");
    case MetadataColumn:
        return tr("Fields");
    case StatusColumn:
        return tr("Status");
    default:
        return {};
    }
}

bool FileTableModel::addPath(const QString& path) {
    if (rowForPath(path) >= 0) {
        return false;
    }
    const int row = records_.size();
    beginInsertRows({}, row, row);
    records_.append({path});
    endInsertRows();
    return true;
}

void FileTableModel::setState(const QString& path, const JobState state, const QString& error) {
    const int row = rowForPath(path);
    if (row < 0) {
        return;
    }
    records_[row].state = state;
    records_[row].error = error;
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

void FileTableModel::setInspection(const QString& path, const InspectionReport& report) {
    const int row = rowForPath(path);
    if (row < 0) {
        return;
    }
    FileRecord& record = records_[row];
    record.report = report;
    record.error = report.error;
    record.state = report.valid && report.canSanitize
                       ? JobState::Ready
                       : (report.error.contains(QStringLiteral("no verified cleaning engine"),
                                                Qt::CaseInsensitive)
                              ? JobState::Unsupported
                              : JobState::Failed);
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

void FileTableModel::setCleaned(const QString& path, const SanitizeReport& report) {
    const int row = rowForPath(path);
    if (row < 0) {
        return;
    }
    FileRecord& record = records_[row];
    record.state = report.success ? JobState::Cleaned : JobState::Failed;
    record.outputPath = report.success ? report.outputPath : QString();
    record.error = report.error;
    if (report.success) {
        record.report = report.before;
    }
    emit dataChanged(index(row, 0), index(row, ColumnCount - 1));
}

void FileTableModel::removeRowsByIndex(QList<int> rows) {
    std::sort(rows.begin(), rows.end(), std::greater<>());
    rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
    for (const int row : rows) {
        if (row < 0 || row >= records_.size()) {
            continue;
        }
        beginRemoveRows({}, row, row);
        records_.removeAt(row);
        endRemoveRows();
    }
}

void FileTableModel::clear() {
    if (records_.isEmpty()) {
        return;
    }
    beginResetModel();
    records_.clear();
    endResetModel();
}

int FileTableModel::rowForPath(const QString& path) const {
    for (int row = 0; row < records_.size(); ++row) {
        if (records_.at(row).path == path) {
            return row;
        }
    }
    return -1;
}

const FileRecord* FileTableModel::recordAt(const int row) const {
    return row >= 0 && row < records_.size() ? &records_.at(row) : nullptr;
}

FileRecord* FileTableModel::recordAt(const int row) {
    return row >= 0 && row < records_.size() ? &records_[row] : nullptr;
}

QList<FileRecord> FileTableModel::records() const {
    return records_;
}

} // namespace metadrop
