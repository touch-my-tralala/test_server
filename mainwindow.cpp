#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStringList headerLabel;
    headerLabel << "Res num" << "Res user" << "Busy time";
    ui->tableWidget->setHorizontalHeaderLabels(headerLabel);

    date_time = QSharedPointer<QDateTime>(new QDateTime(QDateTime::currentDateTime()));
    timer = QSharedPointer<QTimer>(new QTimer());
    timer->setInterval(1000);
    connect(timer.data(), &QTimer::timeout, this, &MainWindow::time_out);

    server = QSharedPointer<Server>(new Server());
    /// NOTE: вся логика касающаяся сервера должна быть реализона в самой библиотеке сервера.
    /// можно вынести отдельные публичные методы (если необходимо) для контроля
    /// или подписаться на сигналы от сервера (например, в целях логирования)
    server->setMaxUser(20);
    QList<quint8> resList = server->getResList();
    for(auto i: resList){
        res_inf.insert(i, new ResInf());
        res_inf[i]->currentUser = server->getResUser(i);
        busyTime = server->getBusyResTime(i);
        hh = busyTime / 3600;
        mm = (busyTime % 3600) / 60;
        ss = busyTime % 60;
        res_inf[i]->time = new QTime(hh ,mm, ss);
    }

    for(auto i = 0; i < res_inf.size(); i++){
        ui->tableWidget->setItem( i, 0, new QTableWidgetItem(QString::number(i)) );
        ui->tableWidget->setItem( i, 1, new QTableWidgetItem(res_inf[i]->currentUser) );
        ui->tableWidget->setItem( i, 2, new QTableWidgetItem(res_inf[i]->time->toString("hh:mm:ss")) );
    }

    timer->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_rejectConnCheckBox_stateChanged(int arg1)
{
    server->setRejectConnection(arg1);
}

void MainWindow::on_rejectResCheckBox_stateChanged(int arg1)
{
    server->setRejectResReq(arg1);
}

void MainWindow::on_clearAllBtn_clicked()
{
    server->allResClear();
}

void MainWindow::on_timeoutBtn_clicked()
{
    qint64 hh = ui->timeEdit->time().hour();
    qint64 mm = ui->timeEdit->time().minute();
    qint64 sec = hh * 3600 + mm * 60;
    server->setTimeOut(sec);
}

void MainWindow::time_out(){
    for(auto i = res_inf.begin(); i != res_inf.end(); i++){
            res_inf[i.key()]->currentUser = server->getResUser(i.key());
            ui->tableWidget->item(i.key(), 1)->setData(Qt::DisplayRole ,res_inf[i.key()]->currentUser);
            busyTime = server->getBusyResTime(i.key());
            hh = busyTime / 3600;
            mm = (busyTime % 3600) / 60;
            ss = busyTime % 60;
            QString t;
            t = QString::number(hh) + ":" +  QString::number(mm) + ":" + QString::number(ss);
            ui->tableWidget->item(i.key(), 2)->setData(Qt::DisplayRole, t);
        }
}
