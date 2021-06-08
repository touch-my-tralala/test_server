#include "server.h"

/* Что осталось доделать:
 * 1) лог
 * 2) время последней транзакции от клиента
 * 3) вместо строчных литералов сделать структуру ключей
 * 4) заполнение ini файла не верно работает.
*/

Server::Server()
{
    Server::ini_parse("init.ini");
    qDebug() << "port = " << port;
    qDebug() << "max_user = " << maxUsers;

    m_server.setMaxPendingConnections(maxUsers);

    if(!m_server.listen(QHostAddress::Any, port)){
        qDebug() << "Error listening";
        return;
    }else{
        qDebug() << "Server listening port " << port;
    }
    connect(&m_server, &QTcpServer::newConnection,
            this, &Server::on_slotNewConnection);

    startServTime = QDateTime::currentDateTime();
}

Server::~Server(){
    QStringList saveList;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        saveList << QString::number(i.key());
    }
    sett->setValue("RESOURCE_LIST", saveList);
    saveList.clear();
    for(auto i =m_userList.begin(); i != m_userList.end(); i++){
        saveList << i.key();
    }
    sett->setValue("USER_LIST", saveList);

    if(m_server.isListening()){
        m_server.close();
    }
}

void Server::setTimeOut(qint64 secs){
    maxBusyTime = secs;
}

void Server::setMaxUser(quint8 maxUsers){
    this->maxUsers = maxUsers;
}

void Server::setRejectConnection(bool a){
    if(a)
        m_server.pauseAccepting();
    else
        m_server.resumeAccepting();
}

void Server::setRejectResReq(bool a){
    reject_res_req = a;
}

QList<quint8> Server::getResList(){
    QList<quint8> resList;
    for(auto i = 0; i < m_resList.size(); i++){
        resList << static_cast<quint8>(i);
    }
    return resList;
}

QStringList Server::getUserList(){
    QStringList usrList;
    for(auto i = m_userList.begin(); i != m_userList.end(); i++){
        usrList << i.key();
    }
    return usrList;
}

QString Server::getResUser(quint8 resNum){
    if(m_resList.contains(resNum))
        return m_resList[resNum].currentUser;
    else
        return "not contain a key";
}

// возвращает время в секундах прошедшее с момента занятия ресурса
qint32 Server::getBusyResTime(quint8 resNum){
    if(m_resList.contains(resNum)){
        if(m_resList[resNum].currentUser == "Free")
            return 0;
        else
            return m_resList[resNum].time.secsTo(QTime::currentTime());
    }else{
        return -1;
    }
}

const QDateTime& Server::getStartTime() const{
    return startServTime;
}

void Server::allResClear(){
    QJsonObject jObj;
    QString usrName;
    for(quint8 i = 0; i < m_resList.size(); ++i){
        if(m_resList[i].currentUser != "Free"){
            usrName = m_resList[i].currentUser;
            // FIXME проверить будет ли работать.
            m_grabRes[usrName].push_back(i);
            m_resList[i].currentUser = "Free";
            m_resList[i].time.setHMS(0, 0, 0);
        }
    }
    QJsonObject oldUserObj({
                           {"type", "grab_res"}
                           });
    for(auto i = m_grabRes.begin(); i != m_grabRes.end(); ++i){        
        oldUserObj.insert("resource", i.value());
        send_to_client(*m_userList[i.key()].socket, oldUserObj);
    }
    m_grabRes.clear();
}

void Server::addNewRes(quint8 resNum){
    if(!m_resList.contains(resNum)){
        m_resList.insert(resNum, ResInf());
    }
}

void Server::addNewUsrName(QString name){
    if(!m_userList.contains(name)){
        m_userList.insert(name, UserInf());
    }
}

void Server::removeRes(quint8 resNum){
    if(m_resList.contains(resNum)){
        m_resList.remove(resNum);
    }
}

void Server::removeUsr(QString name){
    if(m_userList.contains(name)){
        m_userList.remove(name);
    }
}


void Server::ini_parse(QString fname){
    sett = QSharedPointer<QSettings>(new QSettings(QDir::currentPath() + "/" + fname, QSettings::IniFormat)); // FIXME хз как не использовать указатель. не понял как сменить путь.
    if(sett->isWritable()){
        sett->setIniCodec("UTF-8");
        sett->beginGroup("SERVER_SETTINGS");
        if(sett->contains("port")){
            port = static_cast<quint16>(sett->value("port", 9292).toUInt());
        }else{
            port = 9292;
            sett->setValue("port", 9292);
        }

        if(sett->contains("max_user")){
            maxUsers = static_cast<quint8>(sett->value("max_user", 5).toUInt());
        }else{
            maxUsers = 5;
            sett->setValue("max_user", 5);
        }
        sett->endGroup();

        QStringList iniList;
        // Инициализация списка разрешенных пользователей
        if(sett->contains("USER_LIST")){
            sett->beginGroup("USER_LIST");
             iniList = sett->childKeys();
            //for (QString &i : iniList){
            for (auto i : iniList){
                auto name = sett->value(i, "no_data").toString().toLower();
                m_userList.insert(name, UserInf());
            }
            sett->endGroup();
        }

        if(sett->contains("RESOURCE_LIST")){
            sett->beginGroup("RESOURCE_LIST");
            iniList = sett->childKeys();
            for(quint8 i=0; i<iniList.size(); i++){
                m_resList.insert(i, ResInf());
            }
            sett->endGroup();
        }
   }else{
        qDebug() << "ini is read-only";
    }
}


