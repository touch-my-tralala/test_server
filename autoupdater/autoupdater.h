#ifndef AUTOUPDATER_H
#define AUTOUPDATER_H
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QDataStream>

#define BLOCK_WRITE 32768 //< количество байт для отправки

class AutoUpdater
{
public:
    AutoUpdater();
    ~AutoUpdater();

    //! \brief Добавление файлов для обновления.
    //! \param[file] <имя файла, версия>.
    //! \return true если файл успешно добавлен.
    bool addUpdateFile(const QPair<QString, QString> &file);
    //! \brief Удаление из списка всех ранее добавленных файлов
    void removeUpdateFiles(){ m_update_files.clear(); }
    //! \brief Установка пути к директории где лежат файлы обновлений.
    //! \param[absolut] true - абсолютный путь, false - относительный запуска приложения.
    //! \return true если путь существует.
    bool setUpdateFilePath(const QString &path);
    //! \brief Проверка необходимости обновлений файла.
    //! \return true если версии не совпадают.
    bool checkFileVersion(const QString &fileName);
    //! \brief Проверка обновления и отправка нового файла при необходимости. Имеет заголовок 4 байта - размер файла, кастомный заголовок(при необходимости).
    //! \param[file] <имя файла, версия>.
    //! \param[header] кастомный заголовок, который необходимо добавить.
    //! \return true файл успешно отправлен, false нет зарегистрированного файла с таким именем или обновление не требуется.
    bool checkAndSendFile(QTcpSocket &sock, const QPair<QString, QVariant> &file, const QByteArray& header);
    //! \brief Возвращает мапу зарегестрированных файлов
    //! \return <имя файла, версия>
    QMap<QString, QString> getFileMap(){ return m_update_files; }
    //! \brief Возвращает путь, где сейчас ищуться файлы обновлений
    QString getUpdatesPath(){ return m_update_file_path; }

private:
    bool send(QTcpSocket &sock, const QString &fileName, const QByteArray& header);


private:
    QString m_update_file_path = "";
    // FIXME: переделать на контрольную сумму файла или что-то что уникально для файла и не заполнятся руками
    QMap<QString, QString> m_update_files; //< name, version>

};

#endif // AUTOUPDATER_H
