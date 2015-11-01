#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetWork/QNetworkReply>
#include <QBuffer>
class VersionChecker : public QObject
{
    Q_OBJECT
public:
    explicit VersionChecker(QObject *parent = 0);
    void beginCheck(bool silent);

signals:

public slots:
    void readyRead();
    void finished();

private:
    bool checking;
    bool silent;
    QNetworkAccessManager network;
    QNetworkReply *reply;
    QBuffer remoteVersionBuf;
};

#endif // VERSIONCHECKER_H
