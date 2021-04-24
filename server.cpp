#include "server.h"



Server::Server()
{
    Server::ini_parse("init.ini");
    qDebug() << "port=" << port;
    qDebug() << "max_user=" << maxUsers;

    m_server = QSharedPointer<QTcpServer>(new QTcpServer(this));

    if(!m_server->listen(QHostAddress::Any, port)){
        qDebug() << "Error";
        m_server->close();
        return;
    }else{
        qDebug() << "Server listening port " << port;
    }
    connect(m_server.data(), &QTcpServer::newConnection,
            this, &Server::slotNewConnection);
}

Server::~Server(){
    // надо удалить settings? или затереть через nullptr?
    // надо закрыть сервер?
}

void Server::ini_parse(QString fname){
    QSettings sett(QDir::currentPath() + "/" + fname, QSettings::IniFormat);
    sett.setIniCodec("UTF-8");
    port = static_cast<quint16>(sett.value("SERVER_SETTINGS/port", 9999).toUInt());
    maxUsers = static_cast<quint8>(sett.value("SERVER_SETTINGS/max_user", 5).toUInt());
    m_userList = QSharedPointer< QMap<QString, UserInf> >(new QMap<QString, UserInf>);
    // Обработка списка разрешенных пользователей
    sett.beginGroup("USER_LIST");
    QStringList iniList = sett.childKeys();
    QStringList::const_iterator i;
    for (i = iniList.begin();i != iniList.end(); ++i){ // FIXME проверить посткремен или декремент
        m_userList->insert(*i, QSharedPointer<UserInf>(new UserInf));
    }
    // Обработка списка ресурсов. FIXME: Мб стоит использовать имена ресурсов тоже, но пока без этого.
    sett.beginGroup("RESOURCE_LIST");
    iniList = sett.childKeys();
    m_resList = QSharedPointer< QMap<quint8, QSharedPointer<ResInf>> >(new QMap<quint8, QSharedPointer<ResInf>>);
    for(quint8 i=0; i<iniList.size(); i++){
        auto info = QSharedPointer<ResInf>(new ResInf);
        m_resList->insert(i, info);
    }
}


void Server::slotNewConnection(){
    QTcpSocket* clientSocket = m_server->nextPendingConnection(); // ?????
    qDebug() << "New connection";
    connect(clientSocket, &QTcpSocket::disconnected,
            clientSocket, &QTcpSocket::deleteLater);
    connect(clientSocket, &QTcpSocket::readyRead,  // Прием данных от клиента
            this, &Server::slotReadClient);
    send_to_client(clientSocket);
}


void Server:: slotReadClient(){
    QSharedPointer<QTcpSocket> clientSocket = static_cast< QSharedPointer<QTcpSocket> >(sender());
    auto jDoc = QJsonDocument::fromJson(clientSocket->readAll(), &jsonErr);
    QJsonObject json = jDoc.object();
    // Получение адреса клиентского сокета
    auto address = clientSocket->peerAddress();
    if(!m_blockIp->contains(address)){
        if(jsonErr.errorString() == "no error occured"){
            if( m_userList->contains(json["username"].toString()) ){ // Если имя клиента есть в списке
                json_handler(json);
            }else{ // Если нет, то штраф сердечко
                m_blockIp->insert(address);
                // Отправка qmessagebox о том что сосать, а не подключение
                qDebug() << "User is not contained in list";
            }
        }else{ // Ошибка json формата
            qDebug() << "Json format error " << jsonErr.errorString();
        }
    }else{
        // Отправка qmessagebox о том что сосать, а не подключение.
        qDebug() << "User is not contained in list";
    }
}


void Server::json_handler(const QJsonObject &jobj){
    if(jobj.size() > 1){ // Если это запрос ресурсов, а не первичная авторизация.
        quint32 usrTime = static_cast<quint32>(jobj["time"].toInt());
        quint32 reqRes = static_cast<quint32>(jobj["request"].toInt());
        quint32 curRes;
        // Проверка какие ресурсы хочет пользователь
        for(int i=0; i<m_resList->size(); i++){
            curRes = (reqRes >> (i*8)) & 0xFF;
            if(curRes > 0){
                int diffTime = QTime::currentTime().secsTo(*(m_resList->value(i)->time)); // так можно делать?
                // Если ресурс свободен
                if(m_resList->value(i)->currenUser == "Free"){
                    auto usrName = static_cast<QString>(jobj["username"].toString()); // FIXME мб можно без пересоздания объекта?
                    m_resList->insert( i, QSharedPointer<ResInf>(new ResInf(usrTime, usrName)) );
                    // FIXME надо добавить отправку уведомления об успехе и еще тему  с тем что какие-то ресурсы были заняты а какие-то нет
                // Если занят, но таймер показывает, что пора делиться добром;
                }else if(diffTime > 7200){ // 7200сек=2ч
                    auto usrName = static_cast<QString>(jobj["username"].toString()); // FIXME мб можно без пересоздания объекта?
                    m_resList->insert( i, QSharedPointer<ResInf>(new ResInf(usrTime, usrName)) );
                    // FIXME надо добавить отправку уведомления об успехе и еще тему  с тем что какие-то ресурсы были заняты а какие-то нет
                    // и еще что у пользователя забрали ресурс
                }else{
                     // Ответить, что ресурс занят.
                }
            }
        }
    }else{

    }
}


QTime* conver_usr_time(QString string_secs){
    int s = static_cast<int>(string_secs.toInt(), 0);
    int s_div = s % 60;
    int m = s / 60;
    int m_div = m % 60;
    int h = m / 60;
    return new QTime(h, m_div, s_div);
}


void res_request(quint8 resNum, QString &username){
    qDebug() << resNum << "===" << username;
}


void Server::send_to_client(QTcpSocket *sock){
    sock -> write("Connected");
}


