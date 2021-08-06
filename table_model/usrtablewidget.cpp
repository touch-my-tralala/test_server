#include "usrtablewidget.h"

UsrTableWidget::UsrTableWidget(QWidget *parent) : QTableView(parent)
{
    setModel(m_model = new UserTableViewModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}

bool UsrTableWidget::appendUser(const QString &name){
    resizeColumnsToContents();
    return m_model->appendUser(name);
}

QStringList UsrTableWidget::removeSelected(){
    auto a = m_model->removeSelected();
    resizeColumnsToContents();
    return a;
}
