#pragma once

#include <QDialog>

namespace metadrop {

class AboutDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

} // namespace metadrop
