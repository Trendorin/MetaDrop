#include "ui/DropArea.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QStyle>
#include <QUrl>
#include <QVBoxLayout>

namespace metadrop {

DropArea::DropArea(QWidget* parent) : QFrame(parent) {
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Plain);
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    setMinimumHeight(132);
    setAccessibleName(tr("File drop area"));
    setAccessibleDescription(tr("Drop files here or activate this area to open a file picker"));

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 18, 24, 18);
    layout->setSpacing(6);

    auto* icon = new QLabel(this);
    icon->setPixmap(style()->standardIcon(QStyle::SP_DialogOpenButton).pixmap(28, 28));
    icon->setAlignment(Qt::AlignCenter);
    icon->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* title = new QLabel(tr("Drop files to inspect"), this);
    QFont titleFont = title->font();
    titleFont.setBold(true);
    titleFont.setPointSizeF(titleFont.pointSizeF() + 1.0);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    title->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto* subtitle =
        new QLabel(tr("Images, audio, PDF, OpenDocument, and Microsoft Office files"), this);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    subtitle->setForegroundRole(QPalette::PlaceholderText);
    subtitle->setAttribute(Qt::WA_TransparentForMouseEvents);

    layout->addWidget(icon);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    normalPalette_ = palette();
}

void DropArea::dragEnterEvent(QDragEnterEvent* event) {
    if (!localFiles(event->mimeData()).isEmpty()) {
        event->acceptProposedAction();
        setHighlighted(true);
    }
}

void DropArea::dragLeaveEvent(QDragLeaveEvent* event) {
    setHighlighted(false);
    event->accept();
}

void DropArea::dropEvent(QDropEvent* event) {
    setHighlighted(false);
    const QStringList files = localFiles(event->mimeData());
    if (!files.isEmpty()) {
        event->acceptProposedAction();
        emit filesDropped(files);
    }
}

void DropArea::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && rect().contains(event->position().toPoint())) {
        emit browseRequested();
        event->accept();
        return;
    }
    QFrame::mouseReleaseEvent(event);
}

void DropArea::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
        event->key() == Qt::Key_Space) {
        emit browseRequested();
        event->accept();
        return;
    }
    QFrame::keyPressEvent(event);
}

void DropArea::setHighlighted(const bool highlighted) {
    if (!highlighted) {
        setPalette(normalPalette_);
        return;
    }
    QPalette highlightedPalette = normalPalette_;
    highlightedPalette.setColor(QPalette::Window,
                                normalPalette_.color(QPalette::AlternateBase));
    setPalette(highlightedPalette);
}

QStringList DropArea::localFiles(const QMimeData* mimeData) {
    QStringList files;
    if (mimeData == nullptr || !mimeData->hasUrls()) {
        return files;
    }
    for (const QUrl& url : mimeData->urls()) {
        if (url.isLocalFile()) {
            files.append(url.toLocalFile());
        }
    }
    return files;
}

} // namespace metadrop