void Server::on_slotNewConnection(){
    QTcpSocket* clientSocket = m_server.nextPendingConnection(); // FIXME эту хрень так оставить или надо удалять?
    qDebug() << "New connection";
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &Server::on_slotDisconnected);
    connect(clientSocket, &QTcpSocket::readyRead,
            this, &Server::on_slotReadClient);
}


void Server::on_slotReadClient(){
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    qint64 curByteNum = clientSocket->bytesAvailable();
    if(curByteNum <= READ_BLOCK_SIZE){
        buff.append(clientSocket->read(curByteNum));
    }else{
        for(int i=0; i<=curByteNum/READ_BLOCK_SIZE; i=i+READ_BLOCK_SIZE){
            buff.append( clientSocket->read(READ_BLOCK_SIZE) );
        }
    }

    if(buff.size() > 1048576){
        qDebug() << "Buffer size > 1 Mb";
        buff.clear();
        return;
    }

    jDoc = QJsonDocument::fromJson(buff, &jsonErr);
    if(jsonErr.error == QJsonParseError::NoError){
        auto address = clientSocket->peerAddress();
        json_handler(jDoc.object(), address, *clientSocket);
        buff.clear();
    }else{
        qDebug() << jsonErr.errorString();
    }
}


void Server::on_slotDisconnected(){
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    qDebug() << clientSocket << " disconnected";
    // FIXME возможно надо проверять теперь на валидность / подключенность сокета
    clientSocket->abort();
    clientSocket->deleteLater();
}


void Server::json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket){
    if(!m_blockIp.contains(clientIp)){
        QString name = jObj["username"].toString();
        if(m_userList.contains(name)){ // Если имя клиента есть в списке

            if(!jObj.contains("action")){
                new_client_autorization(clientSocket, name);
            }else{
                if(jObj["action"].toString() == "take")
                    res_req_take(jObj);
                else if(jObj["action"].toString() == "free")
                    res_req_free(jObj);
                else
                    qDebug() << "res_request not contain action: " + jObj["action"].toString();
            }
            // отправка всем пользователям актуальной инфы
            send_to_all_clients();

        }else{
            m_blockIp.insert(clientIp);
            QJsonObject jObj({
                             {"type", "connect_fail"}
                             });
            send_to_client(clientSocket, jObj);
            clientSocket.abort();
            qDebug() << "User is not contained in list";
        }
    }else{
        // Отправка сообщения о том что сосать, а не подключение.
        QJsonObject jObj({
                         {"type", "connect_fail"}
                         });
        send_to_client(clientSocket, jObj);
        clientSocket.disconnectFromHost();
        qDebug() << "User ip in banlist";
    }
}


// Первое подключение пользователя. Ему отправляется время старта сервера и список ресурсов и кто их занимает и время.
void Server::new_client_autorization(QTcpSocket &sock, const QString &newUsrName){
    m_userList[newUsrName].socket = &sock; // FIXME здесь норм &??

    //QJsonObject jObj;
    QJsonArray resNum, resUser, resTime;
    for(quint8 i=0; i<m_resList.size(); i++){
        resNum.push_back(i);
        resUser.push_back(m_resList[i].currentUser);
        resTime.push_back(m_resList[i].time.toString("hh:mm:ss"));
    }
    QJsonObject jObj({
                     {"type", "authorization"},
                     {"resnum", resNum},
                     {"resuser", resUser},
                     {"busyTime", resTime}
                     });
    send_to_client(sock, jObj);
}


