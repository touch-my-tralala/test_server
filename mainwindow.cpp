#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    logger_w = new LoggerWidget(this);
    ui->loggerLayout->addWidget(logger_w);


    QStringList headerLabel;
    headerLabel << "Ресурс" << "Пользователь" << "Время использования";
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
        ui->tableWidget->item(i, 0)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem( i, 1, new QTableWidgetItem(res_inf[i].currentUser) );
        ui->tableWidget->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem( i, 2, new QTableWidgetItem(res_inf[i].time.toString("hh:mm:ss")) );
        ui->tableWidget->item(i, 2)->setTextAlignment(Qt::AlignCenter);
    }
    ui->tableWidget->resizeColumnsToContents();

    QStringList usrList = server.getUserList();
    ui->tableView->setModel(m_model = new MyTableViewModel);
    for(auto i = 0; i < usrList.size(); i++){
        m_model->appendUser(usrList[i]);
    }

    work_time = server.getStartTime();
    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText( "Время работы " + QString::number(days) + " дней " + QTime(0,0,0).addSecs(secs).toString("hh:mm:ss") );

    timer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_rejectConnCheckBox_stateChanged(int arg1)
{
    if(arg1)
        logger_w->output("Входящие подключения отклоняются.");
    else
        logger_w->output("Входящие подключения принимаются.");
    server.setRejectConnection(arg1);
}


void MainWindow::on_rejectResCheckBox_stateChanged(int arg1)
{
    if(arg1)
        logger_w->output("Запросы ресурсов отклоняются.");
    else
        logger_w->output("Запросы ресурсов принимаются.");
    server.setRejectResReq(arg1);
}


void MainWindow::on_clearAllBtn_clicked()
{
    logger_w->output("Освобождение всех ресурсов.");
    server.allResClear();
}


void MainWindow::on_timeoutBtn_clicked()
{
    qint64 sec = QTime(0, 0, 0).secsTo(ui->timeEdit->time());
    logger_w->output("Установлен тайм-аут использования ресурсов: " + QTime(0,0,0).addSecs(sec).toString("hh:mm:ss."));
    server.setTimeOut(sec);
}


void MainWindow::time_out(){
    for(auto i = res_inf.begin(); i != res_inf.end(); i++){
            res_inf[i.key()].currentUser = server.getResUser(i.key());
            ui->tableWidget->item(i.key(), 1)->setData(Qt::DisplayRole ,res_inf[i.key()].currentUser);
            ui->tableWidget->item(i.key(), 1)->setTextAlignment(Qt::AlignCenter);
            busyTime = server.getBusyResTime(i.key());
            ui->tableWidget->item(i.key(), 2)->setData(Qt::DisplayRole, QTime(0, 0, 0).addSecs(busyTime).toString("hh:mm:ss"));
            ui->tableWidget->item(i.key(), 2)->setTextAlignment(Qt::AlignCenter);
    }
    ui->tableWidget->resizeColumnsToContents();


    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText( "Время работы " + QString::number(days) + " дней " + QTime(0,0,0).addSecs(secs).toString("hh:mm:ss") );
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 2){
        QStringList usrList = server.getUserList();
        for(auto j: usrList){
            m_model->appendUser(j);
        }
    }
}


void MainWindow::on_addAuthorizedUsrBtn_clicked()
{   // FIXME курсор в рамке текста можно поставить в любом месте, а надо чтобы он двигался только по мере текста
    QString text = ui->userNameLineEdit->text();
    bool answ = m_model->appendUser(text);
    if(answ){
        server.addNewUsrName(ui->userNameLineEdit->text().toLower());
        logger_w->output("Пользователь " + text + " успешно добавлен.");
        //statusBar()->showMessage("User added successfully");
    }else{
        //statusBar()->showMessage("Username is already in the list");
        logger_w->output("Имя " + text + " уже есть в списке.");
    }
    ui->userNameLineEdit->clear();
}


void MainWindow::on_deleteAuthorizedUsrBtn_clicked()
{
    QStringList rmvUsers;

    rmvUsers = m_model->removeSelected();
    for(auto i: rmvUsers){
        server.removeUsr(i);
        logger_w->output("Удален пользователь " + i);
    }
}
