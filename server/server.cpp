#include "server.h"

Server::Server()
{
    Server::ini_parse("init.ini");

    init();
}

Server::~Server()
{
    write_to_config();

    if (m_server.isListening())
        m_server.close();
}

void Server::setTimeOut(qint32 secs)
{
    maxBusyTime = secs;
    emit signalLogEvent("Server → Тайм-аут " + QTime(0, 0, 0).addSecs(secs).toString() + " успешно установлен.");
}

void Server::setMaxUser(quint8 maxUsers)
{
    m_max_users = maxUsers;
    emit signalLogEvent("Server → Макс. количество пользователей " + QString::number(maxUsers) + " успешно установлено.");
}

void Server::setRejectConnection(bool a)
{
    if (a)
    {
        m_server.pauseAccepting();
        emit signalLogEvent("Server → Входящие подключения отклоняются.");
    }
    else
    {
        m_server.resumeAccepting();
        emit signalLogEvent("Server → Входящие подключения принимаются.");
    }
}

void Server::setRejectResReq(bool a)
{
    reject_res_req = a;
    QString s      = a ? "Server → Запросы ресурсов отклояются." : "Server → Запросы ресурсов принимаются.";
    emit    signalLogEvent(s);
}

QStringList Server::getResList()
{
    QStringList resList;
    for (auto i = m_resList.begin(); i != m_resList.end(); i++)
        resList << i.key();

    return resList;
}

QStringList Server::getUserList()
{
    QStringList usrList;
    for (auto i = m_userList.begin(); i != m_userList.end(); i++)
        usrList << i.key();

    return usrList;
}

QString Server::getResUser(QString resName)
{
    if (m_resList.contains(resName))
        return m_resList[resName].first;

    emit signalLogEvent("ОШИБКА → Пользователя с таким именем нет в списке getResUSer().");
    return "";
}

// возвращает время в секундах прошедшее с момента занятия ресурса
qint32 Server::getBusyResTime(QString resName)
{
    if (m_resList.contains(resName))
    {
        if (m_resList[resName].first == KEYS::Common().no_user)
            return 0;
        else
            return m_resList[resName].second.secsTo(QTime::currentTime());
    }
    emit signalLogEvent("ОШИБКА → Ресурса с таким именем нет в списке getBusyResTime().");
    return -1;
}

const QDateTime& Server::getStartTime() const
{
    return startServTime;
}

void Server::changePort(const int &port)
{
    if(m_server.isListening())
        m_server.disconnect();

    if (!m_server.listen(QHostAddress::Any, port))
    {
        emit signalLogEvent("ОШИБКА → Прослушивание порта невозможно.");
        return;
    }
    else{
        m_port = port;
        emit signalLogEvent("Server → порт изменен на - " + QString::number(m_port) + ".");
    }

}

void Server::allResClear()
{
    QJsonObject jObj;
    for (auto i = m_resList.begin(); i != m_resList.end(); i++)
    {
        if (i->first != KEYS::Common().no_user)
        {
            i->first = KEYS::Common().no_user;
            i->second.setHMS(0, 0, 0);
        }
    }
    send_to_all_clients();
    emit signalLogEvent("Server → Очистка всех ресурсов.");
}

void Server::addNewRes(QString resName)
{
    if (!m_resList.contains(resName))
    {
        m_resList.insert(resName, { KEYS::Common().no_user, QTime(0, 0, 0) });
        send_to_all_clients();
        emit signalLogEvent("Server → Ресурс " + resName + " успешно добавлен.");
    }
    else
        emit signalLogEvent("ОШИБКА → Ресурс " + resName + "уже есть в списке.");
}

void Server::addNewUsrName(QString name)
{
    if (!m_userList.contains(name))
    {
        m_userList.insert(name, { nullptr, QTime(0, 0, 0) });
        emit signalLogEvent("Server → Пользователь " + name + " успешно добавлен.");
    }
    else
        emit signalLogEvent("ОШИБКА → Пользователь " + name + " уже есть в списке.");
}

void Server::removeRes(QString resName)
{
    if (m_resList.contains(resName))
    {
        m_resList.remove(resName);
        send_to_all_clients();
        emit signalLogEvent("Server → Ресурс " + resName + " успешно удален.");
    }
    else
        emit signalLogEvent("ОШИБКА → Ресурса " + resName + " нет в списке удален.");
}

void Server::removeUsr(QString name)
{
    if (m_userList.contains(name))
    {
        m_userList.remove(name);
        emit signalLogEvent("Server → Пользователь " + name + " успешно удален.");
    }
    else
        emit signalLogEvent("ОШИБКА → Пользователя " + name + " нет в списке.");
}

