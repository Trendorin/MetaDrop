#pragma once

#include <QFrame>
#include <QPalette>
#include <QStringList>

class QDragEnterEvent;
class QDragLeaveEvent;
class QDropEvent;
class QKeyEvent;
class QMimeData;
class QMouseEvent;

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

private:
    void setHighlighted(bool highlighted);
    [[nodiscard]] static QStringList localFiles(const QMimeData* mimeData);

    QPalette normalPalette_;
};

} // namespace metadrop
