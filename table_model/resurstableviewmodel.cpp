#include "resurstableviewmodel.h"

ResursTableViewModel::ResursTableViewModel(QObject* parent): QAbstractTableModel( parent ){

}


int ResursTableViewModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return m_resurs.count();
}


int ResursTableViewModel::columnCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return LAST;
}


QVariant ResursTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role != Qt::DisplayRole){
        return QVariant();
    }
    if(orientation == Qt::Vertical){
        return section;
    }
    switch (section) {
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

// https://stackoverflow.com/questions/4958893/how-to-set-text-alignment-on-a-column-of-qtableview-programmatically
QVariant ResursTableViewModel::data(const QModelIndex &index, int role) const{
    if(!index.isValid() ||
       m_resurs.count() <= index.row() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole )
      ){
        return QVariant();
    }
    return m_resurs[index.row()][Column( index.column() )];
}


Qt::ItemFlags ResursTableViewModel::flags(const QModelIndex &index) const{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if(index.isValid()){
        if(index.column() == SELECTED){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}


bool ResursTableViewModel::appendRes(const QString &resName){
    for(auto i: m_resurs){
        if(i[NAME] == resName)
            return false;
    }
    ResData resurs;
    resurs[NAME] = resName;
    resurs[USER] = "free";
    resurs[TIME] = "00:00:00";
    resurs[SELECTED] = false;
    int row = m_resurs.count();
    beginInsertRows( QModelIndex(), row, row);
    m_resurs.append(resurs);
    endInsertRows();
    return true;
}


bool ResursTableViewModel::setUser(const QString &resName, const QString &usrName){
    for(auto i: m_resurs){
        if(i[NAME] == resName){
            i[USER] = usrName;
            return true;
        }
    }
    return false;
}


bool ResursTableViewModel::setTime(const QString &resName, const QString &resTime){
    for(auto i: m_resurs){
        if(i[NAME] == resName){
            i[TIME] = resTime;
            return true;
        }
    }
    return false;
}


bool ResursTableViewModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if( !index.isValid() || role != Qt::EditRole || m_resurs.count() <= index.row() ){
        return false;
    }
    m_resurs[index.row()][Column( index.column() )] = value;
    dataChanged(index, index);
    return true;
}


QStringList ResursTableViewModel::removeSelected(){
    int k = 0;
    QStringList rmvUsers;
    for(auto i = m_resurs.begin(); i != m_resurs.end();){
        if( i->value( SELECTED, false).toBool() ){ // FIXME тут сначал забирается текущее значение а затем сразу устанавливается новое?
            beginRemoveRows(QModelIndex(), k, k);
            rmvUsers << i->value(NAME).toString();
            i = m_resurs.erase(i);
            endRemoveRows();
        }else{
            ++k;
            ++i;
        }
    }
    return rmvUsers;
}
