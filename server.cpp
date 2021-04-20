#include "server.h"

Server::Server(quint16 port)
{
    server = new QTcpServer(this);
    if(!server->listen(QHostAddress::Any, port)){
        qDebug() << "Error";
        server->close();
        return;
    }else{
        qDebug() << "Server listening port " << port;
    }
    connect(server, &QTcpServer::newConnection,
            this, &Server::slotNewConnection);
}

Server::~Server(){

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


