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
        ~UserInf(){}

        QTcpSocket* socket = nullptr;
        QTime time;
        quint64 request = 0;

    };

    struct ResInf{
        ResInf(){}
        ~ResInf(){}

        QTime time;
        QString currentUser = "Free";
    };
/*
public:
    struct Keys{
        const QString port = "port";
        const QString max_user = "max_user";
        const QString
    }; */

public:
    Server();
    ~Server() override;
    void setTimeOut(qint64 secs);
    void setMaxUser(quint8 maxUser);
    void setRejectConnection(bool a);
    void setRejectResReq(bool a);
    QList<quint8> getResList();
    QString getResUser(quint8 resNum);
    qint32 getBusyResTime(quint8 resNum);
    void allResClear();
    void addNewRes(quint8 resNum);
    void addNewUsrName(QString name);
    void removeRes(quint8 resNum);
    void removeUsr(QString name);

private slots:
    void on_slotNewConnection();
    void on_slotReadClient();
    void on_slotDisconnected();

private:
    void ini_parse(QString fname);
    void send_to_client(QTcpSocket &sock, const QJsonObject &jObj);
    void send_to_all_clients();
    bool registr(const std::string &username, uint32_t resource_index);
    void json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket);
    void res_req_take(const QJsonObject &jObj);
    void res_req_free(const QJsonObject &jObj);
    void new_client_autorization(QTcpSocket &sock, const QString &newUsrName);

private:
    // params
    qint64 maxBusyTime = 7200; // 2 часа
    const qint64 numReadByte = 32;
    quint32 m_nextBlock = 0;
    quint16 port;
    quint8 maxUsers;
    bool reject_res_req = false;

    QMutex mutex;
    QTcpServer m_server;

    QSettings sett;
    QMap<QString, UserInf>  m_userList; // FIXME можно без qsharedpointer
    QMap<quint8, ResInf>  m_resList; // имя ресурса - текущий пользователь
    QMap<QString, QJsonArray> m_grabRes; // имя пользователя - лист ресурсов, которые у него забрали
    QSet<QHostAddress>  m_blockIp;
    QString startServTime;
    QJsonDocument jDoc;
    QJsonParseError jsonErr;
    QByteArray buff;
};

#endif // SERVER_H