// Запрос ресурсов
void Server::res_req_take(const QJsonObject &jObj){
    QJsonArray resNumReq;   // номер запрашиваемого ресурса
    QJsonArray resStatus;   // ответ на запрашиваемый ресурс
    QJsonArray resNum;      // индекс ресурса (все, не только запрашиваемые)
    QJsonArray resUser;     // текущий владелец ресурса
    QJsonArray resTime;     // время, которое текущий пользователь занимает ресурс
    qint32 usrTime = static_cast<qint32>(jObj["time"].toInt());
    qint32 reqRes = static_cast<qint32>(jObj["request"].toInt());
    qint32 curRes = 0;
    QString usrName = jObj["username"].toString();
    for(quint8 i=0; i<m_resList.size(); i++){
        curRes = (reqRes >> (i*8)) & 0xFF;
        if(curRes){
            qint64 diffTime = m_resList[i].time.secsTo(QTime::currentTime());
            // Если ресурс свободен
            if(m_resList.value(i).currentUser == "Free" && !reject_res_req){
                m_resList[i].currentUser = usrName;
                m_resList[i].time = QTime(0, 0, 0).addSecs(usrTime);
                resNumReq.push_back(i);
                resStatus.push_back(1);
                // вызов потоко-небезопасной функции
                registr(usrName.toUtf8().constData(), i);
            // ресурс занят, но по таймауту можно перехватить
            }else if(diffTime > maxBusyTime && !reject_res_req){
                QString old_user = m_resList.value(i).currentUser;
                m_resList[i].currentUser = usrName;
                m_resList[i].time = QTime(0, 0, 0).addSecs(usrTime);
                resNumReq.push_back(i);
                resStatus.push_back(1);
                // вызов потоко-небезопасной функции
                registr(usrName.toUtf8().constData(), i);

                // Составление списка пользователей у которых забрали ресурсы.
                if(m_grabRes.contains(old_user)){
                    m_grabRes[old_user].push_back(i);
                }else{
                    m_grabRes.insert(old_user, QJsonArray());
                    m_grabRes[old_user].push_back(i);
                }
            // ресурс занят
            }else{
                resNumReq.push_back(i);
                resStatus.push_back(0);
            }
        }
        resNum.push_back(i);
        resUser.push_back(m_resList[i].currentUser);
        resTime.push_back(m_resList[i].time.toString("hh:mm:ss"));
    }

    // Отправка всем пользователям у которых забрали ресурс список какие ресурсы у них забрали.
    QJsonObject oldUserObj;
    oldUserObj.insert("type", "grab_res");
    for(auto i = m_grabRes.begin(); i != m_grabRes.end(); ++i){
        oldUserObj.insert("resource", *i);
        send_to_client(*m_userList[i.key()].socket, oldUserObj); // эта хуйня сломается наверное при отправке send_all_client
    }
    m_grabRes.clear();

    QJsonObject servNotice({
                               {"type", "request_responce"},
                               {"action", "take"},
                               {"resource_responce", resNumReq},
                               {"status", resStatus}
                           });
    send_to_client(*m_userList[usrName].socket, servNotice);
}


// Освобождение ресурсов
void Server::res_req_free(const QJsonObject &jObj){
    QJsonArray resNumReq;   // номер запрашиваемого ресурса
    QJsonArray resStatus;   // ответ на запрашиваемый ресурс
    QJsonArray resNum;      // индекс ресурса (все, не только запрашиваемые)
    QJsonArray resUser;     // текущий владелец ресурса
    QJsonArray resTime;     // время, которое текущий пользователь занимает ресурс
    qint64 reqRes = static_cast<qint64>(jObj["request"].toInt());
    qint64 curRes = 0;
    QString usrName = jObj["username"].toString();
    for(quint8 i=0; i<m_resList.size(); i++){
        curRes = (reqRes >> (i*8)) & 0xFF;
        if(curRes && m_resList[i].currentUser == usrName){
            m_resList[i].currentUser = "Free";
            m_resList[i].time.setHMS(0, 0, 0);
            resNumReq.push_back(i);
            resStatus.push_back(1);
        }else if(curRes){
            resNumReq.push_back(i);
            resStatus.push_back(0);
        }
        resNum.push_back(i);
        resUser.push_back(m_resList[i].currentUser);
        resTime.push_back(m_resList[i].time.toString("hh:mm:ss"));
    }

    QJsonObject servNotice({
                               {"type", "request_responce"},
                               {"action", "free"},
                               {"resource_responce", resNumReq},
                               {"status", resStatus},
                               {"resnum", resNum},
                               {"resuser", resUser},
                               {"busyTime", resTime}
                           });
    send_to_client(*m_userList[usrName].socket, servNotice);
}


void Server::send_to_client(QTcpSocket &sock, const QJsonObject &jObj){
    if(sock.state() == QAbstractSocket::ConnectedState){
        QJsonDocument jsonDoc(jObj);
        sock.write(jsonDoc.toJson(QJsonDocument::Compact));
    }else{
        qDebug() << "Socket not connected";
    }
}


// Обновление данных о ресурсах/пользователях у всех клиентов. Наверное так делать не совсем верно.
void Server::send_to_all_clients(){
    QJsonArray resNum, resUser, resTime;
    for(quint8 i=0; i<m_resList.size(); i++){
        resNum.push_back(QJsonValue(i));
        resUser.push_back(QJsonValue(m_resList.value(i).currentUser));
        resTime.push_back(QJsonValue(m_resList.value(i).time.toString("hh:mm:ss")));
    }
    QJsonObject jObj({
                     {"type", "broadcast"},
                     {"resnum", resNum},
                     {"resuser", resUser},
                     {"busyTime", resTime}
                     });
    QJsonDocument jsonDoc(jObj);
    for(auto i = m_userList.begin(); i != m_userList.end(); ++i){
        if(i.value().socket && i.value().socket->state() == QTcpSocket::ConnectedState ){
            i.value().socket->write(jsonDoc.toJson(QJsonDocument::Compact));
        }
    }
}


bool Server::registr(const std::string &username, uint32_t resource_index){
    Q_UNUSED(username)
    Q_UNUSED(resource_index)
    // потоко-небезопасная функция
    mutex.lock();
    // действия
    mutex.unlock();
    return true;
}


