#include "hostinputdialog.h"

HostInputDialog::HostInputDialog(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
        m_port = new QLineEdit(this);

        QLabel* plblPort   = new QLabel("&Port", this);
        plblPort->setBuddy(m_port);

        QPushButton* pcmdOk     = new QPushButton("&Ok", this);
        QPushButton* pcmdCancel = new QPushButton("&Cancel", this);

        connect(pcmdOk, SIGNAL(clicked()), SLOT(accept()));
        connect(pcmdCancel, SIGNAL(clicked()), SLOT(reject()));

        //Layout setup
        QGridLayout* ptopLayout = new QGridLayout(this);
        ptopLayout->addWidget(plblPort, 0, 0);
        ptopLayout->addWidget(m_port, 0, 1);
        ptopLayout->addWidget(pcmdOk, 2,0);
        ptopLayout->addWidget(pcmdCancel, 2, 1);
        setLayout(ptopLayout);
}
