#include "reportwidget.h"

ReportWidget::ReportWidget(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
    setWindowTitle("Report");
    setFixedSize(400, 300);

    m_text = new QTextEdit(this);

    QLabel* mainLabel  = new QLabel("Не обещаю, что что-то из ваших жалоб будет пофикшено когда-либо.", this);
    QLabel* textLabel  = new QLabel("Кратко опишите проблему", this);

    QPushButton* pcmdOk     = new QPushButton("&Ok", this);
    QPushButton* pcmdCancel = new QPushButton("&Cancel", this);

    connect(pcmdOk, SIGNAL(clicked()), SLOT(accept()));
    connect(pcmdCancel, SIGNAL(clicked()), SLOT(reject()));

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    QHBoxLayout* hLayout = new QHBoxLayout(this);
    QVBoxLayout* vLayout1 = new QVBoxLayout(this);
    vLayout1->addWidget(mainLabel);
    vLayout1->addWidget(textLabel);
    vLayout1->addWidget(m_text);
    hLayout->addWidget(pcmdOk);
    hLayout->addWidget(pcmdCancel);

    vLayout->addLayout(vLayout1);
    vLayout->addLayout(hLayout);
    vLayout->addStretch(0);
    setLayout(vLayout); 
}