void Server::ini_parse(QString fname)
{
    sett = new QSettings(QDir::currentPath() + "/" + fname, QSettings::IniFormat, this);
    if (sett->isWritable())
    {
        sett->setIniCodec("UTF-8");

        // Инициализация настроек сервера
        sett->beginGroup(KEYS::Config().server_settings);
        m_port      = static_cast<quint16>(sett->value(KEYS::Config().port, 9292).toUInt());
        m_max_users = static_cast<quint8>(sett->value(KEYS::Config().max_user, 5).toUInt());
        sett->endGroup();

        // Инициализация списка разрешенных пользователей
        QStringList iniList;

        sett->beginGroup(KEYS::Config().user_list);
        iniList = sett->childKeys();
        for (const auto& i : qAsConst(iniList))
        {
            auto name = sett->value(i, "no_data").toString();
            m_userList.insert(name, { nullptr, QTime(0, 0, 0) });
        }
        sett->endGroup();

        // Инициализация списка ресурсов
        sett->beginGroup(KEYS::Config().resource_list);
        iniList = sett->childKeys();
        for (const auto& i : qAsConst(iniList))
        {
            auto name = sett->value(i, "no_data").toString().toLower();
            m_resList.insert(name, { KEYS::Common().no_user, QTime(0, 0, 0) });
        }
        sett->endGroup();

        // Чтение пути к файлам обновлений
        sett->beginGroup(KEYS::Config().updates);
        if (sett->contains(KEYS::Config().update_path))
        {
            auto path = sett->value(KEYS::Config().update_path).toString();
            if (m_updater.setUpdateFilePath(QDir::currentPath() + path))
            {
                m_updates_path = path;
                updates_file_info();
            }
            else
                emit signalLogEvent("ОШИБКА → путь к файлам обновления не найден. Path = " + path);
        }
        sett->endGroup();
    }
    else
        emit signalLogEvent("ОШИБКА → ini файл в режиме read-only.");
}

void Server::init()
{
    m_server.setMaxPendingConnections(m_max_users);
    emit signalLogEvent("Server → Максимальное количество пользователей " + QString::number(m_max_users));

    if (!m_server.listen(QHostAddress::Any, m_port))
    {
        emit signalLogEvent("ОШИБКА → Прослушивание порта невозможно.");
        return;
    }
    else
        emit signalLogEvent("Server → порт - " + QString::number(m_port) + ".");

    connect(&m_server, &QTcpServer::newConnection,
            this, &Server::on_slotNewConnection);

    startServTime = QDateTime::currentDateTime();
    emit signalLogEvent("Server → Время старта: " + startServTime.toString("hh:mm:ss."));
}

void Server::updates_file_info()
{
    QFile file(QDir::currentPath() + m_updates_path + "/" + "updates.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!file.isOpen())
        return;
    QString val = file.readAll();
    file.close();

    QJsonDocument jDoc = QJsonDocument::fromJson(val.toUtf8());
    QJsonArray    jArr = jDoc.object()["files"].toArray();

    QString file_name, version;
    for (const auto& i : qAsConst(jArr))
    {
        auto j    = i.toObject();
        file_name = j[KEYS::Updater().file_name].toString();
        version   = j[KEYS::Updater().file_version].toString();
        m_updater.addUpdateFile({ file_name, version });
    }
}

void Server::on_slotNewConnection()
{
    QTcpSocket* clientSocket = m_server.nextPendingConnection();

    connect(clientSocket, &QTcpSocket::disconnected,
            this, &Server::on_slotDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &Server::on_slotReadClient);
}

void Server::on_slotReadClient()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    QDataStream readStream(clientSocket);
    readStream.setVersion(QDataStream::Qt_5_12);
    while (!readStream.atEnd())
    {
        if (!m_data_size)
        {
            qint32 header_size = sizeof(quint32);
            if (clientSocket->bytesAvailable() < header_size)
                return;
            readStream >> m_data_size;
        }

        if (clientSocket->bytesAvailable() < m_data_size)
            return;

        quint8 byte;
        for (quint32 i = 0; i < m_data_size; i++)
        {
            readStream >> byte;
            m_buff.append(byte);
        }

        auto jDoc = QJsonDocument::fromJson(m_buff, &jsonErr);
        if (jsonErr.error == QJsonParseError::NoError)
        {
            auto address = clientSocket->peerAddress();
            json_handler(jDoc.object(), address, *clientSocket);
        }
        else
            emit signalLogEvent("ОШИБКА → Ошибка json-формата " + jsonErr.errorString() + ".");

        m_buff.clear();
        m_data_size = 0;
    }
}

