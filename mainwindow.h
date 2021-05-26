#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
//#include <QtCore>
#include <server.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct ResInf{
        ResInf(){}
        ~ResInf(){}
        QTime time;
        QString currentUser = "Free";
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void time_out();
    void on_rejectConnCheckBox_stateChanged(int arg1);
    void on_rejectResCheckBox_stateChanged(int arg1);
    void on_clearAllBtn_clicked();
    void on_timeoutBtn_clicked();

    void on_tabWidget_tabBarClicked(int index);

    void on_tabWidget_currentChanged(int index);

private:
    qint32 busyTime;
    Ui::MainWindow *ui;
    QDateTime date_time;
    Server server;
    QTimer timer;
    QStandardItemModel table_model;
    QMap<quint8, ResInf> res_inf;
};
#endif // MAINWINDOW_H
