#include "ui/MetadataTreeModel.h"

#include <QFont>
#include <QMap>

#include <algorithm>

namespace metadrop {

MetadataTreeModel::MetadataTreeModel(QObject* parent)
    : QAbstractItemModel(parent), root_(std::make_unique<Node>()) {}

MetadataTreeModel::~MetadataTreeModel() = default;

QModelIndex MetadataTreeModel::index(const int row,
                                     const int column,
                                     const QModelIndex& parentIndex) const {
    if (row < 0 || column < 0 || column >= 3) {
        return {};
    }
    Node* parentNode = parentIndex.isValid() ? static_cast<Node*>(parentIndex.internalPointer())
                                             : root_.get();
    if (parentNode == nullptr || row >= static_cast<int>(parentNode->children.size())) {
        return {};
    }
    return createIndex(row, column, parentNode->children.at(static_cast<std::size_t>(row)).get());
}

QModelIndex MetadataTreeModel::parent(const QModelIndex& child) const {
    if (!child.isValid()) {
        return {};
    }
    auto* node = static_cast<Node*>(child.internalPointer());
    if (node == nullptr || node->parent == root_.get() || node->parent == nullptr) {
        return {};
    }
    return createIndex(childRow(node->parent), 0, node->parent);
}

int MetadataTreeModel::rowCount(const QModelIndex& parentIndex) const {
    if (parentIndex.column() > 0) {
        return 0;
    }
    const Node* node = parentIndex.isValid() ? static_cast<Node*>(parentIndex.internalPointer())
                                             : root_.get();
    return node == nullptr ? 0 : static_cast<int>(node->children.size());
}

int MetadataTreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent)
    return 3;
}

QVariant MetadataTreeModel::data(const QModelIndex& index, const int role) const {
    if (!index.isValid()) {
        return {};
    }
    const auto* node = static_cast<Node*>(index.internalPointer());
    if (node == nullptr) {
        return {};
    }
    if (role == Qt::ToolTipRole) {
        return node->toolTip;
    }
    if (role == Qt::FontRole && node->group) {
        QFont font;
        font.setBold(true);
        return font;
    }
    if (role != Qt::DisplayRole) {
        return {};
    }
    switch (index.column()) {
    case 0:
        return node->key;
    case 1:
        return node->value;
    case 2:
        return node->risk;
    default:
        return {};
    }
}

QVariant MetadataTreeModel::headerData(const int section,
                                       const Qt::Orientation orientation,
                                       const int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }
    switch (section) {
    case 0:
        return tr("Metadata");
    case 1:
        return tr("Value");
    case 2:
        return tr("Risk");
    default:
        return {};
    }
}

void MetadataTreeModel::setReport(const InspectionReport& report) {
    beginResetModel();
    root_ = std::make_unique<Node>();

    QMap<QString, QList<MetadataEntry>> grouped;
    for (const auto& entry : report.entries) {
        grouped[entry.group].append(entry);
    }

    for (auto iterator = grouped.cbegin(); iterator != grouped.cend(); ++iterator) {
        auto group = std::make_unique<Node>();
        group->parent = root_.get();
        group->key = iterator.key();
        group->value = tr("%1 fields").arg(iterator.value().size());
        group->group = true;

        QList<MetadataEntry> entries = iterator.value();
        std::sort(entries.begin(), entries.end(), [](const auto& left, const auto& right) {
            if (left.risk != right.risk) {
                return left.risk > right.risk;
            }
            return left.key.localeAwareCompare(right.key) < 0;
        });

        for (const auto& entry : entries) {
            auto child = std::make_unique<Node>();
            child->parent = group.get();
            child->key = entry.key;
            child->value = entry.value;
            child->risk = entry.technical ? tr("Technical") : riskLevelName(entry.risk);
            child->toolTip = entry.privacyReason.isEmpty()
                                 ? entry.value
                                 : entry.privacyReason + QStringLiteral("\n\n") + entry.value;
            group->children.push_back(std::move(child));
        }
        root_->children.push_back(std::move(group));
    }
    endResetModel();
}

void MetadataTreeModel::clear() {
    beginResetModel();
    root_ = std::make_unique<Node>();
    endResetModel();
}

int MetadataTreeModel::childRow(const Node* node) {
    if (node == nullptr || node->parent == nullptr) {
        return 0;
    }
    const auto& siblings = node->parent->children;
    for (std::size_t index = 0; index < siblings.size(); ++index) {
        if (siblings[index].get() == node) {
            return static_cast<int>(index);
        }
    }
    return 0;
}

} // namespace metadrop
