#include "server.h"



Server::Server()
{
    Server::iniParse("init.ini");
    qDebug() << "port=" << this->port;
    qDebug() << "max_user=" << this->maxUsers;
    QStringList::const_iterator i;
        for(i=this->userList.constBegin(); i != this->userList.constEnd(); ++i){
            qDebug() << *i;
        }



//    server = new QTcpServer(this);
//    if(!server->listen(QHostAddress::Any, port)){
//        qDebug() << "Error";
//        server->close();
//        return;
//    }else{
//        qDebug() << "Server listening port " << port;
//    }
//    connect(server, &QTcpServer::newConnection,
//            this, &Server::slotNewConnection);
}

Server::~Server(){
    // надо удалить settings? или затереть через nullptr?
    // надо закрыть сервер?
}

void Server::iniParse(QString fname){
    QSettings sett(QDir::currentPath() + "/" + fname, QSettings::IniFormat);
    sett.setIniCodec("UTF-8");
    this->port = sett.value("SERVER_SETTINGS/port", 9999).toInt();
    this->maxUsers = sett.value("SERVER_SETTINGS/max_user", 5).toInt();
    sett.beginGroup("USER_LIST");
    this->userList = sett.childKeys();
}

void Server::slotNewConnection(){
    QTcpSocket* clientSocket = server->nextPendingConnection();
    qDebug() << "New connection";
    connect(clientSocket, &QTcpSocket::disconnected,
            clientSocket, &QTcpSocket::deleteLater);
    connect(clientSocket, &QTcpSocket::readyRead,  // Прием данных от клиента
            this, &Server::slotReadClient);
    sendToClient(clientSocket);
}

void Server:: slotReadClient(){
    QTcpSocket *clientSocket = static_cast<QTcpSocket*>(sender());
//    auto data = QString::fromUtf8(clientSocket->readAll()).trimmed();
    auto jdata = QJsonDocument::fromJson(clientSocket->readAll(), &jsonErr);
    if(jsonErr.errorString() == "no error occured"){
        this->jsonParse(&jdata);
    }else{
        qDebug() << "Json format error " << jsonErr.errorString();
    }

}

void Server::jsonParse(QJsonDocument *doc){
    qDebug() << "Receive message is " << doc;
}

void Server::sendToClient(QTcpSocket *sock){
    sock -> write("Connected");
}


