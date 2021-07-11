#include "server.h"

/* Что осталось доделать:
 * 2) время последней транзакции от клиента
 * 3) вместо строчных литералов сделать структуру ключей
 * 4) заполнение ini файла не верно работает.
*/

Server::Server()
{
    Server::ini_parse("init.ini");

    emit signalLogEvent("Server → Максимальное количество пользователей - " + QString::number(maxUsers) + ".");

    m_server.setMaxPendingConnections(maxUsers);

    if(!m_server.listen(QHostAddress::Any, port)){
        emit signalLogEvent("ОШИБКА → Прослушивание порта невозможно.");
        return;
    }else{
        emit signalLogEvent("Server → порт - " + QString::number(port) + ".");
    }
    connect(&m_server, &QTcpServer::newConnection,
            this, &Server::on_slotNewConnection);

    startServTime = QDateTime::currentDateTime();
    emit signalLogEvent("Server → Время старта: " + startServTime.toString("hh:mm:ss."));
}

Server::~Server(){
    quint16 j = 0;
    sett->beginGroup(JSON_KEYS::Common().resource_list);
    for(auto i = m_resList.begin(), end = m_resList.end(); i != end; i++){
        sett->setValue("res" + QString::number(j), i.key());
        j++;
    }
    sett->endGroup();

    j = 0;
    sett->beginGroup(JSON_KEYS::Common().user_list);
    for(auto i =m_userList.begin(), end = m_userList.end(); i != end; i++){
        sett->setValue("usr" + QString::number(j), i.key());
        j++;
    }    
    sett->endGroup();

    if(m_server.isListening()){
        m_server.close();
    }
}

void Server::setTimeOut(qint64 secs){
    maxBusyTime = secs;
    emit signalLogEvent("Server → Тайм-аут " + QTime(0,0,0).addSecs(secs).toString() + " успешно установлен.");
}

void Server::setMaxUser(quint8 maxUsers){
    this->maxUsers = maxUsers;
    emit signalLogEvent("Server → Макс. количество пользователей "
                        + QString::number(maxUsers) + " успешно установлено.");
}

void Server::setRejectConnection(bool a){
    if(a){
        m_server.pauseAccepting();
        emit signalLogEvent("Server → Входящие подключения отклоняются.");
    }else{
        m_server.resumeAccepting();
        emit signalLogEvent("Server → Входящие подключения принимаются.");
    }
}

void Server::setRejectResReq(bool a){
    reject_res_req = a;
    QString s = a ? "Server → Запросы ресурсов отклояются." :
                    "Server → Запросы ресурсов принимаются.";
    emit signalLogEvent(s);
}

QStringList Server::getResList(){
    QStringList resList;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++)
        resList << i.key();
    return resList;
}

QStringList Server::getUserList(){
    QStringList usrList;
    for(auto i = m_userList.begin(); i != m_userList.end(); i++){
        usrList << i.key();
    }
    return usrList;
}

QString Server::getResUser(QString resName){
    if(m_resList.contains(resName))
        return m_resList[resName].currentUser;
    else{
        emit signalLogEvent("ОШИБКА → Пользователя с таким именем нет в списке getResUSer().");
        return "";
    }
}

// возвращает время в секундах прошедшее с момента занятия ресурса
qint32 Server::getBusyResTime(QString resName){
    if(m_resList.contains(resName)){
        if(m_resList[resName].currentUser == JSON_KEYS::State().free)
            return 0;
        else
            return m_resList[resName].time.secsTo(QTime::currentTime());
    }else{
        emit signalLogEvent("ОШИБКА → Ресурса с таким именем нет в списке getBusyResTime().");
        return -1;
    }
}

const QDateTime& Server::getStartTime() const{
    return startServTime;
}

void Server::allResClear(){
    QJsonObject jObj;
    QString usrName;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        if(i.value().currentUser != JSON_KEYS::State().free){
            usrName = i.value().currentUser;
            // FIXME проверить будет ли работать.
            m_grabRes[usrName].push_back(i.key());
            i.value().currentUser = JSON_KEYS::State().free;
            i.value().time.setHMS(0, 0, 0);
        }
    }
    QJsonObject oldUserObj({
                           {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().grab_res}
                           });
    for(auto i = m_grabRes.begin(); i != m_grabRes.end(); ++i){        
        oldUserObj.insert(JSON_KEYS::Common().resource, i.value());
        send_to_client(*m_userList[i.key()].socket, oldUserObj);
    }
    m_grabRes.clear();
    emit signalLogEvent("Server → Очистка всех ресурсов.");
}

