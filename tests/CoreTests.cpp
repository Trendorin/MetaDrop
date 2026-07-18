#include "core/AppSettings.h"
#include "core/FileGuard.h"
#include "core/OfficeXmlSanitizer.h"
#include "core/OutputPlanner.h"
#include "core/RiskClassifier.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>

using namespace metadrop;

class CoreTests final : public QObject {
    Q_OBJECT

private slots:
    void classifiesLocationAsCritical();
    void classifiesIdentifiersAsHighRisk();
    void createsNonDestructiveOutputNames();
    void validatesRegularFiles();
    void extractsAndClearsOfficeMetadata();
};

void CoreTests::classifiesLocationAsCritical() {
    const ClassifiedRisk risk =
        RiskClassifier::classify(QStringLiteral("Exif.GPSInfo.GPSLatitude"), QStringLiteral("59.3"));
    QCOMPARE(risk.level, RiskLevel::Critical);
    QVERIFY(!risk.reason.isEmpty());
}

void CoreTests::classifiesIdentifiersAsHighRisk() {
    const ClassifiedRisk risk =
        RiskClassifier::classify(QStringLiteral("xmpMM:DocumentID"), QStringLiteral("uuid:123"));
    QCOMPARE(risk.level, RiskLevel::High);
}

void CoreTests::createsNonDestructiveOutputNames() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString source = directory.filePath(QStringLiteral("photo.jpg"));
    QFile sourceFile(source);
    QVERIFY(sourceFile.open(QIODevice::WriteOnly));
    sourceFile.write("image");
    sourceFile.close();

    SettingsSnapshot settings;
    const QString first = OutputPlanner::destinationFor(source, settings);
    QCOMPARE(QFileInfo(first).fileName(), QStringLiteral("photo.cleaned.jpg"));

    QFile firstOutput(first);
    QVERIFY(firstOutput.open(QIODevice::WriteOnly));
    firstOutput.close();
    const QString second = OutputPlanner::destinationFor(source, settings);
    QCOMPARE(QFileInfo(second).fileName(), QStringLiteral("photo.cleaned (2).jpg"));
}

void CoreTests::validatesRegularFiles() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("sample.bin"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("test");
    file.close();

    const PathValidation valid = FileGuard::validateInput(path);
    QVERIFY(valid.ok);
    QVERIFY(!valid.canonicalPath.isEmpty());

    const PathValidation missing = FileGuard::validateInput(directory.filePath(QStringLiteral("missing")));
    QVERIFY(!missing.ok);
}

void CoreTests::extractsAndClearsOfficeMetadata() {
    const QByteArray coreXml = QByteArrayLiteral(
        "<?xml version=\"1.0\"?>"
        "<cp:coreProperties "
        "xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\" "
        "xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "<dc:creator>Alice</dc:creator><cp:lastModifiedBy>Bob</cp:lastModifiedBy>"
        "</cp:coreProperties>");

    const auto entries = OfficeXmlSanitizer::inspect(QStringLiteral("docProps/core.xml"), coreXml);
    QCOMPARE(entries.size(), 2);
    QVERIFY(std::all_of(entries.cbegin(), entries.cend(),
                        [](const MetadataEntry& entry) { return entry.removable; }));

    const QByteArray cleaned =
        OfficeXmlSanitizer::sanitizedXml(QStringLiteral("docProps/core.xml"));
    QVERIFY(!cleaned.isEmpty());
    QCOMPARE(OfficeXmlSanitizer::inspect(QStringLiteral("docProps/core.xml"), cleaned).size(), 0);
}

QTEST_APPLESS_MAIN(CoreTests)

#include "CoreTests.moc"
