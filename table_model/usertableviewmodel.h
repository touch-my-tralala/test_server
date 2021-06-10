#ifndef USERTABLEVIEWMODEL_H
#define USERTABLEVIEWMODEL_H

#include <QAbstractTableModel>

class UserTableViewModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    UserTableViewModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role );
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
    Qt::ItemFlags flags( const QModelIndex& index ) const;

    bool appendUser(const QString& usrName);

public slots:
    QStringList removeSelected();

private:
    enum Column{
        NAME = 0,
        SELECTED,
        LAST
    };

    typedef QHash<Column, QVariant> UserData;
    QList<UserData> m_users;
};

#endif // USERTABLEVIEWMODEL_H
