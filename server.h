#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
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
        quint64 request = 0;
    };

    struct ResInf{
        ResInf();
        ResInf(int usrTime, QString username){
            this->time = QSharedPointer<QTime>(new QTime(usrTime/3600, usrTime&3600, usrTime%60));
            this->currenUser = username;
        }
        ~ResInf();

        QSharedPointer<QTime> time = nullptr;
        QString currenUser = "Free";
    };

public:
    Server();
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();

private:
    void ini_parse(QString fname);
    void send_to_client(QTcpSocket &sock, const QJsonObject &jObj); // FIXME переделать на const &
    void send_to_all_clients(); // FIXME переделать на const &
    void json_handler(const QJsonObject &jobj);
    void res_request(quint8 resNum, const QString &username);

private:
    quint16 port;
    quint8 maxUsers;
    QSharedPointer< QTcpServer > m_server;
    QByteArray data;
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;
    QMap<QString, QSharedPointer<UserInf>>  m_userList;
    QMap<quint8, QSharedPointer<ResInf>>  m_resList; // имя ресурса - текущий пользователь
    QSet<QHostAddress>  m_blockIp;

};

#endif // SERVER_H