void Server::addNewRes(QString resName){
    if(!m_resList.contains(resName)){
        m_resList.insert(resName, ResInf());
        emit signalLogEvent("Server → Ресурс " + resName + " успешно добавлен.");
    }else{
        emit signalLogEvent("ОШИБКА → Ресурс " + resName + "уже есть в списке.");
    }
}

void Server::addNewUsrName(QString name){
    if(!m_userList.contains(name)){
        m_userList.insert(name, UserInf());
        emit signalLogEvent("Server → Пользователь " + name + " успешно добавлен.");
    }else{
        emit signalLogEvent("ОШИБКА → Пользователь " + name + " уже есть в списке.");
    }
}

void Server::removeRes(QString resName){
    if(m_resList.contains(resName)){
        m_resList.remove(resName);
        emit signalLogEvent("Server → Ресурс " + resName + " успешно удален.");
    }else{
        emit signalLogEvent("ОШИБКА → Ресурса "+ resName + " нет в списке удален.");
    }
}

void Server::removeUsr(QString name){
    if(m_userList.contains(name)){
        m_userList.remove(name);
        emit signalLogEvent("Server → Пользователь " + name + " успешно удален.");
    }else{
        emit signalLogEvent("ОШИБКА → Пользователя " + name + " нет в списке.");
    }
}


void Server::ini_parse(QString fname){
    sett = QSharedPointer<QSettings>(new QSettings(QDir::currentPath() + "/" + fname, QSettings::IniFormat));
    if(sett->isWritable()){
        sett->setIniCodec("UTF-8");

        // Инициализация настроек сервера

        sett->beginGroup(JSON_KEYS::Config().server_settings);
        if(sett->contains(JSON_KEYS::Config().port)){
            port = static_cast<quint16>(sett->value(JSON_KEYS::Config().port, 9292).toUInt());
        }else{
            port = 9292;
            sett->setValue(JSON_KEYS::Config().port, 9292);
        }

        if(sett->contains(JSON_KEYS::Config().max_user)){
            maxUsers = static_cast<quint8>(sett->value(JSON_KEYS::Config().max_user, 5).toUInt());
        }else{
            maxUsers = 5;
            sett->setValue(JSON_KEYS::Config().max_user, 5);
        }
        sett->endGroup();

        // Инициализация списка разрешенных пользователей
        QStringList iniList;

        sett->beginGroup(JSON_KEYS::Common().user_list);
        iniList = sett->childKeys();
        for (auto i : iniList){
            auto name = sett->value(i, "no_data").toString().toLower();
            m_userList.insert(name, UserInf());
        }
        sett->endGroup();

        // Инициализация списка ресурсов
        sett->beginGroup(JSON_KEYS::Common().resource_list);
        iniList = sett->childKeys();
        for(auto i: iniList){
            auto name = sett->value(i, "no_data").toString().toLower();
            m_resList.insert(name, ResInf());
        }
        sett->endGroup();

        // Получение текущей версии приложения
        sett->beginGroup(JSON_KEYS::Config().current_version);
        if(sett->contains(JSON_KEYS::Config().version))
            m_cur_version = sett->value(JSON_KEYS::Config().version).toString();

   }else{
        emit signalLogEvent("ОШИБКА → ini файл в режиме read-only.");
    }
}


void Server::on_slotNewConnection(){
    QTcpSocket* clientSocket = m_server.nextPendingConnection(); // FIXME эту хрень так оставить или надо удалять?

    emit signalLogEvent("Server → Новое подключение: Addres - " + clientSocket->peerAddress().toString() +
                        " Port - " + QString::number(clientSocket->peerPort()));
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
        emit signalLogEvent("ОШИБКА → Буфер превышает 1 Мб.");
        buff.clear();
        return;
    }

    jDoc = QJsonDocument::fromJson(buff, &jsonErr);
    if(jsonErr.error == QJsonParseError::NoError){
        auto address = clientSocket->peerAddress();
        json_handler(jDoc.object(), address, *clientSocket);
        buff.clear();
    }else{
        emit signalLogEvent("ОШИБКА → Ошибка json-формата " + jsonErr.errorString() + ".");
    }
}


void Server::on_slotDisconnected(){
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    emit signalLogEvent("Server → Клиент: Addres - " + clientSocket->peerAddress().toString() +
                        " Port - " + QString::number(clientSocket->peerPort()) + " отключился.");
    // FIXME возможно надо проверять теперь на валидность / подключенность сокета
    clientSocket->abort();
    clientSocket->deleteLater();
}


