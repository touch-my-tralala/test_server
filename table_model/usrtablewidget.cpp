#include "usrtablewidget.h"

UsrTableWidget::UsrTableWidget(QWidget *parent) : QTableView(parent)
{
    setModel(m_model = new UserTableViewModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}
