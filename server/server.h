#ifndef SERVER_H
#define SERVER_H

#include "keys.h"
#include "autoupdater/autoupdater.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <iostream>
#include <QtCore>
#define READ_BLOCK_SIZE 32768 // 32 кб

class Server: public QObject
{
    Q_OBJECT

public:
    Server();
    ~Server() override;
    //! \brief Установка времени после которого можно перехватить ресурс
    void setTimeOut(qint64 secs);
    //! \brief Установка максимального количества пользователей
    void setMaxUser(quint8 maxUser);
    //! \brief Установка режима отклонения входящих подключений
    void setRejectConnection(bool a);
    //! \brief Установка режима отклонения запросов ресурсов
    void setRejectResReq(bool a);
    //! \brief получить список доступных ресурсов
    QStringList getResList();
    //! \brief получить список разрешенных пользователей
    QStringList getUserList();
    //! \brief получить текущего пользователя ресурса
    QString getResUser(QString resName);
    //! \brief получить время сколько ресурс занят
    qint32 getBusyResTime(QString resName);
    //! \brief получить время старта сервера
    const QDateTime& getStartTime() const;
    //! \brief освободить все ресурсы
    void allResClear();
    //! \brief добовить новый ресурс
    void addNewRes(QString resName);
    //! \brief добавить нового разрешенного пользователя
    //! \todo : нормальная система авторизации, а не просто имя
    void addNewUsrName(QString name);
    //! \brief удаление ресурса
    void removeRes(QString resName);
    //! \brief удаления пользователя из списка разрешенных
    void removeUsr(QString name);

signals:
    //! сигнал для логгера
    void signalLogEvent(QString s);

private slots:
    void on_slotNewConnection();
    void on_slotReadClient();
    void on_slotDisconnected();
    //! \brief широковещательная рассылка всем подключенным клиентам
    void send_to_all_clients();

private:
    //! \brief парсинг конфигурации из файла
    void ini_parse(QString fname);
    //! \brief отправка одному клиенту
    void send_to_client(QTcpSocket &sock, const QJsonObject &jObj);
    //! \brief обработка json сообщений
    void json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket);
    //! \brief резервирование ресурса за пользователем
    void res_req_take(const QJsonObject &jObj);
    //! \brief освобождение ресурса от пользователя
    void res_req_free(const QJsonObject &jObj);
    void new_client_autorization(QTcpSocket &sock, const QString &newUsrName);
    //! \brief обработка запроса обновлений. Проверка необходимости и отправка файлов
    void update_req_handle(QTcpSocket &sock, const QJsonObject &jObj);
    //! \brief чтение списка файлов обновления и их версий
    void update_info_json();
    //! \brief запись данных в конфиг файл
    void write_to_config();

private:
    qint64 maxBusyTime = 7200; // 2 часа
    const qint64 numReadByte = 32;
    quint32 m_nextBlock = 0;
    quint16 m_port;
    quint8 m_max_users;
    bool reject_res_req = false;
    QString m_updates_path;
    QMutex mutex;
    QTcpServer m_server;
    AutoUpdater m_updater;
    QSettings* sett; // FIXME: не очень понял в чем дело, но если сделать не как указатель и потом указать setPath(QSettings::IniFormat, QSettings::UserScope, path) то файл не читается.
    QMap<QString, QPair<QTcpSocket*, QTime>>  m_userList;  // FIXME можно без qsharedpointer
    QMap<QString, QPair<QString, QTime>>  m_resList;    // <имя ресурса, <пользователь, время>>
    QMap<QString, QJsonArray> m_grabRes; // имя пользователя - лист ресурсов, которые у него забрали
    QSet<QHostAddress>  m_blockIp;
    QDateTime startServTime;
    QJsonDocument jDoc;
    QJsonParseError jsonErr;
    QByteArray buff;
};

#endif // SERVER_H
