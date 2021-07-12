#ifndef USERTABLEVIEWMODEL_H
#define USERTABLEVIEWMODEL_H

#include <QAbstractTableModel>

class UserTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    UserTableViewModel(QObject* parent = nullptr);
    int           rowCount(const QModelIndex& parent) const override;
    int           columnCount(const QModelIndex& parent) const override;
    QVariant      data(const QModelIndex& index, int role) const override;
    bool          setData(const QModelIndex& index, const QVariant& value, int role) override;
    QVariant      headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool appendUser(const QString& usrName);

public slots:
    QStringList removeSelected();

private:
    enum Column
    {
        NAME = 0,
        SELECTED,
        LAST
    };

    typedef QHash<Column, QVariant> UserData;
    QList<UserData>                 m_users;
};

#endif // USERTABLEVIEWMODEL_H
