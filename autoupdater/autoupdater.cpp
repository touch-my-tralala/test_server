#include "autoupdater.h"

AutoUpdater::AutoUpdater()
{
}

bool AutoUpdater::setUpdateFilePath(const QString &path){
    QFileInfo fileInfo(path);
    if(fileInfo.exists() && fileInfo.isDir()){
        m_update_file_path = path;
        return true;
    }
    return false;
}

bool AutoUpdater::addUpdateFile(const QString &fileName, const QPair<QString, QString> &fileParams){
    QFileInfo fInfo(m_update_file_path + "/" + fileName);
    if(!fInfo.exists() || !fInfo.isFile())
        return false;
    m_update_files.insert(fileName, fileParams);
    return true;
}

void AutoUpdater::autoUpdate(QTcpSocket &socket, const QStringList fileForUpdates){

}

