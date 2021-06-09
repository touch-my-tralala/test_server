#ifndef LOGGERWIDGET_H
#define LOGGERWIDGET_H
#include <QLayout>
#include <QPlainTextEdit>
#include <QTime>
#include <QString>
#include <QtGui>

class LoggerWidget : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit LoggerWidget(QWidget *parent = 0);
    ~LoggerWidget();
    void buildInterface();
    void output(QString s);

private:
    void keyPressEvent(QKeyEvent *e);

signals:
    void onCommand(QString cmd);
};

#endif // LOGGERWIDGET_H
