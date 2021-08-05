#include "resurstableviewmodel.h"

ResursTableViewModel::ResursTableViewModel(QObject* parent)
    : QAbstractTableModel(parent)
{
}

int ResursTableViewModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_resurs.count();
}

int ResursTableViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return LAST;
}

QVariant ResursTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
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
        case USER:
            return "Пользователь";
        case TIME:
            return "Время использования";
        case SELECTED:
            return "Выбор";
    }
    return QVariant();
}

QVariant ResursTableViewModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid() && !(m_resurs.count() <= index.row()))
    {
        if (index.column() == SELECTED && role == Qt::CheckStateRole)
        {
            return m_resurs[index.row()][SELECTED].toBool() ? Qt::Checked : Qt::Unchecked;
        }

        if (role == Qt::TextAlignmentRole)
            return Qt::AlignCenter;

        if ((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() != SELECTED)
            return m_resurs[index.row()][Column(index.column())];
    }
    return QVariant();
}

bool ResursTableViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || m_resurs.count() <= index.row() || role == Qt::EditRole)
        return false;
    if (role == Qt::CheckStateRole)
        setChecked(index, value.toBool());
    else
        m_resurs[index.row()][Column(index.column())] = value;
    emit dataChanged(index, index);
    return true;
}

Qt::ItemFlags ResursTableViewModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags               = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == SELECTED)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

void ResursTableViewModel::setChecked(const QModelIndex& index, bool val)
{
    m_resurs[index.row()][SELECTED] = val;
}

bool ResursTableViewModel::appendRes(const QString& resName)
{
    for (auto& i : m_resurs)
    {
        if (i[NAME] == resName)
            return false;
    }
    ResData resurs;
    resurs[NAME] = resName;
    resurs[USER] = "-";
    resurs[TIME] = "00:00:00";
    auto row     = m_resurs.count();
    beginInsertRows(QModelIndex(), row, row);
    m_resurs.append(resurs);
    endInsertRows();
    return true;
}

bool ResursTableViewModel::setUser(const QString& resName, const QString& usrName)
{
    for (auto& i : m_resurs)
    {
        if (i[NAME] == resName)
        {
            i[USER] = usrName;
            return true;
        }
    }
    return false;
}

bool ResursTableViewModel::setTime(const QString& resName, const QString& resTime)
{
    for (auto& i : m_resurs)
    {
        if (i[NAME] == resName)
        {
            i[TIME] = resTime;
            return true;
        }
    }
    return false;
}

void ResursTableViewModel::removeAllRows()
{
    beginRemoveRows(QModelIndex(), 0, m_resurs.size() - 1);
    m_resurs.clear();
    endRemoveRows();
}

QStringList ResursTableViewModel::removeSelected()
{
    int         k = 0;
    QStringList rmvUsers;
    for (auto i = m_resurs.begin(); i != m_resurs.end();)
    {
        if (i->value(SELECTED, false).toBool())
        {
            beginRemoveRows(QModelIndex(), k, k);
            rmvUsers << i->value(NAME).toString();
            i = m_resurs.erase(i);
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
