#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::time_out);

    QList<quint8> resList = server.getResList();
    ui->tableViewRes->setModel(m_res_model = new ResursTableViewModel);
    for(auto i=0; i < resList.size(); i++){
        m_res_model->appendRes(QString::number(resList[i])); // FIXME: убрать преобразование строку потом
    }

    QStringList usrList = server.getUserList();
    ui->tableViewUsr->setModel(m_usr_model = new UserTableViewModel);
    for(auto i = 0; i < usrList.size(); i++){
        m_usr_model->appendUser(usrList[i]);
    }
    ui->tableViewUsr->resizeColumnsToContents();

    work_time = server.getStartTime();
    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText( "Время работы " + QString::number(days) + " дней " + QTime(0,0,0).addSecs(secs).toString("hh:mm:ss") );

    logger_w = new LoggerWidget(this);
    ui->loggerLayout->addWidget(logger_w);
    connect(&server, &Server::signalLogEvent,
           logger_w, &LoggerWidget::output);

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
        m_res_model->setUser(QString::number(i.key()), res_inf[i.key()].currentUser);
        busyTime = server.getBusyResTime(i.key());
        m_res_model->setTime(QString::number(i.key()), QTime(0, 0, 0).addSecs(busyTime).toString("hh:mm:ss"));
    }

    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText( "Время работы " + QString::number(days) + " дней " + QTime(0,0,0).addSecs(secs).toString("hh:mm:ss") );
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    if(index == 2){
        QStringList usrList = server.getUserList();
        for(auto j: usrList){
            m_usr_model->appendUser(j);
        }
    }
}


void MainWindow::on_addAuthorizedUsrBtn_clicked()
{   // FIXME курсор в рамке текста можно поставить в любом месте, а надо чтобы он двигался только по мере текста
    QString text = ui->lineEdit->text();
    bool answ = m_usr_model->appendUser(text);
    if(answ){
        server.addNewUsrName(ui->lineEdit->text().toLower());
    }
    ui->lineEdit->clear();
}


void MainWindow::on_deleteAuthorizedUsrBtn_clicked()
{
    QStringList rmvUsers;    
    rmvUsers = m_usr_model->removeSelected();
    for(auto i: rmvUsers){
        server.removeUsr(i);
    }
}
