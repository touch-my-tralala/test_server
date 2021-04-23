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
    sett.beginGroup("USER_LIST");
    QStringList users = sett.childKeys();
    QStringList::const_iterator i;
    for (i = users.begin();i != users.end(); i++){
        m_userList->insert(*i, QSharedPointer<UserInf>(new UserInf));
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
                json_parse(json);
            }else{
                // Отправка qmessagebox о том что сосать, а не подключение
                qDebug() << "User is not contained in list";
            }
        }else{
            qDebug() << "Json format error " << jsonErr.errorString();
        }
    }else{
        // Отправка qmessagebox о том что сосать, а не подключение.
        qDebug() << "User is not contained in list";
    }
}

void Server::json_parse(const QJsonObject &doc){
    qDebug() << "Receive message is " << doc;
}

void Server::send_to_client(QTcpSocket *sock){
    sock -> write("Connected");
}


