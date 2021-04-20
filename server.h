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


class Server: public QObject
{
    Q_OBJECT
private:
    QTcpServer *server;
    QByteArray data;
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;


private:
    void sendToClient(QTcpSocket *sock);
    void jsonParse(QJsonDocument *doc);

public:
    Server(quint16 port);
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();
};




#endif // SERVER_H
