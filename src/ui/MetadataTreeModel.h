#pragma once

#include "core/MetadataTypes.h"

#include <QAbstractItemModel>

#include <memory>
#include <vector>

namespace metadrop {

class MetadataTreeModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    explicit MetadataTreeModel(QObject* parent = nullptr);
    ~MetadataTreeModel() override;

    [[nodiscard]] QModelIndex index(int row,
                                    int column,
                                    const QModelIndex& parent = {}) const override;
    [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section,
                                      Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const override;

    void setReport(const InspectionReport& report);
    void clear();
    void retranslate();

private:
    struct Node {
        Node* parent = nullptr;
        QString key;
        QString value;
        QString risk;
        QString toolTip;
        bool group = false;
        std::vector<std::unique_ptr<Node>> children;
    };

    [[nodiscard]] static int childRow(const Node* node);
    void rebuild();
    std::unique_ptr<Node> root_;
    InspectionReport report_;
    bool hasReport_ = false;
};

} // namespace metadrop
