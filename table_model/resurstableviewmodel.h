#ifndef RESURSTABLEVIEWMODEL_H
#define RESURSTABLEVIEWMODEL_H

#include <QAbstractTableModel>

class ResursTableViewModel :public QAbstractTableModel
{
    Q_OBJECT
public:
    ResursTableViewModel(QObject* parent = 0);
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent) const;
    QVariant data(const QModelIndex& index, int role ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role );
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const;
    Qt::ItemFlags flags( const QModelIndex& index ) const;
    bool appendRes(const QString& resName);
    bool setUser(const QString &resName, const QString &usrName);
    bool setTime(const QString &resName, const QString &resTime);

public slots:
    QStringList removeSelected();

private:
    enum Column{
        NAME = 0,
        USER,
        TIME,
        SELECTED,
        LAST
    };

    typedef QHash<Column, QVariant> ResData;
    QList<ResData> m_resurs;
};

#endif // RESURSTABLEVIEWMODEL_H
