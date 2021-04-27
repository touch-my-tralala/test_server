#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QtCore>
// #include <QJsonDocument>
// #include <QJsonValue>  //? in qtcore?
// #include <QSettings> //? in qtcore?
// #include <QDir>
// #include <QSharedPointer> //
// #include <QMutex> //? in qtcore?

// Сначала public секции затем private
// сначала конструктор/деструктор. После этого сначала методы, потом переменные

// приватные методы и переменные в стиле с нижним подчеркиванием
// публичные методы и переменные в стиле кэмэл
// если создается указатель на что-то в приватном поле то в начало добавляют m_

class Server: public QObject
{
    Q_OBJECT

    struct UserInf{
        QSharedPointer<QTcpSocket> socket;
        QSharedPointer<QTime> time;
        quint64 request = 0;
    };

    struct ResInf{
        ResInf(){}
        ResInf(quint32 usrTime, QString username){
            time = QSharedPointer<QTime>(new QTime(usrTime/3600, usrTime&3600, usrTime%60)); // FIXME можно убрать qsharedpointer
            currenUser = username;
        }

        QSharedPointer<QTime> time;
        QString currenUser = "Free";
    };

public:
    Server();
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();
    void slotDisconnected();

private:
    void ini_parse(QString fname);
    void send_to_client(QTcpSocket &sock, const QJsonObject &jObj);
    void send_to_all_clients();
    bool registr(const std::string &username, uint32_t resource_index);
    void all_res_clear();
    void res_req_handler(const QJsonObject &jObj);
    void service_handler(const QJsonObject &jObj);

private:
    bool reject_res_req = false;
    quint16 port;
    quint8 maxUsers;
    quint16 maxBusyTime;
    QSharedPointer< QTcpServer > m_server;
    QByteArray data;
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;
    QMap<QString, QSharedPointer<UserInf>>  m_userList; // FIXME можно без qsharedpointer
    QMap<quint8, QSharedPointer<ResInf>>  m_resList; // имя ресурса - текущий пользователь
    QSet<QHostAddress>  m_blockIp;
    QMutex mutex;
    QString startServTime;
};

#endif // SERVER_H
