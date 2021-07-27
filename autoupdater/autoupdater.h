#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>

#define BLOCK_WRITE 32768 //< количество байт для отправки

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

    //! \brief Добавление файлов для обновления.
    //! \param[file] <имя файла, версия>.
    //! \return true если файл успешно добавлен.
    bool addUpdateFile(const QPair<QString, QString> &file);
    //! \brief Установка пути к директории где лежат файлы обновлений.
    //! \return true если путь существует.
    bool setUpdateFilePath(const QString &path);
    //! \brief Проверка необходимости обновлений файла.
    //! \return true если версии не совпадают.
    bool checkFileVersion(const QString &fileName);
    //! \brief Отправка нового файла обновлений.
    //! \param[file] <имя файла, версия>.
    //! \return true если файл успешно отправлен.
    bool sendFile(QTcpSocket *sock, const QPair<QString, QString> &file);
    //! \brief Отправка всех новых файлов обновлений.
    //! \param[files] QMap<имя файла, версия>.
    //! \return число отправленных файлов, если возникла ошибка при передаче или
    //! открытии файла вернет -1;
    int sendFiles(QTcpSocket *sock, const QMap<QString,  QString> &files);

private:
    bool send(QTcpSocket *sock, const QString &fileName);


private:
    QString m_update_file_path = "";
    // FIXME: переделать на контрольную сумму файла или что-то что уникально для файла и не заполнятся руками
    QMap<QString, QString> m_update_files; //< name, version>

};

#endif // AUTOUPDATER_H
