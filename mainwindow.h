#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//#include <QtCore>
#include <server.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct ResInf{
        ResInf(){
            time = new QTime();
            time->setHMS(0, 0, 0);
        }
        ~ResInf(){
            delete time;
            time = nullptr;
        }
        QTime* time = nullptr;
        QString currentUser = "Free";
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void time_out();
    void on_rejectConnCheckBox_stateChanged(int arg1);
    void on_rejectResCheckBox_stateChanged(int arg1);
    void on_clearAllBtn_clicked();
    void on_timeoutBtn_clicked();

private:
    Ui::MainWindow *ui;
    QSharedPointer<QDateTime> date_time;
    QSharedPointer<Server> server;
    QSharedPointer<QTimer> timer;
    QMap<quint8, ResInf*> res_inf;
    qint64 busyTime;
    qint64 hh, mm, ss;
};
#endif // MAINWINDOW_H
