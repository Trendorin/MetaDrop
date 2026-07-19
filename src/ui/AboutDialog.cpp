#include "ui/AboutDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace metadrop {

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("About MetaDrop"));
    setModal(true);
    setMinimumWidth(480);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 18);
    layout->setSpacing(12);

    auto* icon = new QLabel(this);
    icon->setPixmap(QApplication::windowIcon().pixmap(64, 64));
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon);

    auto* title = new QLabel(tr("MetaDrop %1").arg(QApplication::applicationVersion()), this);
    QFont titleFont = title->font();
    titleFont.setPointSizeF(titleFont.pointSizeF() + 3.0);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* description = new QLabel(
        tr("A native Linux metadata inspector and sanitizer. Files are processed locally; "
           "MetaDrop contains no telemetry, accounts, analytics, or network client."),
        this);
    description->setAlignment(Qt::AlignCenter);
    description->setWordWrap(true);
    layout->addWidget(description);

    auto* details = new QLabel(
        tr("Built with C++20, Qt 6 Widgets, Exiv2, TagLib, qpdf, and libarchive.<br><br>"
           "Licensed under GPL-3.0-or-later. Source, issue tracker, and security policy: "
           "<a href=\"https://github.com/Trendorin/MetaDrop\">github.com/Trendorin/MetaDrop</a>"),
        this);
    details->setOpenExternalLinks(true);
    details->setTextFormat(Qt::RichText);
    details->setWordWrap(true);
    layout->addWidget(details);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Close, this);
    buttons->button(QDialogButtonBox::Close)->setText(tr("Close"));
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}

} // namespace metadrop
