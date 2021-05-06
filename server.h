#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QtCore>

// Сначала public секции затем private
// сначала конструктор/деструктор. После этого сначала методы, потом переменные

// приватные методы и переменные в стиле с нижним подчеркиванием
// публичные методы и переменные в стиле кэмэл
// если создается указатель на что-то в приватном поле то в начало добавляют m_
#define READ_BLOCK_SIZE 32768 // 32 кб

class Server: public QObject
{
    Q_OBJECT

    struct UserInf{
        UserInf(){}
        ~UserInf(){
            time = nullptr;
        }

        QTcpSocket* socket = nullptr;
        QTime* time = nullptr;
        quint64 request = 0;

    };

    struct ResInf{
        ResInf(){
            time = new QTime();
            time->setHMS(0, 0, 0);
        }
        ~ResInf(){
            delete time;
            time = nullptr;
        }

        QTime* time = nullptr;
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
    void json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket);
    void all_res_clear();
    void res_req_take(const QJsonObject &jObj);
    void res_req_free(const QJsonObject &jObj);
    void service_handler(const QJsonObject &jObj);
    void new_client_autorization(QTcpSocket &sock, const QString &newUsrName);

private:
    const qint64 numReadByte = 32;
    quint32 m_nextBlock = 0;
    QByteArray buff;
    bool reject_res_req = false;
    quint16 port;
    quint8 maxUsers;
    qint64 maxBusyTime = 7200; // 2 часа
    QSharedPointer< QTcpServer > m_server;    
    QJsonDocument jsonDoc;
    QJsonParseError jsonErr;
    QMap<QString, UserInf*>  m_userList; // FIXME можно без qsharedpointer
    QMap<quint8, ResInf*>  m_resList; // имя ресурса - текущий пользователь
    QMap<QString, QJsonArray*> m_grabRes; // имя пользователя - лист ресурсов, которые у него забрали
    QSet<QHostAddress>  m_blockIp;
    QMutex mutex;
    QString startServTime;
};

#endif // SERVER_H
