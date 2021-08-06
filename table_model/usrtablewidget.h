#ifndef USRTABLEWIDGET_H
#define USRTABLEWIDGET_H

#include <QTableView>
#include <QTime>
#include <QWidget>

#include "table_model/usertableviewmodel.h"

class UsrTableWidget : public QTableView
{
public:
    UsrTableWidget(QWidget* parent = nullptr);
    bool        appendUser(const QString& name);
    QStringList removeSelected();

private:
    UserTableViewModel* m_model;
};

#endif // USRTABLEWIDGET_H
