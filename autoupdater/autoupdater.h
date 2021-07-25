#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H
#include<QtCore>
#include <QTcpSocket>


class AutoUpdater
{

struct UpdateKeys{
    const QString update_file_name = "update.ini";
    const QString files = "FILES";
    const QString file_path = "file_path"; //< путь файла, где он лежит у клиента
    const QString version = "version";
};

public:
    AutoUpdater();
    ~AutoUpdater();

    //! Добавление файлов для обновления QPair<версия, путь>
    bool addUpdateFile(const QString &fileName, const QPair<QString, QString> &fileParams);
    //! установка пути где лежать файлы обновлений
    bool setUpdateFilePath(const QString &path);
    //! Проверка необходимости обновления и отправка файлов обновлений
    void autoUpdate(QTcpSocket &socket, const QStringList fileForUpdates);

private:
    void check_need_update();
    void send_update_files();


private:
    QString m_update_file_path;
    QSettings sett;
    // FIXME: переделать на контрольную сумму файла или что-то что уникально для файла и не заполнятся руками
    QMap<QString, QPair<QString, QString>> m_update_files; //< name - <version, path>

};

#endif // AUTOUPDATER_H
