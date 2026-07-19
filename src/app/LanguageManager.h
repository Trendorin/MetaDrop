#pragma once

#include <QObject>
#include <QString>
#include <QTranslator>

namespace metadrop {

class LanguageManager final : public QObject {
    Q_OBJECT

public:
    explicit LanguageManager(QObject* parent = nullptr);

    [[nodiscard]] bool applyLanguage(const QString& requestedLanguage);
    [[nodiscard]] QString activeLanguage() const;
    [[nodiscard]] static QString normalizedLanguage(const QString& language);

signals:
    void languageChanged(const QString& language);

private:
    [[nodiscard]] static QString resolveLanguage(const QString& requestedLanguage);

    QTranslator translator_;
    QString activeLanguage_;
    bool translatorInstalled_ = false;
};

} // namespace metadrop
