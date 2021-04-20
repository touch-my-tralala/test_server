#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>


class Server: public QObject
{
    Q_OBJECT
private:
    QTcpServer *server;
    QByteArray data;

private:
    void sendToClient(QTcpSocket *sock);

public:
    Server(quint16 port);
    ~Server();

public slots:
    void slotNewConnection();
    void slotReadClient();
};




#endif // SERVER_H
