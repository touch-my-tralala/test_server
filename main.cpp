#include <QCoreApplication>
#include <server.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Server *test_server = new Server();


    return a.exec();
}
