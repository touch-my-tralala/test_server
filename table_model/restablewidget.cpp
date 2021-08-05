#include "restablewidget.h"

ResTableWidget::ResTableWidget(QWidget* parent)
    : QTableView(parent)
{
    setModel(m_model = new ResursTableViewModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}
