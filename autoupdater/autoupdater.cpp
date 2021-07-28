#include "autoupdater.h"

AutoUpdater::AutoUpdater()
{
}

AutoUpdater::~AutoUpdater() = default;

bool AutoUpdater::setUpdateFilePath(const QString &path){
    QFileInfo fileInfo(path);
    if(fileInfo.exists() && fileInfo.isDir()){
        m_update_file_path = path;
        return true;
    }
    return false;
}

bool AutoUpdater::addUpdateFile(const QPair<QString, QString> &file){
    auto file_name = file.first;
    auto file_version = file.second;
    QFileInfo fInfo(m_update_file_path + "/" + file_name);

    if(!fInfo.exists() || !fInfo.isFile())
        return false;

    m_update_files.insert(file_name, file_version);
    return true;
}

bool AutoUpdater::sendFile(QTcpSocket &sock, const QPair<QString, QVariant> &file){
    auto file_name = file.first;
    auto file_version = static_cast<QString>(file.second.toString());
    if(m_update_files.contains(file_name) && m_update_files[file_name] != file_version){
        send(sock, file_name);
        return true;
    }

    return false;
}

bool AutoUpdater::send(QTcpSocket &sock, const QString &fileName){
    if(sock.state() != QTcpSocket::ConnectedState)
        return false;

    QFile file(m_update_file_path + "/" + fileName);
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray file_arr = file.readAll();
        auto byte_num = file_arr.size();
        sock.write(QJsonDocument({{fileName, byte_num}}).toJson(QJsonDocument::Compact));

        for(int j = 0, f = byte_num; j < f; j += BLOCK_WRITE)
            sock.write(file_arr.mid(j, BLOCK_WRITE));

        return true;
    }
    file.close();
    return false;
}
