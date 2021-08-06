#include "restablewidget.h"

ResTableWidget::ResTableWidget(QWidget* parent)
    : QTableView(parent)
{
    setModel(m_model = new ResursTableViewModel(this));
    resizeColumnsToContents();
    setSelectionMode(QAbstractItemView::NoSelection);
    setFocusPolicy(Qt::NoFocus);
}

bool ResTableWidget::addResource(const QString &name){
    auto a = m_model->appendRes(name);;
    resizeColumnsToContents();
    return a;
}

QStringList ResTableWidget::removeSelected(){
    auto a = m_model->removeSelected();
    resizeColumnsToContents();
    return a;
}
