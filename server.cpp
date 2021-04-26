#include "server.h"

/* Что осталось доделать:
    1) кикать полльзователя если он > минуты не присылает свое имя.
    2) почему сервер закрывается после принятия первого сообщения?
*/

Server::Server()
{
    Server::ini_parse("init.ini");
    qDebug() << "port=" << port;
    qDebug() << "max_user=" << maxUsers;

    m_server = QSharedPointer<QTcpServer>(new QTcpServer(this));
    m_server->setMaxPendingConnections(maxUsers);

    if(!m_server->listen(QHostAddress::Any, port)){
        qDebug() << "Error listening";
        m_server->close();
        return;
    }else{
        qDebug() << "Server listening port " << port;
    }
    connect(m_server.data(), &QTcpServer::newConnection,
            this, &Server::slotNewConnection);

    startServTime = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss");
}

Server::~Server(){
    if(m_server->isListening()){
        m_server->close();
    }
}

void Server::ini_parse(QString fname){
    QSettings sett(QDir::currentPath() + "/" + fname, QSettings::IniFormat);
    sett.setIniCodec("UTF-8");
    port = static_cast<quint16>(sett.value("SERVER_SETTINGS/port", 9999).toUInt());
    maxUsers = static_cast<quint8>(sett.value("SERVER_SETTINGS/max_user", 5).toUInt());
    // Инициализация списка разрешенных пользователей
    sett.beginGroup("USER_LIST");
    QStringList iniList = sett.childKeys();
    QStringList::const_iterator i;
    for (i = iniList.begin();i != iniList.end(); ++i){ // FIXME разобраться почему преинкремент
        auto name = sett.value(*i, "no_data").toString().toLower();
        m_userList.insert(name, QSharedPointer<UserInf>(new UserInf()));
    }
    // Инициализация списка ресурсов. FIXME: Мб стоит использовать имена ресурсов тоже, но пока без этого.
    sett.beginGroup("RESOURCE_LIST");
    iniList = sett.childKeys();
    for(quint8 i=0; i<iniList.size(); i++){
        m_resList.insert(i, QSharedPointer<ResInf>(new ResInf()));
    }
}


void Server::slotNewConnection(){
    QTcpSocket* clientSocket = m_server->nextPendingConnection();
    qDebug() << "New connection";
//    connect(clientSocket, &QTcpSocket::disconnected,
//            clientSocket, &QTcpSocket::deleteLater);
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &Server::slotDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &Server::slotReadClient);
}


void Server:: slotReadClient(){
    QSharedPointer<QTcpSocket> clientSocket = QSharedPointer<QTcpSocket>(static_cast<QTcpSocket*>(sender()));
    //QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    auto jDoc = QJsonDocument::fromJson(clientSocket->readAll(), &jsonErr);
    QJsonObject json = jDoc.object();
    // Получение адреса клиентского сокета
    auto address = clientSocket->peerAddress();
    if(!m_blockIp.contains(address)){
        if(jsonErr.errorString() == "no error occurred"){
            if( m_userList.contains(json["username"].toString()) ){ // Если имя клиента есть в списке
                qDebug() << "Name correct";
                if(json.size() > 1){ // если это не первичная авторизация, а запрос ресурсов
                    m_userList.value(json["username"].toString())->socket = clientSocket;
                    if(json["type"].toString() == "clear_res"){
                        all_res_clear();
                    }
                    if(json["type"].toString() == "service_info"){
                        service_handler(json);
                    }
                    if(json["type"].toString() == "res_request"){
                        res_req_handler(json);
                    }
                    // отправка всем пользователям актуальной инфы
                    send_to_all_clients();
                }else{
                    // При первичной авторизации отправляется время запуска сервера.
                    QJsonObject jObj;
                    jObj.insert("start_time", startServTime);
                    send_to_client(*clientSocket, jObj);
                }
            }else{ // Если нет, то штраф сердечко
                m_blockIp.insert(address);
                QJsonObject jObj;
                jObj.insert("type", "connect fail");
                send_to_client(*clientSocket, jObj);
                clientSocket->disconnectFromHost();
                // Отправка qmessagebox о том что сосать, а не подключение
                qDebug() << "User is not contained in list";
            }
        }else{ // Ошибка json формата            
            qDebug() << "Json format error " << jsonErr.errorString();
        }
    }else{
        // Отправка сообщения о том что сосать, а не подключение.
        QJsonObject jObj;
        jObj.insert("type", "connect fail");
        send_to_client(*clientSocket, jObj);
        clientSocket->disconnectFromHost();
        qDebug() << "User ip in banlist";
    }
}

void Server::slotDisconnected(){
    QSharedPointer<QTcpSocket> clientSocket = QSharedPointer<QTcpSocket>(static_cast<QTcpSocket*>(sender()));
   //QTcpSocket* clientSocket = static_cast<QTcpSocket*>(sender());
    QMap<QString, QSharedPointer<UserInf>>::const_iterator i;
    for(i = m_userList.constBegin(); i != m_userList.constEnd(); ++i){
        if(clientSocket == i.value()->socket)
            i.value()->socket = nullptr;
    }
    //clientSocket->deleteLater();
    clientSocket.clear();
}

