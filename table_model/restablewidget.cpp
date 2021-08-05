#include "restablewidget.h"

ResTableWidget::ResTableWidget(QWidget* parent)
    : QTableView(parent)
{
    setModel(m_model = new ResursTableViewModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}

void ResTableWidget::updateBusyTime(const QString& resName, const int& secs)
{
    auto busyTime = QString::number(secs / 3600) + ":" + QString::number((secs % 3600) / 60) + ":" + QString::number(secs % 60);
    m_model->setTime(resName, busyTime);
}
