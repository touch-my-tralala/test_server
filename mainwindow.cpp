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

    date_time =  QDateTime(QDateTime::currentDateTime());
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::time_out);

    QList<quint8> resList = server.getResList();
    for(auto i: resList){
        res_inf.insert(i, ResInf());
        res_inf[i].currentUser = server.getResUser(i);
        busyTime = server.getBusyResTime(i);
        res_inf[i].time = QTime(0, 0, 0).addSecs(busyTime);
    }

    for(auto i = 0; i < res_inf.size(); i++){
        ui->tableWidget->setItem( i, 0, new QTableWidgetItem(QString::number(i)) );
        ui->tableWidget->setItem( i, 1, new QTableWidgetItem(res_inf[i].currentUser) );
        ui->tableWidget->setItem( i, 2, new QTableWidgetItem(res_inf[i].time.toString("hh:mm:ss")) );
    }

    timer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_rejectConnCheckBox_stateChanged(int arg1)
{
    server.setRejectConnection(arg1);
}

void MainWindow::on_rejectResCheckBox_stateChanged(int arg1)
{
    server.setRejectResReq(arg1);
}

void MainWindow::on_clearAllBtn_clicked()
{
    server.allResClear();
}

void MainWindow::on_timeoutBtn_clicked()
{
    qint64 sec = QTime(0, 0, 0).secsTo(ui->timeEdit->time());
    server.setTimeOut(sec);
}

void MainWindow::time_out(){
    for(auto i = res_inf.begin(); i != res_inf.end(); i++){
            res_inf[i.key()].currentUser = server.getResUser(i.key());
            ui->tableWidget->item(i.key(), 1)->setData(Qt::DisplayRole ,res_inf[i.key()].currentUser);
            busyTime = server.getBusyResTime(i.key());
            ui->tableWidget->item(i.key(), 2)->setData(Qt::DisplayRole, QTime(0, 0, 0).addSecs(busyTime).toString("hh:mm:ss"));
        }
}
