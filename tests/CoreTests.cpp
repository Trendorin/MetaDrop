#include "core/AppSettings.h"
#include "core/FileGuard.h"
#include "core/OfficeXmlSanitizer.h"
#include "core/OutputPlanner.h"
#include "core/RiskClassifier.h"
#include "core/engines/PdfEngine.h"

#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

#include <algorithm>
#include <array>

using namespace metadrop;

namespace {

QByteArray pdfWithPrivateMetadata() {
    QByteArray pdf = QByteArrayLiteral("%PDF-1.4\n");
    std::array<qsizetype, 5> offsets{};

    const auto appendObject = [&pdf, &offsets](const int number, const QByteArray& body) {
        offsets.at(static_cast<std::size_t>(number)) = pdf.size();
        pdf.append(QByteArray::number(number));
        pdf.append(" 0 obj\n");
        pdf.append(body);
        pdf.append("\nendobj\n");
    };

    appendObject(1, QByteArrayLiteral("<< /Type /Catalog /Pages 2 0 R >>"));
    appendObject(2, QByteArrayLiteral("<< /Type /Pages /Kids [3 0 R] /Count 1 >>"));
    appendObject(
        3, QByteArrayLiteral("<< /Type /Page /Parent 2 0 R /MediaBox [0 0 100 100] >>"));
    appendObject(4, QByteArrayLiteral("<< /Author (Alice) /Creator (Private Tool) >>"));

    const qsizetype xrefOffset = pdf.size();
    pdf.append("xref\n0 5\n0000000000 65535 f \n");
    for (int object = 1; object <= 4; ++object) {
        pdf.append(QByteArray::number(offsets.at(static_cast<std::size_t>(object)))
                       .rightJustified(10, '0'));
        pdf.append(" 00000 n \n");
    }
    pdf.append(
        "trailer\n<< /Size 5 /Root 1 0 R /Info 4 0 R "
        "/ID [<00112233445566778899aabbccddeeff> "
        "<00112233445566778899aabbccddeeff>] >>\nstartxref\n");
    pdf.append(QByteArray::number(xrefOffset));
    pdf.append("\n%%EOF\n");
    return pdf;
}

QString entryValue(const InspectionReport& report, const QString& key) {
    for (const MetadataEntry& entry : report.entries) {
        if (entry.key == key) {
            return entry.value;
        }
    }
    return {};
}

} // namespace

class CoreTests final : public QObject {
    Q_OBJECT

private slots:
    void classifiesLocationAsCritical();
    void classifiesIdentifiersAsHighRisk();
    void createsNonDestructiveOutputNames();
    void validatesRegularFiles();
    void extractsAndClearsOfficeMetadata();
    void regeneratesPdfIdentifier();
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

    const QString compoundSource = directory.filePath(QStringLiteral("archive.tar.gz"));
    QFile compoundSourceFile(compoundSource);
    QVERIFY(compoundSourceFile.open(QIODevice::WriteOnly));
    compoundSourceFile.close();
    const QString compoundFirst = OutputPlanner::destinationFor(compoundSource, settings);
    QCOMPARE(QFileInfo(compoundFirst).fileName(), QStringLiteral("archive.cleaned.tar.gz"));
    QFile compoundOutput(compoundFirst);
    QVERIFY(compoundOutput.open(QIODevice::WriteOnly));
    compoundOutput.close();
    const QString compoundSecond = OutputPlanner::destinationFor(compoundSource, settings);
    QCOMPARE(QFileInfo(compoundSecond).fileName(),
             QStringLiteral("archive.cleaned (2).tar.gz"));
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

void CoreTests::regeneratesPdfIdentifier() {
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    const QString sourcePath = directory.filePath(QStringLiteral("private.pdf"));
    QFile source(sourcePath);
    QVERIFY(source.open(QIODevice::WriteOnly));
    const QByteArray payload = pdfWithPrivateMetadata();
    QCOMPARE(source.write(payload), payload.size());
    source.close();

    PdfEngine engine;
    const InspectionReport before = engine.inspect(sourcePath);
    QVERIFY2(before.valid, qPrintable(before.error));
    QVERIFY(before.removableCount() >= 2);
    const QString originalIdentifier =
        entryValue(before, QStringLiteral("File identifier"));
    QVERIFY(!originalIdentifier.isEmpty());

    const QString outputPath = directory.filePath(QStringLiteral("cleaned.pdf"));
    QString error;
    QStringList warnings;
    QVERIFY2(engine.sanitize(sourcePath, outputPath, {}, &error, &warnings), qPrintable(error));

    const InspectionReport after = engine.inspect(outputPath);
    QVERIFY2(after.valid, qPrintable(after.error));
    QCOMPARE(after.removableCount(), 0);
    const QString replacementIdentifier =
        entryValue(after, QStringLiteral("File identifier"));
    QVERIFY(!replacementIdentifier.isEmpty());
    QVERIFY(replacementIdentifier != originalIdentifier);
}

QTEST_APPLESS_MAIN(CoreTests)

#include "CoreTests.moc"
