#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
//#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

class ReportWidget : public QDialog
{
    Q_OBJECT
public:
    ReportWidget(QWidget* parent = nullptr);

    QString getText() { return m_text->toPlainText(); }

private:
    QTextEdit* m_text;
};

#endif // REPORTWIDGET_H
