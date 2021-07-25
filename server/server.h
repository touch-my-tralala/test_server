#ifndef SERVER_H
#define SERVER_H

#include "json_keys.h"
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
        QString currentUser = JSON_KEYS::State().free;
    };

public:
    Server();
    ~Server() override;
    //! Установка времени после которого можно перехватить ресурс
    void setTimeOut(qint64 secs);
    void setMaxUser(quint8 maxUser);
    //! Установка режима отклонения входящих подключений
    void setRejectConnection(bool a);
    //! Установка режима отклонения запросов ресурсов
    void setRejectResReq(bool a);
    //! список доступных ресурсов
    QStringList getResList();
    //! список разрешенных пользователей
    QStringList getUserList();
    QString getResUser(QString resName);
    //! время сколько ресурс занят
    qint32 getBusyResTime(QString resName);
    //! время старта сервера
    const QDateTime& getStartTime() const;
    //! освобождение всех ресурсов
    void allResClear();
    //! добовления нового ресурса
    void addNewRes(QString resName);
    //! добовления нового разрешенного пользователя
    void addNewUsrName(QString name);
    //! удаление ресурса
    void removeRes(QString resName);
    //! удаления пользователя из списка разрешенных
    void removeUsr(QString name);

signals:
    //! сигнал для логгера
    void signalLogEvent(QString s);

private slots:
    void on_slotNewConnection();
    void on_slotReadClient();
    void on_slotDisconnected();

private:
    //! парсинг конфигурации из файла
    void ini_parse(QString fname);
    //! отправка одному клиенту
    void send_to_client(QTcpSocket &sock, const QJsonObject &jObj);
    //! широковещательная рассылка всем подключенным клиентам
    void send_to_all_clients();
    void json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket);
    //! резервирование ресурса за пользователем
    void res_req_take(const QJsonObject &jObj);
    //! освобождение ресурса от пользователя
    void res_req_free(const QJsonObject &jObj);
    void new_client_autorization(QTcpSocket &sock, const QString &newUsrName);

private:
    qint64 maxBusyTime = 7200; // 2 часа
    const qint64 numReadByte = 32;
    quint32 m_nextBlock = 0;
    quint16 m_port;
    quint8 m_max_users;
    bool reject_res_req = false;
    QString m_cur_version;

    QMutex mutex;
    QTcpServer m_server;

    QSharedPointer<QSettings> sett;
    QMap<QString, UserInf>  m_userList;  // FIXME можно без qsharedpointer
    QMap<QString, ResInf>  m_resList;    // имя ресурса - текущий пользователь
    QMap<QString, QJsonArray> m_grabRes; // имя пользователя - лист ресурсов, которые у него забрали
    QSet<QHostAddress>  m_blockIp;
    QDateTime startServTime;
    QJsonDocument jDoc;
    QJsonParseError jsonErr;
    QByteArray buff;
};

#endif // SERVER_H
