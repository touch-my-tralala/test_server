#include "autoupdater.h"

AutoUpdater::AutoUpdater()
{
}

AutoUpdater::~AutoUpdater() = default;

bool AutoUpdater::setUpdateFilePath(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists() && fileInfo.isDir())
    {
        m_update_file_path = path;
        return true;
    }
    return false;
}

bool AutoUpdater::addUpdateFile(const QPair<QString, QString>& file)
{
    auto      file_name    = file.first;
    auto      file_version = file.second;
    QFileInfo fInfo(m_update_file_path + "/" + file_name);

    if (!fInfo.exists() || !fInfo.isFile())
        return false;

    m_update_files.insert(file_name, file_version);
    return true;
}

bool AutoUpdater::checkAndSendFile(QTcpSocket& sock, const QPair<QString, QVariant>& file, const QByteArray& header)
{
    auto file_name    = file.first;
    auto file_version = static_cast<QString>(file.second.toString());
    if (m_update_files.contains(file_name) && m_update_files[file_name] != file_version)
    {
        send(sock, file_name, header);
        return true;
    }

    return false;
}

bool AutoUpdater::send(QTcpSocket& sock, const QString& fileName, const QByteArray& header)
{
    if (sock.state() != QTcpSocket::ConnectedState)
        return false;

    QFile file(m_update_file_path + "/" + fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        bool        firs_pack = true;
        QByteArray  block;
        QDataStream fileStream(&file);
        char*       read_block = new char[BLOCK_WRITE];

        QDataStream sendStream(&block, QIODevice::ReadWrite);

        while (fileStream.readRawData(read_block, BLOCK_WRITE))
        {
            if (firs_pack)
            {
                sendStream << quint32(file.size()) << header << read_block;
                firs_pack = false;
            }
            else
            {
                sendStream << read_block;
            }

            sock.write(block);
            block.clear(); // FIXME: надо проверить, мб после seek(0) можно не обнулять буфер а писать поверх
            sendStream.device()->seek(0);
        }
        delete[] read_block;
        return true;
    }
    file.close();
    return false;
}
