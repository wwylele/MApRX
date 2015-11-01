#include "versionchecker.h"
#include <QTextStream>
#include <QMessageBox>
#include "main.h"
const char* remoteVersionUrl=
        "https://raw.githubusercontent.com/wwylele/MApRX/master/"
        "src/MApRX_Qt/text/version.txt";
VersionChecker::VersionChecker(QObject *parent) :
    QObject(parent),
    checking(false)
{

}

void VersionChecker::beginCheck(bool silent){
    if(checking)return;
    checking=true;
    this->silent=silent;

    QNetworkRequest request;
    request.setUrl(QUrl(remoteVersionUrl));
    QSslConfiguration config;
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::SecureProtocols);
    request.setSslConfiguration(config);

    remoteVersionBuf.open(QBuffer::ReadWrite);
    reply = network.get(request);
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(readyRead()));
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
            reply, SLOT(ignoreSslErrors()));
    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));
}

void VersionChecker::readyRead(){
    remoteVersionBuf.write(reply->readAll());
}




void VersionChecker::finished(){

    auto checkFail=silent?[](){}:[](){// I love this haha
            QMessageBox::critical(nullptr,
                tr("Update check failed"),
                tr("Unable to get update information."));
    };
    auto versionCompare=[](int a[4],int b[4])->int{
        for(int i=0;i<4;i++)if(a[i]!=b[i])return a[i]-b[i];
        return 0;
    };

    if (reply->error()) {
        qDebug()<<"VersionChecker::finished():error";
        checkFail();
    }
    else{
        QTextStream text(&remoteVersionBuf);
        text.seek(0);
        QString versionStr=text.readLine();
        qDebug()<<"VersionChecker::finished():"<<versionStr;
        QStringList sl=versionStr.split(",");
        if(sl.size()==4){
            int remoteVersion[4];
            for(int i=0;i<4;i++)remoteVersion[i]=sl[i].toInt();
            if(versionCompare(remoteVersion,res->version)>0){
                QMessageBox::information(nullptr,
                    tr("Update check finished"),
                    tr("New version detected!<br>"
                       "New version:%1<br>"
                       "Current verion:%2<br>"
                       "<a href=\"https://github.com/wwylele/MApRX/releases\">"
                       "Click here to download new version!</a>")
                        .arg(versionToString(remoteVersion))
                        .arg(versionToString(res->version))
                                         );
            }else{
                if(!silent){
                    QMessageBox::information(nullptr,
                        tr("Update check finished"),
                        tr("This is the latest MApRX version")
                                             );
                }
            }
        }else{
            checkFail();
        }
    }

    reply->deleteLater();
    checking=false;
}
