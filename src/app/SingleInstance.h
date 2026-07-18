#pragma once

#include <QLocalServer>
#include <QObject>
#include <QStringList>

namespace metadrop {

class SingleInstance final : public QObject {
    Q_OBJECT

public:
    enum class StartResult { Primary, Forwarded, Error };

    explicit SingleInstance(QObject* parent = nullptr);
    [[nodiscard]] StartResult start(const QStringList& files, QString* error = nullptr);

signals:
    void filesReceived(const QStringList& files);

private slots:
    void acceptConnection();

private:
    [[nodiscard]] static QString serverName();
    [[nodiscard]] static bool forwardToPrimary(const QString& name, const QStringList& files);

    QLocalServer server_;
};

} // namespace metadrop
