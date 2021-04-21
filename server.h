#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QDebug>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSettings>
#include <QDir>


class Server: public QObject
{
    Q_OBJECT
private:
    QStringList userList;
    quint16 port;
    quint8 maxUsers;
    QTcpServer *server;
    QByteArray data;
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;


private:
    void iniParse(QString fname);
    void sendToClient(QTcpSocket *sock);
    void jsonParse(QJsonDocument *doc);

public:
    Server();
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();
};




#endif // SERVER_H
