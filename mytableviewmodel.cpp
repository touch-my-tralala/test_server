#include "mytableviewmodel.h"

MyTableViewModel::MyTableViewModel(QObject* parent): QAbstractTableModel( parent ){}


int MyTableViewModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return m_users.count();
}


int MyTableViewModel::columnCount(const QModelIndex &parent) const{
    Q_UNUSED(parent)
    return LAST;
}


QVariant MyTableViewModel::headerData(int section, Qt::Orientation orientation, int role) const{
    if(role != Qt::DisplayRole){
        return QVariant();
    }
    if(orientation == Qt::Vertical){
        return section;
    }
    switch (section) {
    case NAME:
        return "Имя";
    case SELECTED:
        return "Выбор";
    }
    return QVariant();
}


QVariant MyTableViewModel::data(const QModelIndex &index, int role) const{
    if(!index.isValid() ||
       m_users.count() <= index.row() ||
       ( role != Qt::DisplayRole && role != Qt::EditRole )
      ){
        return QVariant();
    }
    return m_users[index.row()][Column( index.column() )];
}


Qt::ItemFlags MyTableViewModel::flags(const QModelIndex &index) const{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if(index.isValid()){
        if(index.column() == SELECTED){
            flags |= Qt::ItemIsEditable;
        }
    }
    return flags;
}


bool MyTableViewModel::appendUser(const QString &usrName){
    for(auto i: m_users){
        if(i[NAME] == usrName)
            return false;
    }
    UserData user;
    user[NAME] = usrName;
    user[SELECTED] = false;
    int row = m_users.count();
    beginInsertRows( QModelIndex(), row, row);
    m_users.append(user);
    endInsertRows();
    return true;
}


bool MyTableViewModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if( !index.isValid() || role != Qt::EditRole || m_users.count() <= index.row() ){
        return false;
    }
    m_users[index.row()][Column( index.column() )] = value;
    dataChanged(index, index);
    return true;
}


void MyTableViewModel::removeSelected(){
    int k = 0;
    for(auto i = m_users.begin(); i != m_users.end();){
        if( i->value( SELECTED, false).toBool() ){ // FIXME тут сначал забирается текущее значение а затем сразу устанавливается новое?
            beginRemoveRows(QModelIndex(), k, k);
            i = m_users.erase(i);
            endRemoveRows();
        }else{
            ++k;
            ++i;
        }
        qDebug();
    }
}

