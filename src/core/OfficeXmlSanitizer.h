#pragma once

#include "core/MetadataTypes.h"

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>

namespace metadrop {

class OfficeXmlSanitizer final {
public:
    [[nodiscard]] static bool isMetadataEntry(const QString& archivePath);
    [[nodiscard]] static bool isThumbnailEntry(const QString& archivePath);
    [[nodiscard]] static QByteArray sanitizedXml(const QString& archivePath);
    [[nodiscard]] static QList<MetadataEntry> inspect(const QString& archivePath,
                                                      const QByteArray& xml,
                                                      QStringList* warnings = nullptr);
};

} // namespace metadrop
