#ifndef RESTABLEWIDGET_H
#define RESTABLEWIDGET_H

#include <QTableView>
#include <QTime>
#include <QWidget>

#include "table_model/resurstableviewmodel.h"

class ResTableWidget : public QTableView
{
    Q_OBJECT

public:
    ResTableWidget(QWidget* parent = nullptr);
    //! \brief Добавление ресурса в таблицу
    bool addResource(const QString& name) { return m_model->appendRes(name); }
    //! \brief Удаление отмеченных ресурсов
    //! \return Список имен удаленных ресурсов
    QStringList removeSelected() { return m_model->removeSelected(); }
    //! \brief Изменение времени текущего использования ресурса
    //! \param[secs] время в секундах, прошедшее с момента занятия ресурса
    void updateBusyTime(const QString& resName, const QString& time) { m_model->setTime(resName, time); }
    void setTime(const QString& resName, const QString& time) { m_model->setTime(resName, time); }
    void setUser(const QString& resName, const QString& usrName) { m_model->setUser(resName, usrName); }

private:
    ResursTableViewModel* m_model;
};

#endif // RESTABLEWIDGET_H
