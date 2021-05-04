#include "server.h"
#include<QSettings>
/* Что осталось доделать:
    1) кикать полльзователя если он > минуты не присылает свое имя.
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
    if(sett.isWritable()){
        sett.setIniCodec("UTF-8");
        port = static_cast<quint16>(sett.value("SERVER_SETTINGS/port", 9999).toUInt());
        maxUsers = static_cast<quint8>(sett.value("SERVER_SETTINGS/max_user", 5).toUInt());

        // Инициализация списка разрешенных пользователей
        sett.beginGroup("USER_LIST");
        QStringList iniList = sett.childKeys();
        for (auto i : iniList){
            auto name = sett.value(i, "no_data").toString().toLower();
            m_userList.insert(name, new UserInf()); // FIXME так норм с указателем?
        }
        sett.endGroup();
        // Инициализация списка ресурсов. FIXME: Мб стоит использовать имена ресурсов тоже, но пока без этого.
        sett.beginGroup("RESOURCE_LIST");
        iniList = sett.childKeys();
        for(quint8 i=0; i<iniList.size(); i++){
            m_resList.insert(i, new ResInf(0, "Free"));
        }
        sett.endGroup();
   }else{
        qDebug() << "ini file cant oppen";
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


void Server::slotReadClient(){
    qDebug() << "Ready read";
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender()); // FIXME тут надо QSharedPointer?
    qint64 curByteNum = clientSocket->bytesAvailable();
    if(curByteNum <= READ_BLOCK_SIZE){
        buff.append(clientSocket->read(curByteNum));
    }else{
        for(int i=0; i<=curByteNum/READ_BLOCK_SIZE; i=i+READ_BLOCK_SIZE){
            buff.append( clientSocket->read(READ_BLOCK_SIZE) );
        }
    }
    // FIXME надо добавить то, что если буфер превышает по размеру какое-то значение полностью его очищать
    auto jDoc = QJsonDocument::fromJson(buff, &jsonErr);
    qDebug() << jsonErr.errorString();
    if(jsonErr.error == QJsonParseError::UnterminatedObject){
        qDebug() << jsonErr.errorString();
        return;
    }
    if(jsonErr.error == QJsonParseError::NoError){
        qDebug() << jDoc.toJson();
        auto address = clientSocket->peerAddress();
        json_handler(jDoc.object(), address, *clientSocket);
        buff.clear();
    }
}


void Server::json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket){
    if(!m_blockIp.contains(clientIp)){
        if( m_userList.contains(jObj["username"].toString()) ){ // Если имя клиента есть в списке
            qDebug() << "Name correct";            
            auto jType = jObj["type"].toString();
            if(jType == "clear_res"){   // FIXME можно сделать enum по индексам и через switch  либо через мапу
                all_res_clear();
            }
            if(jType == "service_info"){
                service_handler(jObj);
            }
            if(jType == "res_request"){
                res_req_handler(jObj);
            }
            if(jType == "authorization"){
                new_client_autorization(clientSocket, jObj["username"].toString());
                return;
            }
            // отправка всем пользователям актуальной инфы
            // send_to_all_clients();
        }else{ // Если нет, то штраф сердечко
            m_blockIp.insert(clientIp);
            QJsonObject jObj;
            jObj.insert("type", "connect_fail");
            send_to_client(clientSocket, jObj);
            clientSocket.abort();
            // Отправка qmessagebox о том что сосать, а не подключение
            qDebug() << "User is not contained in list";
        }
    }else{
        // Отправка сообщения о том что сосать, а не подключение.
        QJsonObject jObj;
        jObj.insert("type", "connect_fail");
        send_to_client(clientSocket, jObj);
        clientSocket.disconnectFromHost();
        qDebug() << "User ip in banlist";
    }
}


void Server::slotDisconnected(){
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << clientSocket << " disconnected";
    // FIXME сначала  удалить указатель из контейнера, потом закрыть сокет правильно, потом удалить сокет.
    for(auto i : m_userList){
        if(clientSocket == i->socket) // FIXME это сравнение будет нормально работать или сравняться указатели?
            i->socket = nullptr;
    }
    clientSocket->abort(); // FIXME возможно здесь надо исользовать close()?
    clientSocket->deleteLater();
}


// Первое подключение пользователя. Ему отправляется время старта сервера и список ресурсов и кто их занимает и время.
void Server::new_client_autorization(QTcpSocket &sock, const QString &newUsrName){    
    m_userList[newUsrName]->socket = &sock; // FIXME здесь норм &??

    QJsonObject jObj;
    QJsonArray resNum, resUser, resTime;
    for(quint8 i=0; i<m_resList.size(); i++){
        resNum.push_back(QJsonValue(i));
        resUser.push_back(QJsonValue(m_resList.value(i)->currenUser));
        resTime.push_back(QJsonValue(m_resList.value(i)->time->toString("hh:mm:ss")));
    }
    jObj.insert("type", "authorization");
    jObj.insert("start_time", startServTime);
    jObj.insert("resnum", resNum);
    jObj.insert("resuser", resUser);
    jObj.insert("busyTime", resTime);
    send_to_client(sock, jObj);
    //send_to_client(*m_userList[newUsrName]->socket, jObj); // тоже работает
}


// Очистка всех ресурсов от пользователей и отправка уведомлений о том что ресурс забрали.
void Server::all_res_clear(){
    QJsonObject jObj;
    QString usrName;
    for(quint8 i = 0; i < m_resList.size(); ++i){
        if(m_resList[i]->currenUser != "Free"){
            usrName = m_resList.value(i)->currenUser;
            if(m_grabRes.contains(usrName)){
                m_grabRes[usrName]->push_back(i);
            }else{
                m_grabRes.insert(usrName, new QJsonArray);
                m_grabRes[usrName]->push_back(i);
            m_resList[i]->currenUser = "Free";
            m_resList[i]->time = nullptr;
            }
        }
    }
    QMap<QString, QJsonArray*>::const_iterator i;
    QJsonObject oldUserObj;
    for(i = m_grabRes.begin(); i != m_grabRes.end(); ++i){
        oldUserObj.insert("type", "grab_res");
        oldUserObj.insert("resource", *i.value());
        send_to_client(*m_userList[i.key()]->socket, oldUserObj);
    }
    m_grabRes.clear();
}


// Служебные делишкишы. Установка максимального времени удержания ресурса и блокировка новых соединений.
void Server::service_handler(const QJsonObject &jObj){
    if(jObj["action"].toString()== "occupancy_time"){
        maxBusyTime = static_cast<quint64>(jObj["value"].toInt());
    }

    if(jObj["action"].toString()== "reject_connections"){
        if(jObj["value"].toInt())
            m_server->pauseAccepting();
        else
            m_server->resumeAccepting();
    }

    if(jObj["action"].toString()== "reject_res_req"){
        if(jObj["value"].toInt())
            reject_res_req = true;
        else
            reject_res_req = false;
    }
}


// Запрос/освобождение ресурсов
void Server::res_req_handler(const QJsonObject &jobj){
    QJsonObject servNotice; // Ответ сервера обратившемуся клиенту
    QJsonArray resNum, resStatus;
    quint64 usrTime = static_cast<quint64>(jobj["time"].toInt());
    quint32 reqRes = static_cast<quint32>(jobj["request"].toInt());
    quint32 curRes;
    if(jobj["action"].toString() == "take"){
            // Проверка какие ресурсы хочет пользователь
            auto usrName = jobj["username"].toString();
            for(quint8 i=0; i<m_resList.size(); i++){
                curRes = (reqRes >> (i*8)) & 0xFF;
                if(curRes > 0){
                    //int diffTime = QTime::currentTime().secsTo(*(m_resList[i]->time)); // так можно делать?
                    qint64 diffTime = m_resList[i]->time->secsTo(QTime::currentTime());
                    // Если ресурс свободен
                    if(m_resList.value(i)->currenUser == "Free" && !reject_res_req){
                        m_resList.insert( i, new ResInf(usrTime, usrName));
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(1);
                        // вызов потоко-небезопасной функции
                        registr(usrName.toUtf8().constData(), i);
                    }else if(diffTime > maxBusyTime && !reject_res_req){
                        QString old_user = m_resList.value(i)->currenUser;
                        m_resList.insert( i, new ResInf(usrTime, usrName));
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(1);
                        // вызов потоко-небезопасной функции
                        registr(usrName.toUtf8().constData(), i);
                        // Составление списка пользователей у которых забрали ресурсы.
                        if(m_grabRes.contains(old_user)){
                            m_grabRes[old_user]->push_back(i);
                        }else{
                            m_grabRes.insert(old_user, new QJsonArray);
                            m_grabRes[old_user]->push_back(i);
                        }
                    }else{
                        resNum.push_back(QJsonValue(i));
                        resStatus.push_back(0);
                    }
                }
            }

            // Отправка всем пользователям у которых забрали ресурс список какие ресурсы у них забрали.
            QMap<QString, QJsonArray*>::const_iterator i;
            QJsonObject oldUserObj;
            for(i = m_grabRes.begin(); i != m_grabRes.end(); ++i){
                oldUserObj.insert("type", "grab_res");
                oldUserObj.insert("resource", *i.value());
                send_to_client(*m_userList[i.key()]->socket, oldUserObj);
            }
            m_grabRes.clear();

            servNotice.insert("type", "request_responce");
//            servNotice.insert("username", usrName); // FIXME это надо?
            servNotice.insert("resource", resNum);
            servNotice.insert("status", resStatus);
            send_to_client(*m_userList[usrName]->socket, servNotice);
    }else if(jobj["action"].toString() == "free"){
        for(quint8 i=0; i<m_resList.size(); i++){
            curRes = (reqRes >> (i*8)) & 0xFF;
            if(curRes > 0){
                m_resList.insert( i, new ResInf()); // FIXME так не будет утечки памяти?
            }
        }
    }else{
        qDebug() << "Error. Not take or free res";
    }    
}


void Server::send_to_client(QTcpSocket &sock, const QJsonObject &jObj){
    if(sock.state() == QAbstractSocket::ConnectedState){
        QJsonDocument jDoc(jObj);
        sock.write(jDoc.toJson());
    }else{
        qDebug() << "Socket not connected";
    }
}


// Обновление данных о ресурсах/пользователях у всех клиентов. Наверное так делать не совсем верно.
void Server::send_to_all_clients(){
    QJsonObject jObj;
    QJsonArray resNum, resUser, resTime;
    for(quint8 i=0; i<m_resList.size(); i++){
        resNum.push_back(QJsonValue(i));
        resUser.push_back(QJsonValue(m_resList.value(i)->currenUser));
        resTime.push_back(QJsonValue(m_resList.value(i)->time->toString("hh:mm:ss")));
    }
    jObj.insert("type", "broadcast");
    jObj.insert("resnum", resNum);
    jObj.insert("resuser", resUser);
    jObj.insert("busyTime", resTime);

    QJsonDocument jDoc(jObj);
    if( m_userList["egor"]->socket->isValid() ){
        m_userList["egor"]->socket->write(jDoc.toJson());
    }

//    QJsonDocument jDoc(jObj);
//    for(auto i : m_userList){
//        if(i->socket && i->socket->isValid()){
//            i->socket->write(jDoc.toJson());
//        }
//    }
}


bool Server::registr(const std::string &username, uint32_t resource_index){
    // потоко-небезопасная функция
    mutex.lock();
    // действия
    mutex.unlock();
    return true;
}