void Server::on_slotDisconnected()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket != nullptr)
        clientSocket->abort();
    clientSocket->deleteLater();
}

void Server::json_handler(const QJsonObject& jObj, const QHostAddress& clientIp, QTcpSocket& clientSocket)
{
    // Проверка на бан
    if (m_blockIp.contains(clientIp))
    {
        QJsonObject jObj({ { KEYS::Json().type, KEYS::Json().connect_fail } });
        send_to_client(clientSocket, jObj, Json_type);
        clientSocket.disconnectFromHost();
        emit signalLogEvent("Server → IP клиента " + clientIp.toString() + " находися в бан листе до конца сессиий сервера.");
    }

    // Ответ на запрос обновлений
    // TODO: запрос обновления после подключения пользователя. А не до основных действий
    if (jObj.contains(KEYS::Updater().update_req))
    {
        update_req_handle(clientSocket, jObj);
        return;
    }

    QString name = jObj[KEYS::Json().user_name].toString();
    // Проверка разрешения имени
    if (!m_userList.contains(name))
    {
        m_blockIp.insert(clientIp);
        QJsonObject jObj({ { KEYS::Json().type, KEYS::Json().connect_fail } });
        send_to_client(clientSocket, jObj, Json_type);
        clientSocket.abort();
        emit signalLogEvent("Server → Имя пользователя " + jObj[KEYS::Json().user_name].toString() + " не является разрешенным");
        return;
    }

    // Первое подключение
    if (!jObj.contains(KEYS::Json().action))
    {
        new_client_autorization(clientSocket, name);
        return;
    }

    // Обработка действий
    if (jObj[KEYS::Json().action].toString() == KEYS::Json().take)
        res_req_take(jObj);
    else if (jObj[KEYS::Json().action].toString() == KEYS::Json().drop)
        res_req_free(jObj);

    // Широковещательная рассылка всем клиентам
    send_to_all_clients();
}

// Первое подключение пользователя.
void Server::new_client_autorization(QTcpSocket& sock, const QString& newUsrName)
{
    m_userList[newUsrName].first = &sock;

    QJsonArray resources;
    for (auto i = m_resList.begin(); i != m_resList.end(); i++)
    {
        resources.push_back(QJsonObject({ { KEYS::Json().res_name, i.key() },
                                          { KEYS::Json().user_name, i->first },
                                          { KEYS::Json().time, i->second.second() } }));
    }
    QJsonObject jObj({ { KEYS::Json().type, KEYS::Json().authorization },
                       { KEYS::Json().resources, resources } });
    send_to_client(sock, jObj, Json_type);
}

// Запрос ресурсов
void Server::res_req_take(const QJsonObject& jObj)
{
    if (reject_res_req)
        return;

    QMap<QString, QJsonArray> grab_res;
    QJsonArray                arr;
    QString                   res_name, past_usr;
    qint64                    diffTime = 0;

    auto usr_name = jObj[KEYS::Json().user_name].toString();
    int  secs     = jObj[KEYS::Json().time].toInt();
    auto res_arr  = jObj[KEYS::Json().resources].toArray();

    for (const auto& i : qAsConst(res_arr))
    {
        res_name = i.toString();

        if (m_resList[res_name].first != KEYS::Common().no_user)
            diffTime = m_resList[res_name].second.secsTo(QTime::currentTime());

        if (m_resList[res_name].first == KEYS::Common().no_user || diffTime > maxBusyTime)
        {
            past_usr = m_resList[res_name].first;

            if (past_usr == usr_name)
                continue;

            m_resList[res_name].first  = usr_name;
            m_resList[res_name].second = QTime(0, 0, 0).addSecs(secs);
            arr.push_back(QJsonObject({ { KEYS::Json().res_name, res_name },
                                        { KEYS::Json().status, true } }));

            emit resOwnerChenge();

            // Составление списка пользователей у которых забрали ресурсы.
            if (diffTime > maxBusyTime && past_usr != KEYS::Common().no_user)
            {
                if (grab_res.contains(past_usr))
                    grab_res[past_usr].push_back(res_name);
                else
                {
                    grab_res.insert(past_usr, QJsonArray());
                    grab_res[past_usr].push_back(res_name);
                }
            }
        }
        else
        {
            arr.push_back(QJsonObject({ { KEYS::Json().res_name, res_name },
                                        { KEYS::Json().status, false } }));
        }
    }

    // Отправка всем пользователям у которых забрали ресурс список какие ресурсы у них забрали.
    for (auto i = grab_res.begin(), e = grab_res.end(); i != e; i++)
    {
        QJsonObject jobj({ { KEYS::Json().type, KEYS::Json().grab_res },
                           { KEYS::Json().resources, i.value() } });
        send_to_client(*m_userList[i.key()].first, jobj, Json_type);
    }

    QJsonObject obj({ { KEYS::Json().type, KEYS::Json().request_responce },
                      { KEYS::Json().action, KEYS::Json().take },
                      { KEYS::Json().resources, arr } });
    send_to_client(*m_userList[usr_name].first, obj, Json_type);
}

