#include "core/RiskClassifier.h"

#include <initializer_list>

namespace metadrop {
namespace {

bool containsAny(const QString& haystack, const std::initializer_list<QStringView> needles) {
    for (const auto needle : needles) {
        if (haystack.contains(needle, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

} // namespace

ClassifiedRisk RiskClassifier::classify(const QString& key, const QString& value) {
    const QString text = key + QLatin1Char(' ') + value.left(512);

    if (containsAny(text,
                    {u"gps", u"latitude", u"longitude", u"geolocation", u"location",
                     u"geotag", u"coordinates", u"sublocation", u"city", u"country",
                     u"region", u"face region"})) {
        return {RiskLevel::Critical,
                QStringLiteral("May reveal a precise or approximate location")};
    }

    if (containsAny(text,
                    {u"serial", u"owner", u"author", u"creator", u"artist", u"email",
                     u"phone", u"by-line", u"credit", u"contact", u"account",
                     u"lastmodifiedby", u"printed-by"})) {
        return {RiskLevel::High,
                QStringLiteral("May identify the person, account, or device that created the file")};
    }

    if (containsAny(text,
                    {u"documentid", u"instanceid", u"uniqueid", u"originaldocumentid",
                     u"mediaid", u"deviceid", u"cameraid", u"uuid", u"identifier"})) {
        return {RiskLevel::High,
                QStringLiteral("A persistent identifier can link this file to other files or devices")};
    }

    if (containsAny(text,
                    {u"datetime", u"date time", u"created", u"creation", u"modified",
                     u"timestamp", u"timezone", u"time zone", u"history", u"duration",
                     u"editing-cycles"})) {
        return {RiskLevel::Medium,
                QStringLiteral("Timing or edit-history data can help correlate activity")};
    }

    if (containsAny(text,
                    {u"software", u"application", u"producer", u"generator", u"firmware",
                     u"make", u"model", u"lens", u"hostcomputer", u"operating system"})) {
        return {RiskLevel::Medium,
                QStringLiteral("Software or hardware details can strengthen a fingerprint")};
    }

    if (containsAny(text,
                    {u"comment", u"description", u"subject", u"title", u"keywords",
                     u"copyright", u"rating", u"label", u"category", u"thumbnail"})) {
        return {RiskLevel::Low, QStringLiteral("Descriptive data may expose unintended context")};
    }

    return {RiskLevel::Low, QStringLiteral("Embedded metadata not required to render the file")};
}

} // namespace metadrop