void Server::json_handler(const QJsonObject &jObj, const QHostAddress &clientIp, QTcpSocket &clientSocket){
    if(!m_blockIp.contains(clientIp)){
        QString name = jObj[JSON_KEYS::Common().user_name].toString();
        if(m_userList.contains(name)){ // Если имя клиента есть в списке

            if(!jObj.contains(JSON_KEYS::Action().action)){
                new_client_autorization(clientSocket, name);
            }else{
                if(jObj[JSON_KEYS::Action().action].toString() == JSON_KEYS::Action().take)
                    res_req_take(jObj);
                else if(jObj[JSON_KEYS::Action().action].toString() == JSON_KEYS::Action().leave)
                    res_req_free(jObj);
                else
                    emit signalLogEvent("ОШИБКА → res_request не содержит действия " + jObj[JSON_KEYS::Action().action].toString());
            }
            // отправка всем пользователям актуальной инфы
            send_to_all_clients();

        }else{
            m_blockIp.insert(clientIp);
            QJsonObject jObj({
                             {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().connect_fail}
                             });
            send_to_client(clientSocket, jObj);
            clientSocket.abort();
            emit signalLogEvent("Server → Имя пользователя " + jObj[JSON_KEYS::Common().user_name].toString() + " не является разрешенным");
        }
    }else{
        // Отправка сообщения о том что сосать, а не подключение.
        QJsonObject jObj({
                         {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().connect_fail}
                         });
        send_to_client(clientSocket, jObj);
        clientSocket.disconnectFromHost();
        emit signalLogEvent("Server → IP клиента " + clientIp.toString() + " находися в бан листе до конца сессиий сервера.");
    }
}


// Первое подключение пользователя. Ему отправляется время старта сервера и список ресурсов и кто их занимает и время.
void Server::new_client_autorization(QTcpSocket &sock, const QString &newUsrName){
    m_userList[newUsrName].socket = &sock; // FIXME здесь норм &??

    //QJsonObject jObj;
    QJsonArray resNum, resUser, resTime;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        resNum.push_back(i.key());
        resUser.push_back(i.value().currentUser);
        resTime.push_back(i.value().time.toString("hh:mm:ss"));
    }
    QJsonObject jObj({
                     {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().authorization},
                     {JSON_KEYS::Common().resnum, resNum},
                     {JSON_KEYS::Common().resuser, resUser},
                     {JSON_KEYS::Common().busy_time, resTime},
                     {JSON_KEYS::Config().version, m_cur_version}
                     });
    send_to_client(sock, jObj);
}


// Запрос ресурсов
void Server::res_req_take(const QJsonObject &jObj){
    QJsonArray resNumReq;   // номер запрашиваемого ресурса
    QJsonArray resStatus;   // ответ на запрашиваемый ресурс
    QJsonArray resList;     // индекс ресурса (все, не только запрашиваемые)
    QJsonArray resUser;     // текущий владелец ресурса
    QJsonArray resTime;     // время, которое текущий пользователь занимает ресурс
    qint32 usrTime = static_cast<qint32>(jObj[JSON_KEYS::Common().time].toInt());
    qint32 reqRes = static_cast<qint32>(jObj[JSON_KEYS::ReqType().res_request].toInt());
    qint32 curRes = 0;
    QString usrName = jObj[JSON_KEYS::Common().user_name].toString();
    quint8 idx = 0;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        curRes = (reqRes >> (idx*8)) & 0xFF;
        idx++;
        if(curRes){
            qint64 diffTime = i.value().time.secsTo(QTime::currentTime());
            // Если ресурс свободен
            if(i.value().currentUser == JSON_KEYS::State().free && !reject_res_req){
                i.value().currentUser = usrName;
                i.value().time = QTime(0, 0, 0).addSecs(usrTime);
                resNumReq.push_back(i.key());
                resStatus.push_back(1);
                // вызов потоко-небезопасной функции
                registr(usrName.toUtf8().constData(), idx);
            // ресурс занят, но по таймауту можно перехватить
            }else if(diffTime > maxBusyTime && !reject_res_req){
                QString old_user = i.value().currentUser;
                i.value().currentUser = usrName;
                i.value().time = QTime(0, 0, 0).addSecs(usrTime);
                resNumReq.push_back(i.key());
                resStatus.push_back(1);
                // вызов потоко-небезопасной функции
                registr(usrName.toUtf8().constData(), idx);

                // Составление списка пользователей у которых забрали ресурсы.
                if(m_grabRes.contains(old_user)){
                    m_grabRes[old_user].push_back(i.key());
                }else{
                    m_grabRes.insert(old_user, QJsonArray());
                    m_grabRes[old_user].push_back(i.key());
                }
            // ресурс занят
            }else{
                resNumReq.push_back(i.key());
                resStatus.push_back(0);
            }
        }
        resList.push_back(i.key());
        resUser.push_back(i.value().currentUser);
        resTime.push_back(i.value().time.toString("hh:mm:ss"));
    }

    // Отправка всем пользователям у которых забрали ресурс список какие ресурсы у них забрали.
    QJsonObject oldUserObj;
    oldUserObj.insert(JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().grab_res);
    for(auto i = m_grabRes.begin(); i != m_grabRes.end(); ++i){
        oldUserObj.insert(JSON_KEYS::Common().resource, *i);
        send_to_client(*m_userList[i.key()].socket, oldUserObj); // эта хуйня сломается наверное при отправке send_all_client
    }
    m_grabRes.clear();

    QJsonObject servNotice({
                               {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().request_responce},
                               {JSON_KEYS::Action().action, JSON_KEYS::Action().take},
                               {JSON_KEYS::ReqType().resource_responce, resNumReq},
                               {JSON_KEYS::Common().status, resStatus}
                           });
    send_to_client(*m_userList[usrName].socket, servNotice);
}