// Освобождение ресурсов
void Server::res_req_free(const QJsonObject& jObj)
{
    QJsonArray arr;
    QString    res_name;

    auto usr_name = jObj[KEYS::Json().user_name].toString();
    auto res_arr  = jObj[KEYS::Json().resources].toArray();

    for (const auto& i : qAsConst(res_arr))
    {
        res_name = i.toString();

        if (m_resList[res_name].first == usr_name)
        {
            m_resList[res_name].first = KEYS::Common().no_user;
            m_resList[res_name].second.setHMS(0, 0, 0);
            arr.push_back(QJsonObject({ { KEYS::Json().res_name, res_name },
                                        { KEYS::Json().status, true } }));
            emit resOwnerChenge();
        }
        else
        {
            arr.push_back(QJsonObject({ { KEYS::Json().res_name, res_name },
                                        { KEYS::Json().status, false } }));
        }
    }

    QJsonObject obj({ { KEYS::Json().type, KEYS::Json().request_responce },
                      { KEYS::Json().action, KEYS::Json().drop },
                      { KEYS::Json().resources, arr } });
    send_to_client(*m_userList[usr_name].first, obj, Json_type);
}

void Server::update_req_handle(QTcpSocket& sock, const QJsonObject& jObj)
{
    auto jArr = jObj[KEYS::Updater().files].toArray();

    QString    file_name, file_version;
    QByteArray header;
    header.append(File_type);
    for (const auto& i : qAsConst(jArr))
    {
        auto jObj    = i.toObject();
        file_name    = jObj[KEYS::Updater().file_name].toString();
        file_version = jObj[KEYS::Updater().file_version].toString();
        m_updater.checkAndSendFile(sock, { file_name, file_version }, header);
    }
}

void Server::send_to_all_clients()
{
    QJsonArray resources;

    for (auto i = m_resList.begin(); i != m_resList.end(); i++)
    {
        resources.push_back(QJsonObject({ { KEYS::Json().res_name, i.key() },
                                          { KEYS::Json().user_name, i->first },
                                          { KEYS::Json().time, i->second.second() } }));
    }

    QJsonObject jObj({ { KEYS::Json().type, KEYS::Json().broadcast },
                       { KEYS::Json().resources, resources } });

    for (auto i = m_userList.begin(); i != m_userList.end(); ++i)
    {
        if (i->first != nullptr)
            send_to_client(*i->first, jObj, Json_type);
    }
}

void Server::send_to_client(QTcpSocket& sock, const QJsonObject& jObj, const quint8& type)
{

    if (sock.state() == QAbstractSocket::ConnectedState)
    {
        QDataStream sendStream(&sock);
        sendStream.setVersion(QDataStream::Qt_5_12);
        sendStream << quint8(type) << QJsonDocument(jObj).toJson(QJsonDocument::Compact);
    }
    else
        emit signalLogEvent("ОШИБКА → Сокет c IP -" + sock.peerAddress().toString() + " не подключен.");
}

void Server::write_to_config()
{
    quint32 j = 0;

    sett->clear();
    sett->beginGroup(KEYS::Config().resource_list);
    for (auto i = m_resList.begin(), end = m_resList.end(); i != end; i++)
    {
        sett->setValue("res" + QString::number(j), i.key());
        j++;
    }
    sett->endGroup();

    j = 0;
    sett->beginGroup(KEYS::Config().user_list);
    for (auto i = m_userList.begin(), end = m_userList.end(); i != end; i++)
    {
        sett->setValue("usr" + QString::number(j), i.key());
        j++;
    }
    sett->endGroup();

    sett->beginGroup(KEYS::Config().server_settings);
    sett->setValue(KEYS::Config().port, m_port);
    sett->setValue(KEYS::Config().max_user, m_max_users);
    sett->endGroup();

    sett->beginGroup(KEYS::Config().updates);
    sett->setValue(KEYS::Config().update_path, m_updates_path);
    sett->endGroup();
}
