#ifndef KEYS_H
#define KEYS_H
#include <QString>

class KEYS{

public:
    static const struct Config{
        const QString server_settings = "SERVER_SETTINGS";
        const QString resource_list   = "RESOURCE_LIST";
        const QString user_list       = "USER_LIST";
        const QString port            = "port";
        const QString max_user        = "max_user";
        const QString updates         = "UPDATES";
        const QString update_path     = "update_path";
    }CONFIG;

    static const struct Json{
        // common
        const QString type              = "type";
        const QString user_name         = "username";
        const QString resources         = "resources";
        // res info
        const QString res_name          = "res_name";
        const QString time              = "time";
        const QString status            = "status";
        // req type
        const QString connect_fail      = "connect_fail";
        const QString authorization     = "authorization";
        const QString grab_res          = "grab_res";
        const QString request_responce  = "request_responce";
        const QString broadcast         = "broadcast";
        const QString res_request       = "res_request";
        // Action
        const QString action            = "action";
        const QString take              = "take";
        const QString drop              = "drop";
    }JSON;

    static const struct Common{
        const QString no_user = "-";
    }COMMON;

    static const struct Updater{
        const QString files        = "files";
        const QString update_req   = "update_request";
        const QString file_name    = "file_name";
        const QString file_version = "version";
    }UPDATER;

};

#endif // KEYS_H
