#include "mysqloperate.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QObject>
#include <QVariantList>
#include <QDebug>

MysqlOperate::MysqlOperate()
{

}

//连接数据库
bool MysqlOperate::connect(const QString &dbName)
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
    {
        db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        db = QSqlDatabase::addDatabase("QMYSQL");
    }

    db.setHostName("localhost");
    db.setDatabaseName(dbName);
    db.setUserName("root");
    db.setPassword("root");

    if(!db.open())
    {
        QMessageBox::critical(0, QObject::tr("Database Error"),
                              db.lastError().text());
        return false;
    }

    return true;
}

//创建数据表
bool MysqlOperate::createDBTable()
{
    QSqlQuery query;

    if(!query.exec("CREATE TABLE play ("
                   "id VARCHAR(255) PRIMARY KEY,"
                   "title VARCHAR(255),"
                   "thumbnail VARCHAR(255),"
                   "link VARCHAR(255),"
                   "category VARCHAR(255))"))
    {
        QMessageBox::critical(0, QObject::tr("Database Error"),
                              query.lastError().text());

        return false;
    }

    return true;
}

//向数据库的数据表插入数据
bool MysqlOperate::insertData(VideoMessage *message)
{
    QSqlQuery query;

    //判断是否存在数据表play，不存在就创建
    if(!query.exec("SELECT * FROM play"))
        createDBTable();
//    bool has = query.exec("SELECT * FROM play");
//    //qDebug() << "has" << has;
//    if(has == false)
//        createDBTable();

    //若数据库中已存在该条记录，则不进行插入操作，直接退出
    query.exec("SELECT id FROM play");
    while(query.next())
    {
        QString str = query.value(0).toString();
        if(str == message->id())
            return false;
    }
    query.finish();
    query.prepare("insert into play (id, title, thumbnail, link, category) "
                  "values (?, ?, ?, ?, ?)");
    QVariantList id;
    id << message->id();
    query.addBindValue(id);
    QVariantList title;
    title << message->title();
    query.addBindValue(title);
    QVariantList thumbnail;
    thumbnail << message->thumbnail();
    query.addBindValue(thumbnail);
    QVariantList link;
    link << message->link();
    query.addBindValue(link);
    QVariantList category;
    category << message->category();
    query.addBindValue(category);

    if(!query.execBatch())
    {
        QMessageBox::critical(0, QObject::tr("Database Error"),
                              query.lastError().text());

        return false;
    }

    return true;
}

//从数据库读数据
void MysqlOperate::readFromDB(QList<VideoMessage *> &messages)
{
    QSqlQuery query;

    query.exec("SELECT * FROM play");
    while(query.next())
    {
        //在while循环外创建，messages里面的内容是一样的，是数据库中的最后一个元素
        //原因明天想
        VideoMessage *message = new VideoMessage;

        message->setId(query.value(0).toString());
        message->setTitle(query.value(1).toString());
        message->setThumbnail(query.value(2).toString());
        message->setLink(query.value(3).toString());
        message->setCategory(query.value(4).toString());

        messages.append(message);
    }
}
