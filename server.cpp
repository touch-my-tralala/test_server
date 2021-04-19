#include "server.h"
#include <iostream>
#include <QDebug>
#include <QString>

Server::Server(quint16 port)
{
    qDebug() << "Constructor";
    server = new QTcpServer(this);
    if(!server->listen(QHostAddress::Any, port)){
        qDebug() << "Error";
        server->close();
        return;
    }else{
        qDebug() << "Server listening port " << port;
    }
//    connect(server, SIGNAL(newConnection()),
//            this, SLOT(slotNewConnection()));
    connect(server, &QTcpServer::newConnection,
            this, &Server::slotNewConnection);
}

Server::~Server(){};

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
    QTcpSocket *clientSocket = (QTcpSocket*)sender();
    auto data = QString::fromUtf8(clientSocket->readAll()).trimmed();
    qDebug() << data;
}

void Server::sendToClient(QTcpSocket *sock){
    sock -> write("Connected");
}


