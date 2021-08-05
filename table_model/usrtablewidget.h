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
    QStringList removeUser() { return m_model->removeSelected(); }
    bool        appendUser(const QString& name) { return m_model->appendUser(name); }
    QStringList removeSelected() { return m_model->removeSelected(); }

private:
    UserTableViewModel* m_model;
};

#endif // USRTABLEWIDGET_H
