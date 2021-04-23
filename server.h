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
#include <QTime>
#include <QSharedPointer>

// Сначала public секции затем private
// сначала конструктор/деструктор. После этого сначала методы, потом переменные

// приватные методы и переменные в стиле с нижним подчеркиванием
// публичные методы и переменные в стиле кэмэл
// если создается указатель на что-то в приватном поле то в начало добавляют m_

class Server: public QObject
{
    Q_OBJECT

    struct UserInf{
        QSharedPointer<QTcpSocket> socket = nullptr;
        QSharedPointer<QTime> time = nullptr;
        quint8 request = 0;
    };

public:
    Server();
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();

private:
    void ini_parse(QString fname);
    void send_to_client(QTcpSocket *sock);
    void json_parse(const QJsonObject &doc);

private:
    quint16 port;
    quint8 maxUsers;
    QSharedPointer< QTcpServer > m_server;
    QByteArray data;
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;
    QSharedPointer< QMap<QString, QSharedPointer<UserInf>> > m_userList;
    QSharedPointer< QSet<QHostAddress> > m_blockIp;

};

#endif // SERVER_H
