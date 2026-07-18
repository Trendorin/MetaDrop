#include "app/SingleInstance.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream>
#include <QLocalSocket>
#include <QTimer>

#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace metadrop {

SingleInstance::SingleInstance(QObject* parent) : QObject(parent) {
    connect(&server_, &QLocalServer::newConnection, this, &SingleInstance::acceptConnection);
}

SingleInstance::StartResult SingleInstance::start(const QStringList& files, QString* error) {
    const QString name = serverName();
    if (forwardToPrimary(name, files)) {
        return StartResult::Forwarded;
    }

    server_.setSocketOptions(QLocalServer::UserAccessOption);
    if (server_.listen(name)) {
        return StartResult::Primary;
    }

    // A crashed process can leave a stale socket path. Confirm that no live peer owns it first.
    if (forwardToPrimary(name, files)) {
        return StartResult::Forwarded;
    }
    QLocalServer::removeServer(name);
    if (server_.listen(name)) {
        return StartResult::Primary;
    }

    if (error != nullptr) {
        *error = server_.errorString();
    }
    return StartResult::Error;
}

void SingleInstance::acceptConnection() {
    while (server_.hasPendingConnections()) {
        QLocalSocket* socket = server_.nextPendingConnection();
        if (socket == nullptr) {
            continue;
        }

        const auto readMessage = [this, socket] {
            QDataStream stream(socket);
            stream.setVersion(QDataStream::Qt_6_0);
            stream.startTransaction();
            QStringList files;
            stream >> files;
            if (!stream.commitTransaction()) {
                return;
            }
            emit filesReceived(files);
            socket->disconnectFromServer();
        };
        connect(socket, &QLocalSocket::readyRead, socket, readMessage);
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        if (socket->bytesAvailable() > 0) {
            QTimer::singleShot(0, socket, readMessage);
        }
    }
}

QString SingleInstance::serverName() {
    QByteArray identity = QCoreApplication::applicationFilePath().toUtf8();
#ifdef Q_OS_UNIX
    identity.append(QByteArray::number(static_cast<qulonglong>(getuid())));
#endif
    const QByteArray digest = QCryptographicHash::hash(identity, QCryptographicHash::Sha256).toHex();
    return QStringLiteral("metadrop-%1").arg(QString::fromLatin1(digest.left(20)));
}

bool SingleInstance::forwardToPrimary(const QString& name, const QStringList& files) {
    QLocalSocket socket;
    socket.connectToServer(name, QIODevice::WriteOnly);
    if (!socket.waitForConnected(250)) {
        return false;
    }

    QDataStream stream(&socket);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << files;
    socket.flush();
    const bool written = socket.waitForBytesWritten(1000);
    socket.disconnectFromServer();
    return written;
}

} // namespace metadrop
