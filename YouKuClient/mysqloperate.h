#ifndef MYSQLOPERATE_H
#define MYSQLOPERATE_H

#include <QString>
#include <QList>
#include <QtSql/QSqlQuery>
#include "videomessage.h"

class MysqlOperate
{
public:
    MysqlOperate();
    bool connect(const QString &dbName);
    bool createDBTable();
    bool insertData(VideoMessage *message);
    void readFromDB(QList<VideoMessage *> &messages);
};

#endif // MYSQLOPERATE_H