// Очистка всех ресурсов от пользователей и отправка уведомлений о том что ресурс забрали.
void Server::all_res_clear(){
    QJsonObject jObj;
    QString usrName;
    for(quint8 i = 0; i < m_resList.size(); ++i){
        if(m_resList.value(i)->currenUser != "Free"){
            usrName = m_resList.value(i)->currenUser;
            jObj.insert("type", "grab_res");
            jObj.insert("resource", i);
            send_to_client(*(m_userList.value(usrName)->socket), jObj);
            m_resList.value(i)->currenUser = "Free";
            m_resList.value(i)->time = nullptr;
        }
    }
}

// Служебные делишкишы. Установка максимального времени удержания ресурса и блокировка новых соединений.
void Server::service_handler(const QJsonObject &jObj){
    if(jObj["action"].toString()== "occupancy_time"){
        maxBusyTime = static_cast<quint16>(jObj["value"].toInt());
    }

    if(jObj["action"].toString()== "banning_connections"){
        if(jObj["value"].toInt() == 1)
            m_server->pauseAccepting();
        else
            m_server->resumeAccepting();
    }

    if(jObj["action"].toString()== "reject_res_req"){
        if(jObj["value"].toInt() == 1)
            reject_res_req = true;
        else
            reject_res_req = false;
    }
}

// Запрос/освобождение ресурсов
void Server::res_req_handler(const QJsonObject &jobj){
    QJsonObject servNotice; // Ответ сервера обратившемуся клиенту
    QJsonArray resNum, resStatus;
    quint32 usrTime = static_cast<quint32>(jobj["time"].toInt());
    quint32 reqRes = static_cast<quint32>(jobj["request"].toInt());
    quint32 curRes;
    if(jobj["action"].toString() == "take"){
            // Проверка какие ресурсы хочет пользователь
            auto usrName = static_cast<QString>(jobj["username"].toString()); // FIXME мб можно без пересоздания объекта?
            for(quint8 i=0; i<m_resList.size(); i++){
                curRes = (reqRes >> (i*8)) & 0xFF;
                if(curRes > 0){
                    int diffTime = QTime::currentTime().secsTo(*(m_resList.value(i)->time)); // так можно делать?
                    // Если ресурс свободен
                    if(m_resList.value(i)->currenUser == "Free" && !reject_res_req){
                        m_resList.insert( i, QSharedPointer<ResInf>(new ResInf(usrTime, usrName)) );
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(1);
                        // вызов потоко-небезопасной функции
                        registr(usrName.toUtf8().constData(), i);
                    }else if(diffTime > (maxBusyTime * 3600) && !reject_res_req){
                        QString old_user = m_resList.value(i)->currenUser;
                        m_resList.insert( i, QSharedPointer<ResInf>(new ResInf(usrTime, usrName)) );
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(1);
                        registr(usrName.toUtf8().constData(), i);
                        // FIXME уведомление у пользователя забрали ресурс
                        QJsonObject oldUserObj;
                        oldUserObj.insert("type", "grab_res");
                        oldUserObj.insert("resource", i);
                        send_to_client(*(m_userList.value(old_user)->socket), oldUserObj); // уведомление, что ресурс был перехвачен.
                    }else{
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(0);
                    }
                }
            }
            servNotice.insert("type", "request_responce");
            servNotice.insert("username", usrName);
            servNotice.insert("resource", resNum);
            servNotice.insert("status", resStatus);
            send_to_client(*(m_userList.value(usrName)->socket),
                           servNotice);
    }else if(jobj["action"].toString() == "free"){
        for(quint8 i=0; i<m_resList.size(); i++){
            curRes = (reqRes >> (i*8)) & 0xFF;
            if(curRes > 0){
                m_resList.insert( i, QSharedPointer<ResInf>(new ResInf()) );
            }
        }
    }else{
        qDebug() << "Error. Not take or free res";
    }    
}

void Server::send_to_client(QTcpSocket &sock, const QJsonObject &jObj){
    if(sock.state() == QTcpSocket::ConnectedState){
        QJsonDocument jDoc(jObj);
        sock.write(jDoc.toJson());
    }else{
        qDebug() << "Socket not connected";
    }
}

// Обновление данных о ресурсах/пользователях у всех клиентов. Наверное так делать не совсем верно.
void Server::send_to_all_clients(){
    QJsonObject jObj;
    QJsonArray resNum, resUser;
    QMap<quint8, QSharedPointer<ResInf>>::const_iterator i;
    for(i = m_resList.constBegin(); i != m_resList.constEnd(); ++i){
        resNum.push_back(QJsonValue(*i));
        resUser.push_back(QJsonValue(i.value()->currenUser));
    }
    jObj.insert("type", "broadcast");
    jObj.insert("resnum", resNum);
    jObj.insert("resuser", resUser);
    QJsonDocument jDoc(jObj);
    QMap<QString, QSharedPointer<UserInf>>::const_iterator j;
    for(j = m_userList.constBegin(); j != m_userList.constEnd(); ++j){
        if(j.value()->socket->state() == QTcpSocket::ConnectedState){
            j.value()->socket->write(jDoc.toJson());
        }

    }
}

bool Server::registr(const std::string &username, uint32_t resource_index){
    // потоко-небезопасная функция
    mutex.lock();
    // действия
    mutex.unlock();
    return true;
}


