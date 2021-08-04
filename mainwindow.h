#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QSizePolicy>

#include "server/keys.h"
#include "server/server.h"
#include "table_model/restablewidget.h"
#include "table_model/usrtablewidget.h"
#include "logger/loggerwidget.h"

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
        QString currentUser = "-";
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
    void on_tabWidget_currentChanged(int index);
    void on_pushButtonAdd_clicked();
    void on_pushButtonRemove_clicked();

private:
    qint32 busyTime;
    Ui::MainWindow *ui;
    LoggerWidget *logger_w;
    QDateTime date_time;
    Server server;
    QTimer timer;
    QDateTime work_time;
    UsrTableWidget* m_usr_widget;
    ResTableWidget* m_res_widget;
    QMap<QString, ResInf> m_res_inf;

};
#endif // MAINWINDOW_H
