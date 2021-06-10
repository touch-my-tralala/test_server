#include "loggerwidget.h"

LoggerWidget::LoggerWidget(QWidget *parent) : QPlainTextEdit(parent)
{
    setMaximumBlockCount(10000);
    setFocusPolicy(Qt::WheelFocus);
    buildInterface();
}
LoggerWidget::~LoggerWidget(){

}

void LoggerWidget::buildInterface(){
    this->setStyleSheet("QPlainTextEdit  {"
                                   "background-color: rgba(255, 255, 255, 1);"
                                   "color: rgba(47, 204, 10, 0.95);"
                                   //"background-image: url(:/resources/img/Console_bg.png);"
                                   "background-repeat: no-repeat;"
                                   "background-position: top center;"
                                   "font-weight: 400;"
                                   "font-style: normal;"
                                   "font-size: 13px;"
                                   "}"
                        "QMenu:disabled {"
                               "background-color: #08a4bfa3;"
                               "color:grey;"
                                   "}"
                        "QMenu:enabled {"
                               "background-color: #08a4bfd6;"
                               "color:white;"
                                   "}");
    this->setReadOnly(true);
}

void LoggerWidget::keyPressEvent(QKeyEvent *e){
    if (e->key() == Qt::Key_Delete && e->modifiers() == Qt::NoModifier)
        this->clear();

    if (e->key() == Qt::Key_A && e->modifiers() == Qt::ControlModifier)
        this->selectAll();

    if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
        this->copy();

    if (e->key() == Qt::Key_Z && e->modifiers() == Qt::ControlModifier)
        this->undo();
}

void LoggerWidget::output(QString s){
        QString font_color = "<font color=#11c621>"; //green
        QString font_end = "</font>";
        if(s.contains("ОШИБКА",Qt::CaseInsensitive))
            font_color =  "<font color=#d00c0c>"; //red
//        else
//            font_color =  "<font color=#7a7a7a>"; //grey
        appendHtml(font_color + QTime::currentTime().toString("HH:mm:ss.  ") + s + font_end);
}

