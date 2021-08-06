#include "mainwindow.h"

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("BPOS tracker server");

    m_res_widget = new ResTableWidget(this);
    ui->resTableLayout->addWidget(m_res_widget);

    m_usr_widget = new UsrTableWidget(this);
    ui->usrTableLayout->addWidget(m_usr_widget);

    logger_w = new LoggerWidget(this);
    ui->loggerLayout->addWidget(logger_w);

    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{

    m_tray_icon = new QSystemTrayIcon(this);
    m_tray_icon->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    auto menu        = new QMenu(this);
    auto view_window = new QAction("Открыть", this);
    auto quit_app    = new QAction("Выход", this);

    connect(view_window, &QAction::triggered, this, &MainWindow::show);
    connect(quit_app, &QAction::triggered, this, &MainWindow::close);
    connect(m_tray_icon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    menu->addAction(view_window);
    menu->addAction(quit_app);
    m_tray_icon->setContextMenu(menu);

    // Список ресурсов
    auto resList = server.getResList();
    for (const auto& i : qAsConst(resList))
    {
        m_res_inf.insert(i, ResInf());
        m_res_widget->addResource(i);
    }

    // Список разрешенных пользователей
    auto usrList = server.getUserList();
    for (const auto& i : qAsConst(usrList))
        m_usr_widget->appendUser(i);

    // Время работы
    work_time   = server.getStartTime();
    qint64 days = work_time.daysTo(QDateTime::currentDateTime());
    qint32 secs = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText("Время работы " + QString::number(days) + " дней " + QTime(0, 0, 0).addSecs(secs).toString("hh:mm:ss"));

    // Сигналы от сервера
    connect(&server, &Server::signalLogEvent, logger_w, &LoggerWidget::output);
    connect(&server, &Server::resOwnerChenge, this, &MainWindow::update_res_info);

    // Таймер обновление времени в таблице на 1 секунду
    timer.setInterval(1000);
    connect(&timer, &QTimer::timeout, this, &MainWindow::time_out);
    timer.start();
}

void MainWindow::closeEvent(QCloseEvent* event)
{

    if (isVisible() && ui->tray_en->isChecked())
    {
        event->ignore();
        this->hide();
        m_tray_icon->show();
        if (m_message_flag)
        {
            QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
            m_tray_icon->showMessage("Tray Program", "Приложение свернуто в трей", icon, 2000);
            m_message_flag = false;
        }
    }
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
    qint32 sec = QTime(0, 0, 0).secsTo(ui->timeEdit->time());
    server.setTimeOut(sec);
}

void MainWindow::time_out()
{
    qint32 secs, busyTime;
    for (auto i = m_res_inf.begin(); i != m_res_inf.end(); i++)
    {
        if (i->currentUser != "-")
        {
            busyTime      = server.getBusyResTime(i.key());
            auto usr_time = QTime(0, 0, 0).addSecs(busyTime);
            m_res_widget->updateBusyTime(i.key(), usr_time.toString("hh:mm:ss"));
        }
    }

    qint32 days = static_cast<qint32>(work_time.daysTo(QDateTime::currentDateTime()));
    secs        = work_time.time().secsTo(QTime::currentTime());
    ui->workTime->setText("Время работы " + QString::number(days) + " дней " + QTime(0, 0, 0).addSecs(secs).toString("hh:mm:ss"));
}

void MainWindow::update_res_info()
{
    for (auto i = m_res_inf.begin(), e = m_res_inf.end(); i != e; i++)
    {
        auto cur_usr = server.getResUser(i.key());
        m_res_widget->setUser(i.key(), cur_usr);
        m_res_widget->setTime(i.key(), "00:00:00");
        i.value().currentUser = cur_usr;
        i.value().time        = QTime(0, 0, 0);
    }
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
    }
}

void MainWindow::on_pushButtonAdd_clicked()
{
    QString text = ui->lineEdit->text();

    if (ui->tabWidget->currentIndex() == 0)
    {
        m_res_widget->addResource(text);
        server.addNewRes(text);
        m_res_inf.insert(text, ResInf());
    }

    if (ui->tabWidget->currentIndex() == 2)
    {
        m_usr_widget->appendUser(text);
        server.addNewUsrName(text);
    }
    ui->lineEdit->clear();
}

void MainWindow::on_pushButtonRemove_clicked()
{
    if (ui->tabWidget->currentIndex() == 0)
    {
        auto rmvList = m_res_widget->removeSelected();
        for (const auto& i : qAsConst(rmvList))
        {
            server.removeRes(i);
            m_res_inf.remove(i);
        }
    }

    if (ui->tabWidget->currentIndex() == 2)
    {
        auto rmvList = m_usr_widget->removeSelected();
        for (auto& i : rmvList)
            server.removeUsr(i);
    }
}

void MainWindow::on_pushButton_clicked()
{
    QSharedPointer<ReportWidget> h_dialog = QSharedPointer<ReportWidget>(new ReportWidget);
    if (h_dialog->exec() == QDialog::Accepted)
    {
        QFile file("/reports/reports.txt");
        if (file.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            QTextStream out(&file);
            out << h_dialog->getText();
            file.close();
        }
    }
}

void MainWindow::on_changeAddress_triggered()
{
    QSharedPointer<HostInputDialog> h_dialog = QSharedPointer<HostInputDialog>(new HostInputDialog);
    if (h_dialog->exec() == QDialog::Accepted)
    {
        auto port = static_cast<quint16>(h_dialog->getPort().toInt());
        server.changePort(port);
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::DoubleClick:
            if (!isVisible())
            {
                this->show();
                m_tray_icon->hide();
            }
            else
            {
                this->hide();
                m_tray_icon->show();
            }

            break;

        default:
            break;
    }
}
