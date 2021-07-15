#include "usertableviewmodel.h"

// FIXME: реализовать model-view-controller паттерн
UserTableViewModel::UserTableViewModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int UserTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_users.count();
}

int UserTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return LAST;
}

QVariant UserTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (orientation == Qt::Vertical)
    {
        return section;
    }
    switch (section)
    {
        case NAME:
            return "Имя";
        case SELECTED:
            return "Выбор";
    }
    return QVariant();
}

QVariant UserTableViewModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && !(m_users.count() <= index.row()))
    {
        if (index.column() == SELECTED && role == Qt::CheckStateRole){
            return m_users[index.row()][SELECTED].toBool() ? Qt::Checked : Qt::Unchecked;
        }

        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;

        if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() != SELECTED)
            return m_users[index.row()][Column(index.column())];
    }
    return QVariant();
}

bool UserTableViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_users.count() <= index.row() || role == Qt::EditRole)
        return false;
    if (role == Qt::CheckStateRole)
        setChecked(index, value.toBool());
    else
        m_users[index.row()][Column(index.column())] = value;
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags UserTableViewModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags               = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == SELECTED)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

void UserTableViewModel::setChecked(const QModelIndex& index, bool val)
{
    m_users[index.row()][SELECTED] = val;
}

bool UserTableViewModel::appendUser(const QString& usrName)
{
    for (auto& i : m_users)
    {
        if (i[NAME] == usrName)
            return false;
    }
    UserData user;
    user[NAME]     = usrName;
    auto row       = m_users.count();
    beginInsertRows(QModelIndex(), row, row);
    m_users.append(user);
    endInsertRows();
    return true;
}

QStringList UserTableViewModel::removeSelected()
{
    int         k = 0;
    QStringList rmvUsers;
    for (auto i = m_users.begin(); i != m_users.end();)
    {
        if (i->value(SELECTED, false).toBool())
        {
            beginRemoveRows(QModelIndex(), k, k);
            rmvUsers << i->value(NAME).toString();
            i = m_users.erase(i);
            endRemoveRows();
        }
        else
        {
            ++k;
            ++i;
        }
    }
    return rmvUsers;
}
