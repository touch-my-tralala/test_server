#include "mainwindow.h"

#include "ui_mainwindow.h"

// Прочитать про модели в qt. Может стоит сделать чекбокс через qdelegate.

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("BPOS trecker");
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::time_out);

    QStringList resList = server.getResList();
    ui->tableViewRes->setModel(m_res_model = new ResursTableViewModel);
    for (auto& i : resList)
        m_res_model->appendRes(i); // FIXME: убрать преобразование строку потом

    ui->tableViewRes->resizeColumnsToContents();
    ui->tableViewRes->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableViewRes->setFocusPolicy(Qt::NoFocus);

    QStringList usrList = server.getUserList();
    ui->tableViewUsr->setModel(m_usr_model = new UserTableViewModel);
    for (auto& i : usrList)
        m_usr_model->appendUser(i);

    ui->tableViewUsr->resizeColumnsToContents();
    ui->tableViewUsr->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableViewUsr->setFocusPolicy(Qt::NoFocus);

    work_time   = server.getStartTime();
    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText("Время работы " + QString::number(days) + " дней " + QTime(0, 0, 0).addSecs(secs).toString("hh:mm:ss"));

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

void MainWindow::time_out()
{
    for (auto i = res_inf.begin(); i != res_inf.end(); i++)
    {
        res_inf[i.key()].currentUser = server.getResUser(i.key());
        m_res_model->setUser(i.key(), res_inf[i.key()].currentUser);
        busyTime = server.getBusyResTime(i.key());
        m_res_model->setTime(i.key(), QTime(0, 0, 0).addSecs(busyTime).toString("hh:mm:ss"));
    }

    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint64 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText("Время работы " + QString::number(days) + " дней " + QTime(0, 0, 0).addSecs(secs).toString("hh:mm:ss"));
}

void MainWindow::on_tabWidget_currentChanged(int index)
{
    ui->lineEdit->setEnabled(true);
    ui->pushButtonAdd->setEnabled(true);
    ui->pushButtonRemove->setEnabled(true);

    if (index == 0)
    {
        ui->lineEdit->clear();
        ui->lineEdit->setPlaceholderText("Type resource name");
    }

    if (index == 1)
    {
        ui->lineEdit->setEnabled(false);
        ui->pushButtonAdd->setEnabled(false);
        ui->pushButtonRemove->setEnabled(false);
    }

    if (index == 2)
    {
        ui->lineEdit->clear();
        ui->lineEdit->setPlaceholderText("Type user name");
        QStringList usrList = server.getUserList();
        for (auto& j : usrList)
        {
            m_usr_model->appendUser(j);
        }
    }
}

void MainWindow::on_pushButtonAdd_clicked()
{
    QString text = ui->lineEdit->text();
    if (ui->tabWidget->currentIndex() == 0)
    {
        if (m_res_model->appendRes(text))
            server.addNewRes(text); // FIXME изменить потом на строку
        ui->lineEdit->clear();
    }

    if (ui->tabWidget->currentIndex() == 2)
    {
        bool answ = m_usr_model->appendUser(text);
        if (answ)
        {
            server.addNewUsrName(ui->lineEdit->text().toLower());
        }
        ui->lineEdit->clear();
    }
}

void MainWindow::on_pushButtonRemove_clicked()
{
    QStringList rmvList;
    if (ui->tabWidget->currentIndex() == 0)
    {
        rmvList = m_res_model->removeSelected();
        for (auto& i : rmvList)
            server.removeRes(i);
    }

    if (ui->tabWidget->currentIndex() == 2)
    {
        rmvList = m_usr_model->removeSelected();
        for (auto& i : rmvList)
            server.removeUsr(i);
    }
}
