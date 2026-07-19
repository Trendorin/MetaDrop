#pragma once

#include <QFrame>
#include <QPalette>
#include <QStringList>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QKeyEvent;
class QLabel;
class QMimeData;
class QMouseEvent;
class QEvent;

namespace metadrop {

class DropArea final : public QFrame {
    Q_OBJECT

public:
    explicit DropArea(QWidget* parent = nullptr);

signals:
    void filesDropped(const QStringList& files);
    void browseRequested();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void setHighlighted(bool highlighted);
    void retranslateUi();
    [[nodiscard]] static QStringList localFiles(const QMimeData* mimeData);

    QPalette normalPalette_;
    QLabel* title_ = nullptr;
    QLabel* subtitle_ = nullptr;
};

} // namespace metadrop
