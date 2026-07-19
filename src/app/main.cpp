#include "app/SingleInstance.h"
#include "app/LanguageManager.h"
#include "core/AppSettings.h"
#include "core/MetadataTypes.h"
#include "ui/MainWindow.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QMessageBox>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif

int main(int argc, char* argv[]) {
#ifdef Q_OS_UNIX
    // Any file created by a backend starts private; verified outputs may be widened later.
    umask(S_IRWXG | S_IRWXO);
#endif
    QApplication application(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("Trendorin"));
    QApplication::setOrganizationDomain(QStringLiteral("github.com/Trendorin"));
    QApplication::setApplicationName(QStringLiteral("MetaDrop"));
    QApplication::setApplicationDisplayName(QStringLiteral("MetaDrop"));
    QApplication::setApplicationVersion(QString::fromLatin1(METADROP_VERSION));
    QApplication::setDesktopFileName(QString::fromLatin1(METADROP_APP_ID));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/metadrop.png")));

    qRegisterMetaType<metadrop::InspectionReport>();
    qRegisterMetaType<metadrop::SanitizeReport>();

    metadrop::AppSettings settings;
    metadrop::LanguageManager languages;
    (void)languages.applyLanguage(settings.snapshot().language);
    QObject::connect(&settings, &metadrop::AppSettings::changed, &languages, [&settings, &languages] {
        (void)languages.applyLanguage(settings.snapshot().language);
    });

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QApplication::translate("main", "Inspect and remove privacy-sensitive file metadata"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption noTrayOption(
        QStringLiteral("no-tray"),
        QApplication::translate("main", "Run without a system tray icon"));
    parser.addOption(noTrayOption);
    parser.addPositionalArgument(QStringLiteral("files"),
                                 QApplication::translate("main", "Files to inspect"),
                                 QStringLiteral("[files…]"));
    parser.process(application);

    metadrop::SingleInstance instance;
    QString instanceError;
    const auto instanceResult = instance.start(parser.positionalArguments(), &instanceError);
    if (instanceResult == metadrop::SingleInstance::StartResult::Forwarded) {
        return 0;
    }
    if (instanceResult == metadrop::SingleInstance::StartResult::Error) {
        QMessageBox::critical(nullptr, QObject::tr("MetaDrop could not start"), instanceError);
        return 1;
    }

    metadrop::MainWindow window(&settings);
    window.setTrayEnabled(!parser.isSet(noTrayOption));
    QObject::connect(&instance, &metadrop::SingleInstance::filesReceived, &window,
                     &metadrop::MainWindow::receiveFilesFromSecondaryInstance);

    window.show();
    window.addFiles(parser.positionalArguments());
    return application.exec();
}
