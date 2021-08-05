#include "reportwidget.h"

ReportWidget::ReportWidget(QWidget* parrent)
    : QDialog(parrent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{

    // Надо установить размер

    m_text = new QLineEdit(this);
    m_text->setMaxLength(200);
    m_text->setBaseSize(100, 100);
    QCheckBox* checkBox = new QCheckBox(this);

    QLabel* mainLabel  = new QLabel("&Не обещаю, что что-то из ваших жалоб будет пофикшено в ближайшее время.", this);
    QLabel* textLabel  = new QLabel("&Кратко опишите проблему", this);
    QLabel* checkLabel = new QLabel("&Серёга-mode", this);

    textLabel->setBuddy(m_text);
    checkLabel->setBuddy(checkBox);

    QPushButton* pcmdOk     = new QPushButton("&Ok", this);
    QPushButton* pcmdCancel = new QPushButton("&Cancel", this);

    connect(pcmdOk, SIGNAL(clicked()), SLOT(accept()));
    connect(pcmdCancel, SIGNAL(clicked()), SLOT(reject()));
    connect(checkBox, &QCheckBox::stateChanged, this, &ReportWidget::bigMode);

    QHBoxLayout* hLayout = new QHBoxLayout(this);
    hLayout->addWidget(pcmdCancel);
    hLayout->addWidget(pcmdOk);

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->addWidget(mainLabel);
    vLayout->addWidget(textLabel);
    vLayout->addWidget(m_text);
    vLayout->addWidget(checkLabel);
    vLayout->addWidget(checkBox);
    vLayout->addLayout(hLayout);

    setLayout(vLayout);
}

void ReportWidget::bigMode(int state)
{
    if (state)
    {
        m_text->setMaxLength(5000);
        m_text->setBaseSize(800, 800);
    }
    else
    {
        m_text->setMaxLength(200);
        m_text->setBaseSize(100, 100);
    }
}