// Освобождение ресурсов
void Server::res_req_free(const QJsonObject &jObj){
    QJsonArray resNumReq;   // номер запрашиваемого ресурса
    QJsonArray resStatus;   // ответ на запрашиваемый ресурс
    QJsonArray resList;     // индекс ресурса (все, не только запрашиваемые)
    QJsonArray resUser;     // текущий владелец ресурса
    QJsonArray resTime;     // время, которое текущий пользователь занимает ресурс
    qint64 reqRes = static_cast<qint64>(jObj[JSON_KEYS::ReqType().res_request].toInt());
    qint64 curRes = 0;
    QString usrName = jObj[JSON_KEYS::Common().user_name].toString();
    quint8 idx=0;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        curRes = (reqRes >> (idx*8)) & 0xFF;
        idx++;
        if(curRes && i.value().currentUser == usrName){
            i.value().currentUser = JSON_KEYS::State().free;
            i.value().time.setHMS(0, 0, 0);
            resNumReq.push_back(i.key());
            resStatus.push_back(1);
        }else if(curRes){
            resNumReq.push_back(i.key());
            resStatus.push_back(0);
        }
        resList.push_back(i.key());
        resUser.push_back(i.value().currentUser);
        resTime.push_back(i.value().time.toString("hh:mm:ss"));
    }

    QJsonObject servNotice({
                               {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().request_responce},
                               {JSON_KEYS::Action().action, JSON_KEYS::Action().leave},
                               {JSON_KEYS::ReqType().resource_responce, resNumReq},
                               {JSON_KEYS::Common().status, resStatus},
                               {JSON_KEYS::Common().resnum, resList},
                               {JSON_KEYS::Common().resuser, resUser},
                               {JSON_KEYS::Common().busy_time, resTime}
                           });
    send_to_client(*m_userList[usrName].socket, servNotice);
}


void Server::send_to_client(QTcpSocket &sock, const QJsonObject &jObj){
    if(sock.state() == QAbstractSocket::ConnectedState){
        QJsonDocument jsonDoc(jObj);
        sock.write(jsonDoc.toJson(QJsonDocument::Compact));
    }else{
        emit signalLogEvent("ОШИБКА → Сокет c IP -" + sock.peerAddress().toString() + " не подключен.");
    }
}


// Обновление данных о ресурсах/пользователях у всех клиентов. Наверное так делать не совсем верно.
void Server::send_to_all_clients(){
    QJsonArray resList, resUser, resTime;
    for(auto i = m_resList.begin(); i != m_resList.end(); i++){
        resList.push_back(i.key());
        resUser.push_back(i.value().currentUser);
        resTime.push_back(i.value().time.toString("hh:mm:ss"));
    }
    QJsonObject jObj({
                     {JSON_KEYS::ReqType().type, JSON_KEYS::ReqType().broadcast},
                     {JSON_KEYS::Common().resnum, resList},
                     {JSON_KEYS::Common().resuser, resUser},
                     {JSON_KEYS::Common().busy_time, resTime}
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


