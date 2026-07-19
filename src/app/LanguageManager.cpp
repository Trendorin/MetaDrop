#include "app/LanguageManager.h"

#include <QCoreApplication>
#include <QLocale>

namespace metadrop {

LanguageManager::LanguageManager(QObject* parent) : QObject(parent) {}

bool LanguageManager::applyLanguage(const QString& requestedLanguage) {
    const QString resolved = resolveLanguage(requestedLanguage);
    if (resolved == activeLanguage_) {
        return true;
    }

    if (translatorInstalled_) {
        QCoreApplication::removeTranslator(&translator_);
        translatorInstalled_ = false;
    }

    if (resolved != QStringLiteral("en")) {
        const QString resource = QStringLiteral(":/i18n/metadrop_%1.qm").arg(resolved);
        if (!translator_.load(resource)) {
            activeLanguage_ = QStringLiteral("en");
            QLocale::setDefault(QLocale(QLocale::English));
            emit languageChanged(activeLanguage_);
            return false;
        }
        translatorInstalled_ = QCoreApplication::installTranslator(&translator_);
        if (!translatorInstalled_) {
            activeLanguage_ = QStringLiteral("en");
            QLocale::setDefault(QLocale(QLocale::English));
            emit languageChanged(activeLanguage_);
            return false;
        }
    }

    activeLanguage_ = resolved;
    QLocale::setDefault(QLocale(resolved));
    emit languageChanged(activeLanguage_);
    return true;
}

QString LanguageManager::activeLanguage() const {
    return activeLanguage_.isEmpty() ? QStringLiteral("en") : activeLanguage_;
}

QString LanguageManager::normalizedLanguage(const QString& language) {
    const QString normalized = language.trimmed().toLower().section(QLatin1Char('_'), 0, 0)
                                   .section(QLatin1Char('-'), 0, 0);
    if (normalized == QStringLiteral("ua")) {
        return QStringLiteral("uk");
    }
    if (normalized == QStringLiteral("system") || normalized == QStringLiteral("en") ||
        normalized == QStringLiteral("ru") || normalized == QStringLiteral("uk") ||
        normalized == QStringLiteral("de")) {
        return normalized;
    }
    return QStringLiteral("system");
}

QString LanguageManager::resolveLanguage(const QString& requestedLanguage) {
    const QString normalized = normalizedLanguage(requestedLanguage);
    if (normalized != QStringLiteral("system")) {
        return normalized;
    }

    switch (QLocale::system().language()) {
    case QLocale::Russian:
        return QStringLiteral("ru");
    case QLocale::Ukrainian:
        return QStringLiteral("uk");
    case QLocale::German:
        return QStringLiteral("de");
    default:
        return QStringLiteral("en");
    }
}

} // namespace metadrop
